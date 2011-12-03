//
//  parser_tables.cpp
//  Parse
//
//  Created by Andrew Hunter on 07/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <algorithm>

#include "TameParse/Lr/parser_tables.h"

using namespace std;
using namespace contextfree;
using namespace lr;

/// \brief Ranks actions in the order they should appear in the final table
static inline int action_score(int action) {
    switch (action) {
        case lr_action::act_guard:
            // Guard actions are always evaluated first, so we can substitute the guard symbol if it's matched
            return 0;
            
        case lr_action::act_weakreduce:
            // Weak reduce actions come first as they should always be performed if their symbol will produce a shift
            return 1;
            
        case lr_action::act_shift:
        case lr_action::act_shiftstrong:
        case lr_action::act_divert:
            // Shift actions are preferred if there's a conflict
            return 2;
            
        case lr_action::act_reduce:
            // Reduce actions have the lowest priority
            return 3;
            
        case lr_action::act_goto:
            // Gotos never actually produce a clash
            return 4;
            
        default:
            return 5;
    }
}

static inline bool compare_actions(const parser_tables::action& a, const parser_tables::action& b) {
    // First, compare on symbol IDs
    if (a.m_SymbolId < b.m_SymbolId) return true;
    if (a.m_SymbolId > b.m_SymbolId) return false;
    
    // Next, compare on action types
    int aScore = action_score(a.m_Type);
    int bScore = action_score(b.m_Type);
    
    if (aScore < bScore) return true;
    if (aScore > bScore) return false;
    
    // Actions are equal
    return false;
}

