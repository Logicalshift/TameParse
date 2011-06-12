//
//  lalr_machine.cpp
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <queue>

#include "lalr_machine.h"

using namespace std;
using namespace contextfree;
using namespace lr;

static const empty_item an_empty_item;

/// \brief Creates an empty LALR machine, which will reference the specified gramamr
lalr_machine::lalr_machine(contextfree::grammar& gram)
: m_Grammar(&gram) {
}

/// \brief Creates the closure for a particular lalr state
void lalr_machine::create_closure(closure_set& target, const lalr_state& state, const grammar* gram) {
    queue<lr1_item_container> waiting;
    
    for (int itemId = 0; itemId < state.count_items(); itemId++) {
        // Mark this item as waiting
        lr1_item_container  lr1C(new lr1_item(state[itemId], state.lookahead_for(itemId)), true);
        waiting.push(lr1C);
        
        // Add to the result
        // target.insert(lr1);
    }
    
    // Iterate through the set of waiting items
    for (;!waiting.empty(); waiting.pop()) {
        const lr1_item_container& nextItem = waiting.front();
        
        // Take the item apart
        const rule& rule    = *nextItem->rule();
        int         offset  = nextItem->offset();
        
        // No items are generated for an item where the offset is at the end of the rule
        if (offset >= rule.items().size()) continue;
        
        // Get the items added by this entry. The items themselves describe how they affect a LR(0) closure
        lr1_item_set newItems;
        rule.items()[offset]->closure(*nextItem, newItems, *gram);
        
        // Add any new items to the waiting queue
        for (lr1_item_set::iterator it = newItems.begin(); it != newItems.end(); it++) {
            if (target.insert(**it).second) {
                // This is a new item: add it to the list of items waiting to be processed
                waiting.push(*it);
            }
        }
    }
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
    
    // Build the closure for this state
    closure_set stateClosure;
    create_closure(stateClosure, *newState, m_Grammar);
    
    // Add the items from the closure
    for (closure_set::const_iterator closureItem = stateClosure.begin(); closureItem != stateClosure.end(); closureItem++) {
        newState->add_closure(*closureItem);
    }
    
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
    if (newStateId < 0 || newStateId >= m_States.size()) return;
    
    // Set this transition
    m_Transitions[stateId].insert(transition(item, newStateId));
}

/// \brief Adds the specified set of lookahead items to the state with the supplied ID
bool lalr_machine::add_lookahead(int stateId, int itemId, const contextfree::item_set& newLookahead) {
    if (itemId < 0) return false;
    item_set& la = m_States[stateId]->lookahead_for(itemId);
    
    bool result = false;
    for (item_set::const_iterator it = newLookahead.begin(); it != newLookahead.end(); it++) {
        // Ignore the empty item
        if ((*it)->type() == item::empty) continue;
        
        // Otherwise, carry on
        if (la.insert(*it).second) result = true;
    }
    
    return result;
}
     
/// \brief Adds the specified set of lookahead items to the state with the supplied ID
bool lalr_machine::add_lookahead(int stateId, const lr0_item& item, const contextfree::item_set& newLookahead) {
    return add_lookahead(stateId, m_States[stateId]->find_identifier(item), newLookahead);
}
