//
//  ignored_symbols.cpp
//  Parse
//
//  Created by Andrew Hunter on 21/05/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#include "TameParse/Lr/ignored_symbols.h"

using namespace contextfree;
using namespace lr;

/// \brief Creates a new ignored symbols rewriter
ignored_symbols::ignored_symbols() {
    
}

/// \brief Copy constructor
ignored_symbols::ignored_symbols(const ignored_symbols& copyFrom) 
: m_IgnoredItems(copyFrom.m_IgnoredItems) {
}

/// \brief Destructor
ignored_symbols::~ignored_symbols() {
}

/// \brief Adds a new ignored item to this object
///
/// The new item should be a terminal, as it doesn't make sense in the current parser design to ignore
/// other item types.
void ignored_symbols::add_item(const contextfree::item_container& newItem) {
    m_IgnoredItems.insert(newItem);
}

/// \brief Creates a clone of this rewriter
action_rewriter* ignored_symbols::clone() const {
    return new ignored_symbols(*this);
}

/// \brief Modifies the specified set of actions according to the rules in this rewriter
void ignored_symbols::rewrite_actions(int state, lr_action_set& actions, const lalr_builder& builder) const {
    // We generate a new actions set, and replace the existing actions at the end
    lr_action_set newActions;
    
    // Make a note of any ignored symbols that are used in shift actions, and also change any reduce actions
    // that reference an ignored symbol to a weak reduce action
    item_set usedIgnored;
    for (lr_action_set::const_iterator action = actions.begin(); action != actions.end(); ++action) {
        switch ((*action)->type()) {
            case lr_action::act_reduce:
                if (m_IgnoredItems.find((*action)->item()) != m_IgnoredItems.end()) {
                    // Replace this action with a weak reduce
                    lr_action weakReduce(lr_action::act_weakreduce, (*action)->item(), (*action)->next_state(), (*action)->rule());
                    
                    newActions.insert(weakReduce);
                } else {
                    // Just copy this action
                    newActions.insert(*action);
                }
                break;
                
            case lr_action::act_shift:
                // Shifts always get copied through
                newActions.insert(*action);
                
                // This is a 'used ignored' if thje item is in the ignored list
                if (m_IgnoredItems.find((*action)->item()) != m_IgnoredItems.end()) {
                    usedIgnored.insert((*action)->item());
                }
                break;
                
            case lr_action::act_ignore:
                // Ignored actions always copied through
                newActions.insert(*action);
                
                // Add to usedIgnored to prevent this symbol appearing multiple times
                usedIgnored.insert((*action)->item());
                break;
                
            default:
                // All other actions get copied through
                newActions.insert(*action);
                break;
        }
    }
    
    // Add 'ignore' actions for any unused ignored item
    for (item_set::const_iterator ignored = m_IgnoredItems.begin(); ignored != m_IgnoredItems.end(); ++ignored) {
        if (usedIgnored.find(*ignored) == usedIgnored.end()) {
            // Create a new ignored action
            lr_action newIgnored(lr_action::act_ignore, *ignored, state);
            
            // Add to the new action list
            newActions.insert(newIgnored);
        }
    }
    
    // Set the actions to the new set
    actions = newActions;
}