/// \brief Creates a parser from the result of the specified builder class
parser_tables::parser_tables(const lalr_builder& builder, const weak_symbols* weakSymbols) 
: m_DeleteTables(true) {
    // Allocate the tables
    m_NumStates             = builder.count_states();
    m_NonterminalActions    = new action*[m_NumStates];
    m_TerminalActions       = new action*[m_NumStates];
    m_Counts                = new action_count[m_NumStates];
    
    contextfree::end_of_input eoi;
    contextfree::end_of_guard eog;
    m_EndOfInput            = builder.gram().identifier_for_item(eoi);
    m_EndOfGuard            = builder.gram().identifier_for_item(eog);
    
    const grammar& gram = builder.gram();
    
    map<int, int>   ruleIds;                            // Maps their rule IDs to our rule IDs
    vector<int>     eogStates;                          // The end of guard states
    
    // Build up the tables for each state
    for (int stateId = 0; stateId < m_NumStates; ++stateId) {
        // Get the actions for this state
        const lr_action_set& actions = builder.actions_for_state(stateId);
        
        // Count the number of terminal and nonterminal actions
        int termCount       = 0;
        int nontermCount    = 0;
        
        for (lr_action_set::const_iterator nextAction = actions.begin(); nextAction != actions.end(); ++nextAction) {
            if ((*nextAction)->item()->type() == item::terminal) {
                termCount++;
            } else {
                nontermCount++;
            }
        }
        
        // Allocate action tables of the appropriate size
        action* termActions     = new action[termCount];
        action* nontermActions  = new action[nontermCount];
        
        int     termPos         = 0;                    // Current item in the terminal table
        int     nontermPos      = 0;                    // Current item in the nonterminal table
        
        // Fill up the actions (not in order)
        for (lr_action_set::const_iterator nextAction = actions.begin(); nextAction != actions.end(); ++nextAction) {
            // Get the next state
            int nextState   = (*nextAction)->next_state();
            int type        = (*nextAction)->type();
            
            // If the next action is a reduce action, then the next state should actually be the rule to reduce
            if (type == lr_action::act_reduce || type == lr_action::act_weakreduce || type == lr_action::act_accept) {
                // Look up the ID we assigned this rule
                int                     ruleId  = (*nextAction)->rule()->identifier(gram);
                map<int, int>::iterator found   = ruleIds.find(ruleId);
                
                if (found == ruleIds.end()) {
                    nextState       = (int)ruleIds.size();
                    ruleIds[ruleId] = nextState;
                } else {
                    nextState       = found->second;
                }
            }
            
            if ((*nextAction)->item()->type() == item::terminal) {
                // Add a new terminal action
                termActions[termPos].m_Type             = type;
                termActions[termPos].m_NextState        = nextState;
                termActions[termPos].m_SymbolId         = (*nextAction)->item()->symbol();
                
                termPos++;
            } else {
                // Add a new nonterminal action
                nontermActions[nontermPos].m_Type       = type;
                nontermActions[nontermPos].m_NextState  = nextState;
                nontermActions[nontermPos].m_SymbolId   = gram.identifier_for_item((*nextAction)->item());
                
                if (nontermActions[nontermPos].m_SymbolId == m_EndOfGuard) {
                    eogStates.push_back(stateId);
                }
                
                nontermPos++;
            }
        }
        
        // Sort the actions for this state
        std::sort(termActions, termActions + termCount, compare_actions);
        std::sort(nontermActions, nontermActions + nontermCount, compare_actions);
        
        // Store the actions in the table
        m_TerminalActions[stateId]      = termActions;
        m_NonterminalActions[stateId]   = nontermActions;
        m_Counts[stateId].m_NumTerms    = termCount;
        m_Counts[stateId].m_NumNonterms = nontermCount;
    }
    
    // Store the end of guard state table (we evaluate states in order, so this is already sorted)
    m_NumEndOfGuards    = (int) eogStates.size();
    m_EndGuardStates    = new int[m_NumEndOfGuards];
    for (int x=0; x<m_NumEndOfGuards; ++x) {
        m_EndGuardStates[x] = eogStates[x];
    }
    
    // Fill in the rule table
    m_NumRules  = (int)ruleIds.size();
    m_Rules     = new reduce_rule[ruleIds.size()];
    
    for (map<int, int>::iterator ruleId = ruleIds.begin(); ruleId != ruleIds.end(); ++ruleId) {
        const rule_container& rule = gram.rule_with_identifier(ruleId->first);
        
        m_Rules[ruleId->second].m_Identifier    = gram.identifier_for_item(rule->nonterminal());
        m_Rules[ruleId->second].m_RuleId        = rule->identifier(gram);
        m_Rules[ruleId->second].m_Length        = (int)rule->items().size();
    }
    
    // Fill in the weak symbols table
    if (!weakSymbols) {
        // No weak symbols
        m_NumWeakToStrong   = 0;
        m_WeakToStrong      = NULL;
    } else {
        // Count the number of weak symbols
        m_NumWeakToStrong   = 0;
        for (weak_symbols::strong_iterator weakForStrong = weakSymbols->begin_strong(); weakForStrong != weakSymbols->end_strong(); ++weakForStrong) {
            m_NumWeakToStrong += weakForStrong->second.size();
        }
        
        // Fill in the table as an unordered list
        m_WeakToStrong      = new symbol_equivalent[m_NumWeakToStrong];
        int pos = 0;
        for (weak_symbols::strong_iterator weakForStrong = weakSymbols->begin_strong(); weakForStrong != weakSymbols->end_strong(); ++weakForStrong) {
            // Fetch the strong symbol
            const item_container& strong = weakForStrong->first;
            
            // We're only actually mapping terminal symbols here (which is all that should be in this array anyway)
            if (strong->type() != item::terminal) continue;
            
            // Iterate through the weak symbols that this symbol is matched to
            for (item_set::const_iterator weakItem = weakForStrong->second.begin(); weakItem != weakForStrong->second.end(); ++weakItem) {
                // Fetch the weak symbol that this item is mapped to
                const item_container& weak = *weakItem;
                
                // Should be a terminal item
                if (weak->type() != item::terminal) continue;
                
                // Fill in this item
                symbol_equivalent equiv = { weak->symbol(), strong->symbol() };
                m_WeakToStrong[pos++]   = equiv;
            }
        }
        
        // In case there were any mappings between non terminal symbols, update the symbol count
        m_NumWeakToStrong = pos;
        
        // Sort the items
        sort(m_WeakToStrong, m_WeakToStrong + m_NumWeakToStrong);
    }
}

/// \brief Creates a parser from a set of tables. Tables passed into this constructor will not be deleted by the destructor
parser_tables::parser_tables(int numStates, int endOfInputSymbol, int endOfGuardSymbol, action** terminalActions, action** nonterminalActions, action_count* actionCounts, int* endGuardStates, int numEndGuards, int numRules, reduce_rule* reduceRules, int numWeakToStrong, symbol_equivalent* weakToStrong)
: m_NumStates(numStates)
, m_EndOfInput(endOfInputSymbol)
, m_EndOfGuard(endOfGuardSymbol)
, m_TerminalActions(terminalActions)
, m_NonterminalActions(nonterminalActions)
, m_Counts(actionCounts)
, m_EndGuardStates(endGuardStates)
, m_NumEndOfGuards(numEndGuards)
, m_NumRules(numRules)
, m_Rules(reduceRules)
, m_NumWeakToStrong(numWeakToStrong)
, m_WeakToStrong(weakToStrong)
, m_DeleteTables(false) {
}

