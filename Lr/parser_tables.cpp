//
//  parser_tables.cpp
//  Parse
//
//  Created by Andrew Hunter on 07/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <algorithm>

#include "parser_tables.h"

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
    
    // Next, compare on action types
    if (action_score(a.m_Type) < action_score(b.m_Type)) return true;
    
    // Actions are equal
    return false;
}

/// \brief Creates a parser from the result of the specified builder class
parser_tables::parser_tables(const lalr_builder& builder) {
    // Allocate the tables
    m_NumStates             = builder.count_states();
    m_NonterminalActions    = new action*[m_NumStates];
    m_TerminalActions       = new action*[m_NumStates];
    m_Counts                = new action_count[m_NumStates];
    
    contextfree::end_of_input eoi;
    m_EndOfInput            = builder.gram().identifier_for_item(eoi);
    
    const grammar& gram = builder.gram();
    
    map<int, int> ruleIds;                          // Maps their rule IDs to our rule IDs
    
    // Build up the tables for each state
    for (int stateId = 0; stateId < m_NumStates; stateId++) {
        // Get the actions for this state
        const lr_action_set& actions = builder.actions_for_state(stateId);
        
        // Count the number of terminal and nonterminal actions
        int termCount       = 0;
        int nontermCount    = 0;
        
        for (lr_action_set::const_iterator nextAction = actions.begin(); nextAction != actions.end(); nextAction++) {
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
        for (lr_action_set::const_iterator nextAction = actions.begin(); nextAction != actions.end(); nextAction++) {
            // Get the next state
            int nextState   = (*nextAction)->next_state();
            int type        = (*nextAction)->type();
            
            // If the next action is a reduce action, then the next state should actually be the rule to reduce
            if (type == lr_action::act_reduce || type == lr_action::act_weakreduce) {
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
    
    // Fill in the rule table
    m_NumRules  = (int)ruleIds.size();
    m_Rules     = new reduce_rule[ruleIds.size()];
    
    for (map<int, int>::iterator ruleId = ruleIds.begin(); ruleId != ruleIds.end(); ruleId++) {
        const rule_container& rule = gram.rule_with_identifier(ruleId->first);
        
        m_Rules[ruleId->second].m_Identifier    = gram.identifier_for_item(rule->nonterminal());
        m_Rules[ruleId->second].m_Length        = (int)rule->items().size();
    }
}

/// \brief Copy constructor
parser_tables::parser_tables(const parser_tables& copyFrom) 
: m_NumStates(copyFrom.m_NumStates)
, m_NumRules(copyFrom.m_NumRules)
, m_EndOfInput(copyFrom.m_EndOfInput) {
    // Allocate the action tables
    m_TerminalActions       = new action*[m_NumStates];
    m_NonterminalActions    = new action*[m_NumStates];
    m_Counts                = new action_count[m_NumStates];
    
    // Copy the states
    for (int stateId=0; stateId<m_NumStates; stateId++) {
        // Copy the counts
        m_Counts[stateId] = copyFrom.m_Counts[stateId];
        
        // Allocate the array for this state
        m_TerminalActions[stateId]      = new action[m_Counts[stateId].m_NumTerms];
        m_NonterminalActions[stateId]   = new action[m_Counts[stateId].m_NumNonterms];
        
        // Copy the terminals and nonterminals
        for (int x=0; x<m_Counts[stateId].m_NumTerms; x++) {
            m_TerminalActions[stateId][x] = copyFrom.m_TerminalActions[stateId][x];
        }
        for (int x=0; x<m_Counts[stateId].m_NumNonterms; x++) {
            m_NonterminalActions[stateId][x] = copyFrom.m_NonterminalActions[stateId][x];
        }
    }
    
    // Allocate the rule table
    m_Rules = new reduce_rule[m_NumRules];
    for (int ruleId=0; ruleId<m_NumRules; ruleId++) {
        m_Rules[ruleId] = copyFrom.m_Rules[ruleId];
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
    // Destroy each entry in the parser table
    for (int x=0; x<m_NumStates; x++) {
        delete[] m_NonterminalActions[x];
        delete[] m_TerminalActions[x];
    }
    
    // Destroy the tables themselves
    delete[] m_NonterminalActions;
    delete[] m_TerminalActions;
    delete[] m_Rules;
    delete[] m_Counts;
}
