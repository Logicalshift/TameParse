//
//  parser.cpp
//  Parse
//
//  Created by Andrew Hunter on 07/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <algorithm>

#include "parser.h"

using namespace std;
using namespace contextfree;
using namespace lr;

/// \brief Ranks actions in the order they should appear in the final table
static inline int action_score(int action) {
    switch (action) {
        case lr_action::act_weakreduce:
            // Weak reduce actions come first as they should always be performed if their symbol will produce a shift
            return 0;
            
        case lr_action::act_shift:
            // Shift actions are preferred if there's a conflict
            return 1;
            
        case lr_action::act_reduce:
            // Reduce actions have the lowest priority
            return 2;
            
        case lr_action::act_goto:
            // Gotos never actually produce a clash
            return 3;
            
        default:
            return 4;
    }
}

static inline bool compare_actions(const parser::action& a, const parser::action& b) {
    // First, compare on symbol IDs
    if (a.m_SymbolId < b.m_SymbolId) return true;
    
    // Next, compare on action types
    if (action_score(a.m_Type) < action_score(b.m_Type)) return true;
    
    // Actions are equal
    return false;
}

/// \brief Creates a parser from the result of the specified builder class
parser::parser(const lalr_builder& builder) {
    // Allocate the tables
    m_NumStates             = builder.count_states();
    m_NonterminalActions    = new action*[m_NumStates];
    m_TerminalActions       = new action*[m_NumStates];
    
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
            }
        }
        
        // Sort the actions for this state
        std::sort(termActions, termActions + termCount, compare_actions);
        std::sort(nontermActions, nontermActions + nontermCount, compare_actions);
        
        // Store the actions in the table
        m_TerminalActions[stateId]      = termActions;
        m_NonterminalActions[stateId]   = nontermActions;
    }
    
    // Fill in the rule table
    m_Rules = new reduce_rule[ruleIds.size()];
    
    for (map<int, int>::iterator ruleId = ruleIds.begin(); ruleId != ruleIds.end(); ruleId++) {
        m_Rules[ruleId->second].m_Identifier    = ruleId->first;
        m_Rules[ruleId->second].m_Length        = (int)gram.rule_with_identifier(ruleId->first)->items().size();
    }
}

/// \brief Destructor
parser::~parser() {
    // Destroy each entry in the parser table
    for (int x=0; x<m_NumStates; x++) {
        delete[] m_NonterminalActions[x];
        delete[] m_TerminalActions[x];
    }
    
    // Destroy the tables themselves
    delete[] m_NonterminalActions;
    delete[] m_TerminalActions;
    delete[] m_Rules;
}
