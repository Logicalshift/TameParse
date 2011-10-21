//
//  conflict.cpp
//  Parse
//
//  Created by Andrew Hunter on 04/06/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/Lr/conflict.h"

using namespace std;
using namespace contextfree;
using namespace lr;

/// \brief Creates a new conflict object (describing a non-conflict)
conflict::conflict(int stateId, const item_container& token)
: m_StateId(stateId)
, m_Token(token) {
}

/// \brief Copy constructor
conflict::conflict(const conflict& copyFrom)
: m_StateId(copyFrom.m_StateId)
, m_Token(copyFrom.m_Token)
, m_Shift(copyFrom.m_Shift)
, m_Reduce(copyFrom.m_Reduce) {
}

/// \brief Destroys this conflict object
conflict::~conflict() { }

/// \brief Clones this conflict
conflict* conflict::clone() const {
    return new conflict(*this);
}

/// \brief Returns true if conflict a is less than conflict b
bool conflict::compare(const conflict* a, const conflict* b) {
    if (a == b) return false;
    if (!a) return true;
    if (!b) return false;
    
    return (*a) < (*b);
}

/// \brief Orders this conflict relative to another
bool conflict::operator<(const conflict& compareTo) const {
    if (m_StateId < compareTo.m_StateId)    return true;
    if (m_StateId > compareTo.m_StateId)    return false;
    
    if (m_Token < compareTo.m_Token)        return true;
    if (compareTo.m_Token < m_Token)        return false;
    
    if (m_Shift < compareTo.m_Shift)        return true;
    if (m_Shift > compareTo.m_Shift)        return false;
    
    if (m_Reduce < compareTo.m_Reduce)      return true;
    
    return false;
}

/// \brief Adds an LR(0) item that will be followed if the token is shifted
void conflict::add_shift_item(const lr0_item_container& item) {
    m_Shift.insert(item);
}

/// \brief Adds an LR(0) item that will be reduced if the token is in the lookahead
///
/// This returns an item that the caller can add the set of reduce states for this item
conflict::possible_reduce_states& conflict::add_reduce_item(const lr0_item_container& item) {
    return m_Reduce[item];
}