/// \brief Copy constructor
parser_tables::parser_tables(const parser_tables& copyFrom) 
: m_NumStates(copyFrom.m_NumStates)
, m_NumRules(copyFrom.m_NumRules)
, m_EndOfInput(copyFrom.m_EndOfInput)
, m_EndOfGuard(copyFrom.m_EndOfGuard)
, m_DeleteTables(copyFrom.m_DeleteTables)
, m_NumWeakToStrong(copyFrom.m_NumWeakToStrong) {
    // Allocate the action tables
    m_TerminalActions       = new action*[m_NumStates];
    m_NonterminalActions    = new action*[m_NumStates];
    m_Counts                = new action_count[m_NumStates];
    
    // Copy the states
    for (int stateId=0; stateId<m_NumStates; ++stateId) {
        // Copy the counts
        m_Counts[stateId] = copyFrom.m_Counts[stateId];
        
        // Allocate the array for this state
        m_TerminalActions[stateId]      = new action[m_Counts[stateId].m_NumTerms];
        m_NonterminalActions[stateId]   = new action[m_Counts[stateId].m_NumNonterms];
        
        // Copy the terminals and nonterminals
        for (int x=0; x<m_Counts[stateId].m_NumTerms; ++x) {
            m_TerminalActions[stateId][x] = copyFrom.m_TerminalActions[stateId][x];
        }
        for (int x=0; x<m_Counts[stateId].m_NumNonterms; ++x) {
            m_NonterminalActions[stateId][x] = copyFrom.m_NonterminalActions[stateId][x];
        }
    }
    
    // Allocate the rule table
    m_Rules = new reduce_rule[m_NumRules];
    for (int ruleId=0; ruleId<m_NumRules; ++ruleId) {
        m_Rules[ruleId] = copyFrom.m_Rules[ruleId];
    }
    
    // Allocate the end of guard state table
    m_NumEndOfGuards    = copyFrom.m_NumEndOfGuards;
    m_EndGuardStates    = new int[m_NumEndOfGuards];
    
    // Copy the states
    for (int x=0; x<m_NumEndOfGuards; ++x) {
        m_EndGuardStates[x] = copyFrom.m_EndGuardStates[x];
    }

    // Copy the weak to strong table
    if (copyFrom.m_WeakToStrong) {
        m_WeakToStrong = new symbol_equivalent[m_NumWeakToStrong];

        for (int x=0; x<m_NumWeakToStrong; ++x) {
            m_WeakToStrong[x] = copyFrom.m_WeakToStrong[x];
        }
    } else {
        m_WeakToStrong = NULL;
    }
}

/// \brief Assignment
parser_tables& parser_tables::operator=(const parser_tables& copyFrom) {
    if (&copyFrom == this) return *this;
    
    // TODO
    
    return *this;
}

/// \brief Destructor
parser_tables::~parser_tables() {
    if (m_DeleteTables) {
        // Destroy each entry in the parser table
        for (int x=0; x<m_NumStates; ++x) {
            delete[] m_NonterminalActions[x];
            delete[] m_TerminalActions[x];
        }
        
        // Destroy the tables themselves
        delete[] m_NonterminalActions;
        delete[] m_TerminalActions;
        delete[] m_Rules;
        delete[] m_Counts;
        delete[] m_EndGuardStates;
        if (m_WeakToStrong) delete[] m_WeakToStrong;
    }
}

/// \brief Calculates the size in bytes of these parser tables
size_t parser_tables::size() const {
    // Start with the size of this class
    size_t total = sizeof(parser_tables);
    
    // Add in the size of the various arrays
    total += 2 * sizeof(action*) * m_NumStates;                 // Size of the nonterminal and terminal action arrays
    total += sizeof(action_count) * m_NumStates;                // m_Counts
    total += sizeof(reduce_rule) * m_NumRules;                  // m_Rules
    
    // Add up the size of the various rule arrays
    for (int stateId = 0; stateId < m_NumStates; ++stateId) {
        total += sizeof(action) * m_Counts[stateId].m_NumTerms;
        total += sizeof(action) * m_Counts[stateId].m_NumNonterms;
    }
    
    // This is the result
    return total;
}
