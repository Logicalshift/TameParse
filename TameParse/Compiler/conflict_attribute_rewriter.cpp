//
//  conflict_attribute_rewriter.cpp
//  TameParse
//
//  Created by Andrew Hunter on 08/01/2012.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the \"Software\"), to 
//  deal in the Software without restriction, including without limitation the 
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
//  sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
//  IN THE SOFTWARE.
//

#include <map>
#include <vector>

#include "TameParse/Compiler/conflict_attribute_rewriter.h"

using namespace std;
using namespace contextfree;
using namespace lr;
using namespace yy_language;
using namespace compiler;

/// \brief Creates a new conflict rewriter
conflict_attribute_rewriter::conflict_attribute_rewriter(const rule_item_data* ruleData) 
: m_RuleData(ruleData) {
}

/// \brief Modifies the specified set of actions according to the rules in this rewriter
void conflict_attribute_rewriter::rewrite_actions(int state, lr_action_set& actions, const lalr_builder& builder) const {
    typedef vector<lr_action_container>         action_list;
    typedef map<item_container, action_list>    action_map;

    // Map items to their corresponding actions
    action_map actionsForItem;

    for (lr_action_set::const_iterator act = actions.begin(); act != actions.end(); ++act) {
        // Only interested in terminal items (which can generate shift actions)
        if ((*act)->item()->type() != item::terminal) continue;

        // Push this item back
        actionsForItem[(*act)->item()].push_back(*act);
    }

    // Fetch the current state
    const lalr_state& curState = *builder.machine().state_with_id(state);
    lr1_item_set closure;

    // Search for items with shift/reduce conflicts
    for (action_map::iterator actForItem = actionsForItem.begin(); actForItem != actionsForItem.end(); ++actForItem) {
        // Must be multiple actions for this particular item
        if (actForItem->second.size() < 2) continue;

        // Must be at least one shift/reduce conflict
        // Shift/weak reduce is ignored (presumably some other conflict that has been resolved)
        int numShift    = 0;
        int numReduce   = 0;

        for (action_list::iterator act = actForItem->second.begin(); act != actForItem->second.end(); ++act) {
            if ((*act)->type() == lr_action::act_shift)         numShift++;
            if ((*act)->type() == lr_action::act_shiftstrong)   numShift++;
            if ((*act)->type() == lr_action::act_reduce)        numReduce++;
        }

        // Nothing to do if this is not a shift/reduce conflict
        if (numShift == 0 || numReduce == 0) {
            continue;
        }

        // Generate the closure for this state if it hasn't been generated already
        if (closure.empty()) {
            builder.generate_closure(curState, closure, &builder.gram());
        }

        // Work out the action to perform by inspecting the shift items in the state that correspond to this item
        int resolve = ebnf_item_attributes::conflict_notspecified;

        for (lr1_item_set::const_iterator lrItem = closure.begin(); lrItem != closure.end(); ++lrItem) {
            // Only want items that will produce a shift action
            if ((*lrItem)->at_end()) continue;

            // Get the item that this rule will shift
            const item_container& shiftItem = (*lrItem)->rule()->items()[(*lrItem)->offset()];

            // We only care if this item corresponds to our item
            if (*shiftItem != *actForItem->first) continue;

            // Get the attributes for this item
            const ebnf_item_attributes& attr = m_RuleData->attributes_for(*(*lrItem)->rule(), (*lrItem)->offset());

            // Set the resolution action if it's specified for this item
            // TODO: report a warning if more than one rule can result in a shift and there's multiple actions
            if (attr.conflict_action != resolve) {
                if (attr.conflict_action != ebnf_item_attributes::conflict_notspecified) {
                    resolve = attr.conflict_action;
                }
            }
        }

        // Nothing to do if there is no action
        if (resolve == ebnf_item_attributes::conflict_notspecified) {
            continue;
        }

        // Also, items with reduce/reduce conflicts can only be resolved using 'reduce' (not weak reduce!)
        if (numReduce > 1 && resolve != ebnf_item_attributes::conflict_reduce) {
            continue;
        }

        // We can resolve this conflict.
        for (action_list::iterator act = actForItem->second.begin(); act != actForItem->second.end(); ++act) {
            // Decide if we want to keep this action or not
            if ((*act)->type() != lr_action::act_shift 
                && (*act)->type() != lr_action::act_shiftstrong
                && (*act)->type() != lr_action::act_reduce) {
                // We only act on shift/reduce actions
                continue;
            }

            if (resolve == ebnf_item_attributes::conflict_shift) {
                // Remove reduce actions
                if ((*act)->type() == lr_action::act_reduce) {
                    actions.erase(*act);
                }
            }

            else if (resolve == ebnf_item_attributes::conflict_reduce) {
                // Remove shift actions
                if ((*act)->type() != lr_action::act_reduce) {
                    actions.erase(*act);
                }
            }

            else if (resolve == ebnf_item_attributes::conflict_weakreduce) {
                // Change reduce actions to weak reduce actions
                if ((*act)->type() == lr_action::act_reduce) {
                    actions.erase(*act);

                    lr_action_container weakAct(new lr_action(lr_action::act_weakreduce, (*act)->item(), (*act)->next_state(), (*act)->rule()));
                    actions.insert(weakAct);
                }
            }
        }
    }
}

/// \brief Creates a clone of this rewriter
action_rewriter* conflict_attribute_rewriter::clone() const {
    return new conflict_attribute_rewriter(m_RuleData);
}