/// \brief Adds the conflicts found in a single state of the specified LALR builder to the given target list
static void find_conflicts(const lalr_builder& builder, int stateId, conflict_list& target) {
    // Get the actions in this state
    const lr_action_set& actions = builder.actions_for_state(stateId);
    
    // Run through these actions, and find places where there are conflicts (two actions for a single symbol)
    typedef map<item_container, lr_action_set> items_to_actions;
    items_to_actions actionsForItem;
    
    for (lr_action_set::const_iterator nextAction = actions.begin(); nextAction != actions.end(); nextAction++) {
        // Ignore actions that don't cause conflicts
        bool ignore;
        switch ((*nextAction)->type()) {
            case lr_action::act_goto:
            case lr_action::act_ignore:
            case lr_action::act_accept:
                ignore = true;
                break;
                
            default:
                ignore = false;
                break;
        }
        if (ignore) continue;
        
        // Add this as an action for this item
        actionsForItem[(*nextAction)->item()].insert(*nextAction);
    }
    
    // Generate conflicts for any item with multiple actions
    for (items_to_actions::iterator nextItem = actionsForItem.begin(); nextItem != actionsForItem.end(); nextItem++) {
        // Ignore items with just one action (or no actions, if that's ever possible)
        if (nextItem->second.size() < 2) continue;
        
        // Ignore items if all but one of the items are 'weak'
        int numWeak = 0;
        for (lr_action_set::const_iterator nextAction = nextItem->second.begin(); nextAction != nextItem->second.end(); nextAction++) {
            switch ((*nextAction)->type()) {
                case lr_action::act_guard:
                case lr_action::act_weakreduce:
                case lr_action::act_ignore:
                    numWeak++;
                    break;
                    
                default:
                    break;
            }
        }
        
        if (nextItem->second.size() - numWeak < 2) continue;
        
        // Create a new conflict for this action
        const item_container&   conflictToken = *nextItem->first;
        conflict_container      newConf(new conflict(stateId, conflictToken), true);
        
        // Fetch the items in this state
        const lalr_state& thisState = *builder.machine().state_with_id(stateId);
        
        // Get the closure of this state
        lr1_item_set closure;
        builder.generate_closure(thisState, closure, &builder.gram());
        
        // Describe the actions resulting in this conflict by going through the items in the closure of the state
        const grammar*    gram = &builder.gram();
        
        // Iterate through the closure to get the items that can be shifted as part of this conflict
        for (lr1_item_set::const_iterator nextItem = closure.begin(); nextItem != closure.end(); nextItem++) {
            if (!(*nextItem)->at_end()) {
                // This item will result in a shift: add it to the list if it can shift our item
                const item_set& firstItems = gram->first((*nextItem)->rule()->items()[(*nextItem)->offset()]);
                
                if (firstItems.contains(conflictToken)) {
                    // This item can result in a shift of the specified token
                    newConf->add_shift_item(**nextItem);
                }
            }
        }

        // Iterate through the items in the state to look for the reduce actions
        for (int itemId = 0; itemId < thisState.count_items(); itemId++) {
            // Get the item that this refers to
            const lalr_state::container& thisItem = thisState[itemId];

            // Get the lookahead for this item
            const lr1_item::lookahead_set& la = *thisState.lookahead_for(thisItem);

            // If this state isn't a reducing state, then compute the closure to see if it produces any reducing states on the conflicted nonterminal
            if (!thisItem->at_end()) {
                // Fetch the rule for this item
                const rule& rule    = *thisItem->rule();
                int         offset  = thisItem->offset();

                // Generate the closure for this item
                lr1_item        nextItem(thisItem, la);
                lr1_item_set    itemClosure;

                rule.items()[offset]->cache_closure(nextItem, itemClosure, *gram);

                // For any items in the closure that are at the end and have a reduce action, generate some reduce information
                for (lr1_item_set::iterator closureItem = itemClosure.begin(); closureItem != itemClosure.end(); closureItem++) {
                    // Ignore items that are not at the end of the closure
                    if (!(*closureItem)->at_end()) continue;

                    // Ignore items that do not contain the closure item in their lookahead
                    if (!(*closureItem)->lookahead().contains(conflictToken)) continue;

                    // This item is part of the conflict: add a new reduce item
                    conflict::possible_reduce_states& reduceTo = newConf->add_reduce_item(**closureItem);

                    // Add a reduction action for this (for this nonterminal, we just end up in the same state)
                    reduceTo.insert(conflict::lr_item_id(stateId, itemId));
                }

                // TODO: if the symbols following the current state can be reduced to the empty symbol then we need to indicate where the rest of the lookahead comes from
                continue;
            }

            // This is a reducing state

            // This isn't part of the conflict if it isn't being reduced based on the 
            if (!la.contains(conflictToken)) continue;

            // Add a conflict for this item
            conflict::possible_reduce_states& reduceTo = newConf->add_reduce_item(thisItem);

            // Work out which states can be reached by looking at the spontaneous and lookahead propagation
            typedef set<conflict::lr_item_id>  lr_item_id_set;
            lr_item_id_set sourceItems;
            builder.find_lookahead_source(stateId, itemId, conflictToken, sourceItems);

            // Add to the reduce set
            reduceTo.insert(sourceItems.begin(), sourceItems.end());
        }
        
        // Ignore conflicts that contain no items at all
        if (newConf->first_reduce_item() == newConf->last_reduce_item() && newConf->first_shift_item() == newConf->last_shift_item()) {
            // These items are generally caused by things like weak symbols where the action was generated spontaneously from a strong symbol
            continue;
        }
        
        // Add the new conflict to the list
        target.push_back(newConf);
    }
}

/// \brief Adds the conflicts found in the specified LALR builder object to the passed in list
void conflict::find_conflicts(const lalr_builder& builder, conflict_list& target) {
    // Iterate through the states in the builder
    for (int stateId=0; stateId < builder.count_states(); stateId++) {
        ::find_conflicts(builder, stateId, target);
    }
}

