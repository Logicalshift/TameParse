//
//  lalr_machine.cpp
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/common.h"

#include <queue>

#include "TameParse/Lr/lalr_machine.h"

using namespace std;
using namespace contextfree;
using namespace lr;

static const empty_item an_empty_item;

/// \brief Creates an empty LALR machine, which will reference the specified gramamr
lalr_machine::lalr_machine(contextfree::grammar& gram)
: m_Grammar(&gram) {
}

/// \brief Adds a new state to this machine, or retrieves the identifier for the existing state
///
/// This returns the identifier for the state, and the state is updated so that its identifier 
/// reflects this. The state should not have any more lr1_items added to it, but it is permissable
/// to alter the lookahead set after calling this.
///
/// Typically, a state will be added with no lookahead set, and the lookahead will be added later.
/// (An algorithm that builds the LALR parser from an LR(1) set might do this differently, though)
///
int lalr_machine::add_state(container& newState) {
    // Just return the state ID if the new state already has one
    if (newState->identifier() >= 0) {
        return newState->identifier();
    }
    
    // Try to find the existing state
    state_to_identifier::iterator found = m_StateIds.find(newState);
    
    if (found != m_StateIds.end()) {
        // Set the identifier for the state that was passed in
        newState->set_identifier(found->second);
        
        // Return the result
        return found->second;
    }
    
    // The new ID is the last entry in the state table
    int newId = (int) m_States.size();
    
    // Set the new ID
    newState->set_identifier(newId);
    
    // Store this state
    m_StateIds[newState] = newId;
    m_States.push_back(newState);
    m_Transitions.push_back(transition_set());

    // Result is the new state ID
    return newId;
}

/// \brief Adds a transition to this state machine
///
/// Transitions involving terminals create shift actions in the final parser. Nonterminals and EBNF
/// items go into the goto table for the final parser. The empty item should be ignored. Guard items
/// are a little weird: they act like shift actions if they are matched.
void lalr_machine::add_transition(int stateId, const contextfree::item_container& item, int newStateId) {
    // Do nothing if the new state ID is invalid
    if (newStateId < 0 || newStateId >= (int) m_States.size()) return;
    
    // Set this transition
    m_Transitions[stateId].insert(transition(item, newStateId));
}

/// \brief Adds the specified set of lookahead items to the state with the supplied ID
bool lalr_machine::add_lookahead(int stateId, int itemId, const contextfree::item_set& newLookahead) {
    if (itemId < 0) return false;
    item_set& la = m_States[stateId]->lookahead_for(itemId);
    
    bool result = false;
    for (item_set::const_iterator it = newLookahead.begin(); it != newLookahead.end(); ++it) {
        // Ignore the empty item
        if ((*it)->type() == item::empty) continue;
        
        // Otherwise, carry on
        if (la.insert(*it)) result = true;
    }
    
    return result;
}
     
/// \brief Adds the specified set of lookahead items to the state with the supplied ID
bool lalr_machine::add_lookahead(int stateId, const lr0_item& item, const contextfree::item_set& newLookahead) {
    return add_lookahead(stateId, m_States[stateId]->find_identifier(item), newLookahead);
}
