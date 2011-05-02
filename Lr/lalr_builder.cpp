//
//  lalr_builder.cpp
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <queue>
#include <set>

#include "lalr_builder.h"

using namespace std;
using namespace contextfree;
using namespace lr;

/// \brief Creates a new builder for the specified grammar
lalr_builder::lalr_builder(contextfree::grammar& gram)
: m_Grammar(&gram)
, m_Machine(gram) {
    
}

/// \brief Adds an initial state to this builder that will recognise the language specified by the supplied symbol
///
/// To build a valid parser, you need to add at least one symbol. The builder will add a new state that recognises
/// this language
int lalr_builder::add_initial_state(const contextfree::item_container& language) {
    // Create a new rule for this language ('<language>' $)
    empty_item      empty;
    end_of_input    eoi;
    rule            languageRule(empty);
    
    languageRule << language;
    
    // Create the initial item with an empty lookahead (we only store kernels in the lalr machine)
    lalr_state  initialState;
    lr0_item    item(m_Grammar, languageRule, 0);
    
    int newItemId = initialState.add(item);
    
    // Set the lookahead for this state to be '$'
    initialState.lookahead_for(newItemId).insert(eoi);
    
    // Add this to the machine
    lalr_machine::container c(initialState);
    return m_Machine.add_state(c);
}

/// \brief Set of LR(1) items that represent a closure of a LALR state
typedef set<lr1_item_container> closure_set;

/// \brief Maps an item to the state that's reached when it's encountered
typedef map<item_container, lalr_state_container> state_for_item;

/// \brief Creates the closure for a particular lalr state
static void create_closure(closure_set& target, const lalr_state& state, const grammar* gram) {
    queue<lr1_item_container> waiting;
    
    for (int itemId = 0; itemId < state.count_items(); itemId++) {
        // Mark this item as waiting
        lr1_item            lr1 = lr1_item(state[itemId], state.lookahead_for(itemId));
        lr1_item_container  lr1C(lr1);
        waiting.push(lr1C);
        
        // Add to the result
        target.insert(lr1C);
    }
    
    // Iterate through the set of waiting items
    for (;!waiting.empty(); waiting.pop()) {
        const lr1_item_container& nextItem = waiting.front();
        
        // Take the item apart
        const rule& rule    = nextItem->rule();
        int         offset  = nextItem->offset();
        
        // No items are generated for an item where the offset is at the end of the rule
        if (offset >= rule.items().size()) continue;
        
        // Get the items added by this entry. The items themselves describe how they affect a LR(0) closure
        lr1_item_set newItems;
        rule.items()[offset]->closure(*nextItem, newItems, *gram);
        
        // Add any new items to the waiting queue
        for (lr1_item_set::iterator it = newItems.begin(); it != newItems.end(); it++) {
            if (target.insert(*it).second) {
                // This is a new item: add it to the list of items waiting to be processed
                waiting.push(*it);
            }
        }
    }
}

/// \brief Finishes building the parser (the LALR machine will contain a LALR parser after this call completes)
void lalr_builder::complete_parser() {
    // Queue of states that still need to be processed
    queue<int> waitingStates;
    
    // Begin by filling the queue of states with all the states that are defined
    for (int x=0; x<m_Machine.count_states(); x++) {
        waitingStates.push(x);
    }
    
    // Keep track of the highest state we've encountered (so we can establish when a state creates a new entry in the LALR machine)
    int maxState = m_Machine.count_states();
    
    // Iterate until there are no new states
    while (!waitingStates.empty()) {
        // Fetch the next state to process, and remove the last state from the queue
        int                 nextStateId = waitingStates.front();
        const lalr_state&   nextState   = *m_Machine.state_with_id(nextStateId);
        waitingStates.pop();
        
        // Generate the closure for this state
        closure_set closure;
        create_closure(closure, nextState, m_Grammar);
        
        // Work out the transitions by inspecting each item in the closure (ie, generate the kernels reached from this state)
        state_for_item newStates;
        
        for (closure_set::iterator item = closure.begin(); item != closure.end(); item++) {
            // Take the item apart
            const rule& rule    = (*item)->rule();
            int         offset  = (*item)->offset();
            
            // Items at the end of a rule don't produce any transitions
            if (offset == rule.items().size()) continue;
            
            // Get the item that the 'dot' is before
            const item_container& dottedItem = rule.items()[offset];
            
            // Other items produce a transition on the item that's being pointed at
            // Ie, if we have an item A -> b * c d, we add a transition on 'c' to a new item A -> b c * d 
            lr0_item            transitItem(**item, offset+1);
            lr0_item_container  transitItemContainer(transitItem);
            
            // Add this transition for the appropriate item
            newStates[dottedItem]->add(transitItemContainer);
        }
        
        // Add the new states (and transitions) to the machine
        for (state_for_item::iterator nextState = newStates.begin(); nextState != newStates.end(); nextState++) {
            // Add the state that was generated for this item
            int targetState = m_Machine.add_state(nextState->second);
            
            // Add a transition for this item
            m_Machine.add_transition(nextStateId, nextState->first, targetState);
            
            // If this is a new state then add it to the list that need processing
            if (targetState >= maxState) {
                maxState = targetState+1;
                waitingStates.push(targetState);
            }
        }
    }
}
