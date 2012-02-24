//
//  precedence_rewriter.cpp
//  TameParse
//
//  Created by Andrew Hunter on 21/01/2012.
//  Copyright (c) 2012 Andrew Hunter. All rights reserved.
//

#include <set>

#include "TameParse/Lr/precedence_rewriter.h"

using namespace std;
using namespace contextfree;
using namespace lr;

/// \brief Modifies the specified set of actions according to the rules in this rewriter
///
/// This call should modify the contents of the supplied action set according to whatever rules it considers 
/// suitable.
/// 
/// Typical rewriting actions might include removing actions so that ambiguous grammars can be accepted, replacing
/// rules with alternatives (such as weak reduce actions for cases where a grammar can be parsed as LR(1)) or
/// adding new rules.
void precedence_rewriter::rewrite_actions(int stateId, lr_action_set& actions, const lalr_builder& builder) const {
    // Define types
    typedef map<item_container, lr_action_set> actions_for_item;

    // Get some information about this state
    const lalr_state& state = *builder.machine().state_with_id(stateId);

    // Find the actions for all of the items
    actions_for_item itemActions;

    for (lr_action_set::iterator act = actions.begin(); act != actions.end(); ++act) {
        // Only interested in actions for terminals
        if ((*act)->item()->type() != item::terminal) continue;

        // Add this action
        itemActions[(*act)->item()].insert(*act);
    }

    // Find items that have shift/reduce conflicts
    for (actions_for_item::iterator itemAct = itemActions.begin(); itemAct != itemActions.end(); ++itemAct) {
        // Get the shift and reduce sides of this action
        lr_action_set::const_iterator   shift;
        lr_action_set::const_iterator   reduce;
        int                             numShift    = 0;
        int                             numReduce   = 0;

        for (lr_action_set::const_iterator act = itemAct->second.begin(); act != itemAct->second.end(); ++act) {
            if ((*act)->type() == lr_action::act_shift) {
                shift = act;
                numShift++;
            } else if ((*act)->type() == lr_action::act_reduce) {
                reduce = act;
                numReduce++;
            }
        }

        // Must be a shift/reduce conflict with exactly one shift and one reduce, or we won't try to resolve it
        if (numShift != 1 || numReduce != 1) continue;

        // Fetch the items that the shift applies to
        lr0_item_set                        shiftItems;
        const contextfree::item_container&  terminal = (*shift)->item();

        for (lalr_state::iterator lrItem = state.begin(); lrItem != state.end(); lrItem++) {
            // Only interested in items that will shift the target token
            if ((*lrItem)->at_end())                                            continue;
            if ((*lrItem)->rule()->items()[(*lrItem)->offset()] != terminal)    continue;

            // Add to the set
            shiftItems.insert(*lrItem);
        }

        // Get the precedence of the shift token and the rule being reduced
        int shiftPrec   = get_precedence(terminal, shiftItems);
        int reducePrec  = get_rule_precedence(*(*reduce)->rule());

        // Ignore any items which don't have a precedence
        if (shiftPrec == no_precedence || reducePrec == no_precedence) continue;

        // Perform the action with the lowest precedence
        if (shiftPrec < reducePrec) {
            // Perform the reduce action
            // (a + b - c --> (a + b) - c)
            actions.erase(*shift);
        } else if (shiftPrec > reducePrec) {
            // Perform the shift action
            // (a + b - c --> a + (b - c))
            actions.erase(*reduce);
        } else {
            // When precedences are identical, then look at the associativity for the reduce side of the rule
            associativity reduceAssoc   = get_rule_associativity(*(*reduce)->rule());

            // If the item being reduced is right associative, then resolve as a shift
            // (So a + b + c --> a + (b + c))
            // Note that the second '+' is the place where the conflict is occurring, so
            // its associativity is immaterial at this point.
            if (reduceAssoc == right) {
                actions.erase(*reduce);
            }
            
            // Favour reducing if the item being reduced is left associative
            // (So a + b + c --> (a + b) + c)
            else if (reduceAssoc == left) {
                actions.erase(*shift);
            }
        }
    }
}

/// \brief Finds the item in a rule that defines its precedence
int precedence_rewriter::precedence_item_index(const contextfree::rule& rule) const {
    // Initial position is undefined
    int itemPos = -1;

    // LR(0) item set representing no items
    static lr0_item_set noItems;

    // Check each item in the rule
    for (size_t pos = 0; pos < rule.items().size(); pos++) {
        // Fetch this item
        const item_container& thisItem = rule.items()[pos];

        // Must be a terminal symbol
        if (thisItem->type() != item::terminal) continue;

        // Get this item's precedence
        int itemPrecedence = get_precedence(thisItem, noItems);

        // Ignore this item if it has no precedence
        if (itemPrecedence == no_precedence) continue;

        // We can only work out the precedence for rules containing a single 'operator' item
        if (itemPos != -1) return -1;

        // This is the result
        itemPos = (int) pos;
    }
    
    // Return the position we found
    return itemPos;
}

/// \brief Returns the precedence for a particular rule
int precedence_rewriter::get_rule_precedence(const contextfree::rule& rule) const {
    // LR(0) item set representing no items
    static lr0_item_set noItems;

    // Get the index of the item that defines the precedence for this rule
    int precedenceIndex = precedence_item_index(rule);

    // Result is no precedence if the index is out of range
    if (precedenceIndex < 0) return no_precedence;

    // Return the precedence of the item at the specified index
    return get_precedence(rule.items()[precedenceIndex], noItems);
}

/// \brief Returns the associativity for a particular rule
precedence_rewriter::associativity precedence_rewriter::get_rule_associativity(const contextfree::rule& rule) const {
    // LR(0) item set representing no items
    static lr0_item_set noItems;

    // Get the index of the item that defines the precedence for this rule
    int precedenceIndex = precedence_item_index(rule);

    // Result is no associativity if the index is out of range
    if (precedenceIndex < 0) return nonassociative;

    // Return the associativity of the item at the specified index
    return get_associativity(rule.items()[precedenceIndex], noItems);   
}
