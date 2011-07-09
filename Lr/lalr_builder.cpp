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
    lalr_state* initialState = new lalr_state();
    lr0_item    item(m_Grammar, languageRule, 0);
    
    int newItemId = initialState->add(item);
    
    // Set the lookahead for this state to be '$'
    initialState->lookahead_for(newItemId).insert(eoi);
    
    // Add this to the machine
    lalr_machine::container c(initialState, true);
    return m_Machine.add_state(c);
}

/// \brief Maps an item to the state that's reached when it's encountered
typedef map<item_container, lalr_state_container> state_for_item;

/// \brief Creates the closure for a particular lalr state
void lalr_builder::create_closure(closure_set& target, const lalr_state& state, const grammar* gram) {
    queue<lr1_item_container> waiting;
    
    for (int itemId = 0; itemId < state.count_items(); itemId++) {
        // Mark this item as waiting
        lr0_item_container  lr0 = state[itemId];
        lr1_item_container  lr1(new lr1_item(state[itemId], state.lookahead_for(itemId)), true);
        waiting.push(lr1);
        
        // Add to the result
        target.insert(lr0);
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

        // Generate the closure for this state
        closure_set closure;
        create_closure(closure, nextState, m_Grammar);

        // Work out the transitions by inspecting each item in the closure (ie, generate the kernels reached from this state)
        state_for_item newStates;
        
        for (closure_set::const_iterator item = closure.begin(); item != closure.end(); item++) {
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
            
            // Guard items produce a guard rule initial state, if there isn't one already
            if (dottedItem->type() == item::guard) {
                // Get the underlying guard object
                const guard* thisGuard = dynamic_cast<const guard*>(dottedItem.item());
                if (thisGuard != NULL) {
                    // Get the rule ID
                    int ruleId = thisGuard->get_rule()->identifier(*m_Grammar);
                    
                    // If there's no state defined for this rule, then define a new one
                    if (m_StatesForGuard.find(ruleId) == m_StatesForGuard.end()) {
                        // Create a state for this rule
                        lalr_state*         guardState = new lalr_state();
                        lr0_item_container  guardItem(new lr0_item(m_Grammar, thisGuard->get_rule(), 0), true);
                        
                        int guardItemId = guardState->add(guardItem);
                        
                        // Set the lookahead to be '$'
                        end_of_guard eog;
                        guardState->lookahead_for(guardItemId).insert(eog);
                        
                        // Add the state
                        lalr_state_container guardStateContainer(guardState, true);
                        int guardStateId = m_Machine.add_state(guardStateContainer);
                        
                        // Add this as a state to be processed
                        waitingStates.push(guardStateId);
                        
                        // Store as a known state
                        m_StatesForGuard[ruleId] = guardStateId;
                    }
                }
            }
            
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
            
            // Create the set of spontaneous lookahead items
            set<lr_item_id>& spontaneousTargets = m_Spontaneous[lr_item_id(stateId, itemId)];
            
            // Iterate through the items in the closure
            for (lr1_item_set::iterator it = closure.begin(); it != closure.end(); it++) {
                const rule& closeRule   = *(*it)->rule();
                const int   closeOffset = (*it)->offset();

                // Ignore this item if it's at the end
                if (closeOffset >= closeRule.items().size()) continue;
                
                const item_container& closeSymbol = closeRule.items()[closeOffset];
                
                // Spontaneously generate lookaheads for this transition
                //  -- We have an item a = b ^ C d (which was generated as part of the closure)
                //  -- The lookahead for this item is copied to the item a = b C ^ d in the state reached by matching C
                lalr_machine::transition_set::const_iterator targetState = transitions.find(closeSymbol);
                if (targetState == transitions.end()) continue;

                // Generated item
                const item_set& lookahead       = (*it)->lookahead();
                lr0_item        generated(**it, closeOffset+1);
                int             targetItemId    = m_Machine.state_with_id(targetState->second)->find_identifier(generated);
                
                if (targetItemId < 0) continue;
                
                // Add the lookahead for our symbol to this item (spontaneously generated)
                m_Machine.add_lookahead(targetState->second, targetItemId, lookahead);
                
                // If the lookahead is not empty, or isn't just the empty item, then add to the spontaneous set
                if (!lookahead.empty() || lookahead.size() > 1 || lookahead.find(empty_c) == lookahead.end()) {
                    spontaneousTargets.insert(lr_item_id(targetState->second, targetItemId));
                }
                
                // This creates a propagation if the empty item is in the lookahead
                //   -- We have an item a = b ^ C d
                //   -- This item was part of the closure for item e = f ^ g h
                //   -- (ie, g expands to a = b ^ C d, and h can be empty)
                //   -- We copy the lookahead from e = f ^ g h to the item a = b C ^ d in the transition for C
                //   -- e = f ^ g might also have lookahead propagated from elsewhere: we need to copy the lookahead
                //      from these items as well
                if (lookahead.find(empty_c) != lookahead.end()) {
                    // Add a propagation for this item
                    m_Propagate[lr_item_id(stateId, itemId)].insert(lr_item_id(targetState->second, targetItemId));
                }
            }
        }
    }

    // Create set of items to do propagation from (we use a set rather than a queue so we don't re-add states multiple times)
    set<lr_item_id> toPropagate;
    
    // Fill with all the states which do propagation
    for (propagation::iterator it = m_Propagate.begin(); it != m_Propagate.end(); it++) {
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


/// \brief Adds the closure of the specified LALR state to the specified set
void lalr_builder::generate_closure(const lalr_state& state, lr1_item_set& closure, const contextfree::grammar* gram) {
    typedef lr1_item::lookahead_set lookahead_set;
    queue<lr1_item_container>       waitingForClosure;
    
    // Add the initial items
    for (lalr_state::iterator lrItem = state.begin(); lrItem != state.end(); lrItem++) {
        // Create an LR(1) item from the LALR machine
        const lookahead_set*    la      = state.lookahead_for(*lrItem);
        if (!la) continue;
        
        lr1_item_container      nextItem(new lr1_item(*lrItem, *la), true);
        
        // Add to the closure
        closure.insert(nextItem);
        waitingForClosure.push(nextItem);
    }
    
    // Add the closure of the items that are waiting
    for (;!waitingForClosure.empty(); waitingForClosure.pop()) {
        // Get the next item for adding to the closure
        lr1_item_container& nextItem = waitingForClosure.front();
        
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
            if (closure.insert(**it).second) {
                // This is a new item: add it to the list of items waiting to be processed
                waitingForClosure.push(*it);
            }
        }
    }
}

/// \brief Adds guard actions appropriate for the specified guard item
void lalr_builder::add_guard(const item_container& item, lr_action_set& newSet) const {
    const guard* thisGuard = dynamic_cast<const guard*>(item.item());
    if (thisGuard == NULL) return;
    
    // Must have an associated state ID
    int                             guardRuleId     = thisGuard->get_rule()->identifier(*m_Grammar);
    map<int, int>::const_iterator   guardStateId    = m_StatesForGuard.find(guardRuleId);
    
    if (guardStateId == m_StatesForGuard.end()) return;
    
    // Fetch the initial set for this guard symbol
    item_set initial = thisGuard->initial(*m_Grammar);
    
    // Add guard actions for each of the terminal items in initial
    for (item_set::iterator initialSym = initial.begin(); initialSym != initial.end(); initialSym++) {
        // Only terminals
        if ((*initialSym)->type() != item::terminal) continue;
        
        // Add a guard action for this item
        newSet.insert(lr_action_container(new lr_action(lr_action::act_guard, *initialSym, guardStateId->second, thisGuard->get_rule()), true));
    }
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
    
    // Create the LR(1) closure for this state
    // If none of the items in the state have an empty item, then this is unnecessary (this is only required to
    // create the reductions for these items)
    lr1_item_set closure;
    generate_closure(thisState, closure, m_Grammar);
    
    // For each transition on a guarded symbol, add a guard transition to check for it
    for (transition_set::const_iterator maybeGuard = transits.begin(); maybeGuard != transits.end(); maybeGuard++) {
        // Get the item for this transition
        const item_container&   thisItem    = maybeGuard->first;
        
        // Must be a guard
        if (thisItem->type() != item::guard) continue;
        
        // Add the guard actions for this item
        add_guard(thisItem, newSet);
    }
    
    // Add a shift action for each transition
    for (transition_set::const_iterator it = transits.begin(); it != transits.end(); it++) {
        // Get the item being shifted
        const item_container&   thisItem    = it->first;
        int                     targetState = it->second;
        
        // Add a new shift or goto action (terminals are shift actions, everything else is a goto)
        lr_action::action_type actionType = lr_action::act_goto;
        if (thisItem->type() == item::terminal || thisItem->type() == item::guard) {
            actionType = lr_action::act_shift;
        }
        
        lr_action newAction(actionType, thisItem, targetState);
        newSet.insert(newAction);
    }
    
    // For any LR items that are at the end of their rule, generate a reduce action for the appropriate symbols
    for (lr1_item_set::iterator lrItem = closure.begin(); lrItem != closure.end(); lrItem++) {
        // Ignore items that aren't at the end
        if (!(*lrItem)->at_end()) continue;
        
        // This item needs to be a reduce action for all of its lookahead symbols
        const lookahead_set& la = (*lrItem)->lookahead();
        
        const rule_container& rule = (*lrItem)->rule();
        
        // Use the accepting action if the target symbol is 'empty' (which we use a placeholder for a symbol representing a language)
        // Also use the accepting action for guards
        // TODO: use a dedicated item type instead of overloading the empty symbol
        lr_action::action_type actionType = lr_action::act_reduce;
        if (rule->nonterminal()->type() == item::empty || rule->nonterminal()->type() == item::guard) {
            actionType = lr_action::act_accept;
        }
        
        for (lookahead_set::const_iterator reduceSymbol = la.begin(); reduceSymbol != la.end(); reduceSymbol++) {
            // We don't produce actions for nonterminal items (the default closures do add these to the follow set, though)
            // Guards also produce reduce actions
            int reduceSymbolType = (*reduceSymbol)->type();
            if (reduceSymbolType    != item::terminal 
                && reduceSymbolType != item::eoi 
                && reduceSymbolType != item::eog 
                && reduceSymbolType != item::guard)
                continue;
            
            // For reductions that act on guards, produce appropriate guard actions
            if (reduceSymbolType == item::guard) {
                // Add the guard actions for this item
                add_guard(*reduceSymbol, newSet);
            }
            
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
