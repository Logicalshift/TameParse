//
//  lalr_builder.cpp
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#undef TRACE

#ifdef TRACE
#include <iostream>
#include "Language/formatter.h"

using namespace language;
#endif

#include <queue>
#include <set>

#include "lalr_builder.h"

using namespace std;
using namespace contextfree;
using namespace lr;

/// \brief Creates a new builder for the specified grammar
lalr_builder::lalr_builder(contextfree::grammar& gram, contextfree::terminal_dictionary& terminals)
: m_Grammar(&gram)
, m_Terminals(&terminals)
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

/// \brief Maps an item to the state that's reached when it's encountered
typedef map<item_container, lalr_state_container> state_for_item;

/// \brief Finishes building the parser (the LALR machine will contain a LALR parser after this call completes)
void lalr_builder::complete_parser() {
    empty_item      empty;

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
        
        // Work out the transitions by inspecting each item in the closure (ie, generate the kernels reached from this state)
        state_for_item newStates;
        
        for (lalr_state::all_iterator item = nextState.begin_all(); item != nextState.end_all(); item++) {
            // Take the item apart
            const rule& rule    = *(*item)->rule();
            int         offset  = (*item)->offset();
            
            // Items at the end of a rule don't produce any transitions
            if (offset == rule.items().size()) continue;
            
            // Get the item that the 'dot' is before
            const item_container& dottedItem = rule.items()[offset];
            
            // Don't produce a transition for this item if it doesn't specify that one should be produced
            if (!dottedItem->generate_transition()) continue;
            
            // Other items produce a transition on the item that's being pointed at
            // Ie, if we have an item A -> b * c d, we add a transition on 'c' to a new item A -> b c * d 
            lr0_item                transitItem(**item, offset+1);
            lr0_item_container      transitItemContainer(transitItem);
            
            // Add this transition for the appropriate item
            lalr_state_container& lalrState = newStates[dottedItem];
            lalrState->add(transitItemContainer);
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
    
    // Need the lookaheads to build a complete parser
    complete_lookaheads();
}
 
/// \brief Generates the lookaheads for the parser (when the machine has been built up as a LR(0) grammar)
void lalr_builder::complete_lookaheads() {
    empty_item      empty;
    item_container  empty_c(&empty, false);

    // Now we know all of the states, we need to generate the spontaneous items and work out how items propagate
    // We build closures for the items all over again here, which seems wasteful given than we have to do it
    // to build the original set. I think we can generate the spontaneous items inside the above algorithm, but
    // you need to do merging, and you also need to track the kernels for each item generated by the closure
    // in order to do propagation.
    //
    // Try this for now, we can work on merging if it's slow
    //
    // We start with only the initial states, with a lookahead of '$'
    
    // Create an empty lookahead set (we'll use this a lot)
    item_set emptyLookahead;
    emptyLookahead.insert(empty);
    
    // Create the propagation map
    m_Propagate.clear();
    
#ifdef TRACE
    for (int stateId = 0; stateId < m_Machine.count_states(); stateId++) {
        const lalr_state_container&         thisState   = m_Machine.state_with_id(stateId);

        wcerr << L"BUILDER: LR(0) kernel state #" << stateId << endl;
        for (lalr_state::set_iterator itemId = thisState->begin_kernel(); itemId != thisState->end_kernel(); itemId++) {
            wcerr << L"  BUILDER: " << formatter::to_string(*itemId->first, gram(), terminals()) << endl;
        }
    }
#endif
    
    // Iterate through the states, and generate spontaneous lookaheads and also the propagation table
    for (int stateId = 0; stateId < m_Machine.count_states(); stateId++) {
        // Get the state object
        const lalr_state_container&         thisState   = m_Machine.state_with_id(stateId);
        const lalr_machine::transition_set& transitions = m_Machine.transitions_for_state(stateId);
        
        // Iterate through the items in this state
        for (int itemId = 0; itemId < thisState->count_items(); itemId++) {
            // Get the item
            const lalr_state::container& thisItem = (*thisState)[itemId];
            
            const rule& thisRule    = *thisItem->rule();
            int         thisOffset  = thisItem->offset();
            
            // Ignore items that are at the end (ie, have no closure)
            if (thisOffset >= thisRule.items().size()) continue;
            
            // Get the parser symbol at this offset
            const item_container& symbol = thisRule.items()[thisOffset];
            
            // Get the closure for this item, with an empty item in the lookahead
            lr1_item lr1(thisItem, emptyLookahead);
            lr1_item_set closure;
            
            closure.insert(lr1);
            symbol->closure(lr1, closure, *m_Grammar);
            
            // Iterate through the items in the closure
            for (lr1_item_set::iterator it = closure.begin(); it != closure.end(); it++) {
                const rule& closeRule   = *(*it)->rule();
                const int   closeOffset = (*it)->offset();

                // Ignore this item if it's at the end
                if (closeOffset >= closeRule.items().size()) continue;
                
                const item_container& closeSymbol = closeRule.items()[closeOffset];
                
                // If this doesn't contain the empty item, then spontaneously generate lookahead for this transition
                lalr_machine::transition_set::const_iterator targetState = transitions.find(closeSymbol);
                if (targetState == transitions.end()) continue;

                // Generated item
                const item_set& lookahead       = (*it)->lookahead();
                lr0_item        generated(**it, closeOffset+1);
                int             targetItemId    = m_Machine.state_with_id(targetState->second)->find_identifier(generated);
                
                if (targetItemId < 0) continue;
                
                // Add the lookahead for our symbol to this item (spontaneously generated)
                m_Machine.add_lookahead(targetState->second, targetItemId, lookahead);
                
                // This creates a propagation if the empty item is in the lookahead
                if (lookahead.find(empty_c) != lookahead.end()) {
                    // Add a propagation for this item
                    m_Propagate[lr_item_id(stateId, itemId)].insert(lr_item_id(targetState->second, targetItemId));
                }
            }
        }
    }

#ifdef TRACE
    for (int stateId = 0; stateId < m_Machine.count_states(); stateId++) {
        const lalr_state_container&         thisState   = m_Machine.state_with_id(stateId);
        
        wcerr << L"BUILDER: LALR spontaneous state #" << stateId << endl;
        for (int itemId = 0; itemId < thisState->count_items(); itemId++) {
            wcerr << L"  BUILDER: " << formatter::to_string(*(*thisState)[itemId], gram(), terminals()) << L" " << formatter::to_string(thisState->lookahead_for(itemId), gram(), terminals()) << endl;
        }
    }
#endif

    // Create set of items to do propagation from (we use a set rather than a queue so we don't re-add states multiple times)
    set<lr_item_id> toPropagate;
    
    // Fill with all the states which do propagation
    for (propagation::iterator it = m_Propagate.begin(); it != m_Propagate.end(); it++) {
#ifdef TRACE
        wcerr << L"BUILDER: should propagate from " << it->first.first << L": " << formatter::to_string(*(*m_Machine.state_with_id(it->first.first))[it->first.second], gram(), terminals()) << endl;
        
        for (set<lr_item_id>::iterator path = it->second.begin(); path != it->second.end(); path++) {
            wcerr << L"  BUILDER: to " << path->first << L": " << formatter::to_string(*(*m_Machine.state_with_id(path->first))[path->second], gram(), terminals()) << endl;
        }

#endif

        toPropagate.insert(it->first);
    }
    
    // While there are still states to process, do propagation
    while (!toPropagate.empty()) {
        // Get the next state we need to do propagation on
        lr_item_id nextState = *(toPropagate.begin());
        
        // Remove from the set waiting to be processed
        toPropagate.erase(nextState);
        
        // Fetch out the items
        set<lr_item_id>& propItems = m_Propagate[nextState];
        
        // Get the lookahead for this item
        const item_set& itemLookahead = m_Machine.state_with_id(nextState.first)->lookahead_for(nextState.second);

        // Perform propagation
        for (set<lr_item_id>::iterator path = propItems.begin(); path != propItems.end(); path++) {
            // Propagating lookahead from the item identified by nextState to *path
            if (m_Machine.add_lookahead(path->first, path->second, itemLookahead)) {
                // If the lookahead changed things, then we'll need to propagate the changed lookahead from the item specified by path
                if (m_Propagate.find(*path) != m_Propagate.end()) {
                    toPropagate.insert(*path);
                }
            }
        }
    }
}


/// \brief Adds a new action rewriter to this builder
void lalr_builder::add_rewriter(const action_rewriter_container& rewriter) {
    m_ActionRewriters.push_back(rewriter);
}

/// \brief Replaces the rewriters that this builder will use
void lalr_builder::set_rewriters(const action_rewriter_list& list) {
    m_ActionRewriters = list;
}

/// \brief After the state machine has been completely built, returns the actions for the specified state
///
/// If there are conflicts, this will return multiple actions for a single symbol.
const lr_action_set& lalr_builder::actions_for_state(int state) const {
    // Try to find an existing action
    map<int, lr_action_set>::const_iterator existing = m_ActionsForState.find(state);
    if (existing != m_ActionsForState.end()) return existing->second;
    
    // Build up a new set
    typedef lalr_machine::transition_set    transition_set;
    typedef lr1_item::lookahead_set         lookahead_set;
    
    lr_action_set&          newSet      = m_ActionsForState[state];
    const transition_set&   transits    = m_Machine.transitions_for_state(state);
    const lalr_state&       thisState   = *m_Machine.state_with_id(state);
    
    // Add a shift action for each transition
    for (transition_set::const_iterator it = transits.begin(); it != transits.end(); it++) {
        // Get the item being shifted
        const item_container&   thisItem    = it->first;
        int                     targetState = it->second;
        
        // Add a new shift or goto action (terminals are shift actions, everything else is a goto)
        lr_action::action_type actionType = lr_action::act_goto;
        if (thisItem->type() == item::terminal) {
            actionType = lr_action::act_shift;
        }        
        
        lr_action newAction(actionType, thisItem, targetState);
        newSet.insert(newAction);
    }
    
    // For any LR items that are at the end of their rule, generate a reduce action for the appropriate symbols
    for (lalr_state::all_iterator lrItem = thisState.begin_all(); lrItem != thisState.end_all(); lrItem++) {
        // Ignore items that aren't at the end
        if (!(*lrItem)->at_end()) continue;
        
        // This item needs to be a reduce action for all of its lookahead symbols
        const lookahead_set* la = thisState.lookahead_for(*lrItem);
        if (!la) continue;
        
        const rule_container& rule = (*lrItem)->rule();
        
        // Use the accepting action if the target symbol is 'empty' (which we use a placeholder for a symbol representing a language)
        // TODO: use a dedicated item type instead of overloading the empty symbol
        lr_action::action_type actionType = lr_action::act_reduce;
        if (rule->nonterminal()->type() == item::empty) {
            actionType = lr_action::act_accept;
        }
        
        for (lookahead_set::const_iterator reduceSymbol = la->begin(); reduceSymbol != la->end(); reduceSymbol++) {
            // Generate a reduce action for this symbol
            lr_action newAction(actionType, *reduceSymbol, -1, rule);
            newSet.insert(newAction);
        }
    }
    
    // Rewrite this list of actions according to the action rewriters
    for (action_rewriter_list::const_iterator rewrite = m_ActionRewriters.begin(); rewrite != m_ActionRewriters.end(); rewrite++) {
        (*rewrite)->rewrite_actions(state, newSet, *this);
    }
    
    // Return this as the result
    return newSet;
}

/// \brief Returns the items that the lookaheads are propagated to for a particular item in this state machine
const std::set<lalr_builder::lr_item_id>& lalr_builder::propagations_for_item(int state, int item) const {
    return m_Propagate[lr_item_id(state, item)];
}
