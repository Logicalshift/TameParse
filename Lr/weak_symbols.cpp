//
//  weak_symbols.cpp
//  Parse
//
//  Created by Andrew Hunter on 06/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "weak_symbols.h"

using namespace std;
using namespace dfa;
using namespace contextfree;
using namespace lr;

/// \brief Constructs a translator with no weak symbols
weak_symbols::weak_symbols() {
}

/// \brief Constructs a rewriter with the specified map of strong to weak symbols
weak_symbols::weak_symbols(const item_map& map) 
: m_StrongToWeak(map) {
}

/// \brief Copy constructor
weak_symbols::weak_symbols(const weak_symbols& copyFrom)
: m_StrongToWeak(copyFrom.m_StrongToWeak) {
}

/// \brief Destructor
weak_symbols::~weak_symbols() {
}

/// \brief Maps the specified strong symbol to the specified set of weak symbols
void weak_symbols::add_symbols(const contextfree::item_container& strong, const contextfree::item_set& weak) {
    // Merge the set of symbols for this strong symbol
    m_StrongToWeak[strong].insert(weak.begin(), weak.end());
}

/// \brief Given a set of weak symbols and a DFA (note: NOT an NDFA), determines the appropriate strong symbols and adds them
void weak_symbols::add_symbols(const dfa::ndfa& dfa, const contextfree::item_set& weak) {
    // Create a set of the identifiers of the weak terminals in the set
    set<int> weakIdentifiers;
    
    for (item_set::const_iterator it = weak.begin(); it != weak.end(); it++) {
        // Only terminal symbols are returned by a NDFA
        if ((*it)->type() != item::terminal) continue;
        
        // Map this item
        weakIdentifiers.insert((*it)->symbol());
    }
    
    // Map of weak to strong symbols
    map<int, set<int> > weakToStrong;
    
    // Iterate through all of the DFA states
    typedef ndfa::accept_action_list accept_actions;
    for (int state = 0; state < dfa.count_states(); state++) {
        // Get the accepting actions for this state
        const accept_actions& accept = dfa.actions_for_state(state);
        
        // Ignore this state if it has no accepting actions
        if (accept.empty()) continue;
        
        // Iterate through the accepting action list and determine the 'strongest' symbol
        int         strongest   = 0x7fffffff;
        set<int>    usedWeak;
        
        for (accept_actions::const_iterator it = accept.begin(); it != accept.end(); it++) {
            int sym = (*it)->symbol();
            
            // Ignore weak symbols
            if (weakIdentifiers.find(sym) != weakIdentifiers.end()) {
                usedWeak.insert(sym);
                continue;
            }
            
            // Lower symbol IDs are stronger than weaker ones, and the strongest ID is the one that will be accepted
            // (Same as is defined in the ordering of accept_action)
            if (sym < strongest) {
                strongest = sym;
            }
        }
        
        // If no 'strongest' symbol was found, then all the symbols here are weak, and we can ignore this state
        if (strongest == 0x7fffffff)    continue;
        
        // Also can ignore the state if no weak symbols were found
        if (usedWeak.empty())           continue;
        
        // Map the weak symbols we found to this identifier
        for (set<int>::iterator it = usedWeak.begin(); it != usedWeak.end(); it++) {
            weakToStrong[*it].insert(strongest);
        }
    }
    
    // TODO: if a weak symbol maps to several strong identifiers, it needs to be split into several symbols
    
    // Fill in the strong map for each weak symbol
    for (map<int, set<int> >::iterator it = weakToStrong.begin(); it != weakToStrong.end(); it++) {
        // Every item must have at least one strong symbol mapped to it
        if (it->second.empty()) continue;
        
        // Use the first strong symbol for this weak symbol
        item_container strongTerm(new terminal(*it->second.begin()), true);
        item_container weakTerm(new terminal(it->first), true);
        
        m_StrongToWeak[strongTerm].insert(weakTerm);
        
        // TODO: split up weak symbols with multiple strong symbols
    }
}

/// \brief Modifies the specified set of actions according to the rules in this rewriter
void weak_symbols::rewrite_actions(int state, lr_action_set& actions, const lalr_builder& builder) const {
    // Determine if there are any actions referring to strong symbols in this action set
    bool haveStrong = false;
    
    for (lr_action_set::const_iterator it = actions.begin(); it != actions.end(); it++) {
        // This only deals with actions on terminal items
        if ((*it)->item()->type() != item::terminal) continue;
        
        // See if the symbol is marked as being 'strong' and has weak symbols associated with it
        item_map::const_iterator found = m_StrongToWeak.find((*it)->item());
        if (found != m_StrongToWeak.end() && !found->second.empty()) {
            haveStrong = true;
            break;
        }
    }
    
    // If none of the actions reference strong items, then the actions can be left as they are
    if (!haveStrong) return;
    
    // Start building up a new set of actions
    lr_action_set newActions;
    
    // Add any actions containing strong symbols, and remember the associated weak symbols
    map<int, int> weakToStrong;         // Every weak symbol is associated with at most one strong symbol
    
    for (lr_action_set::const_iterator it = actions.begin(); it != actions.end(); it++) {
        // This only deals with actions on terminal items. Preserve all other items.
        if ((*it)->item()->type() != item::terminal) {
            newActions.insert(*it);
            continue;
        }

        // Strong actions are always preserved
        item_map::const_iterator strong = m_StrongToWeak.find((*it)->item());
        if (strong == m_StrongToWeak.end()) continue;
        
        newActions.insert(*it);
        
        // Store the weak symbols in the weak to strong table
        for (item_set::const_iterator weakId = strong->second.begin(); weakId != strong->second.end(); weakId++) {
            weakToStrong[(*weakId)->symbol()] = strong->first->symbol();
        }
    }
    
    // Modify any weak symbols that were in the original action set
    for (lr_action_set::const_iterator it = actions.begin(); it != actions.end(); it++) {
        // Skip over them non terminal items again
        if ((*it)->item()->type() != item::terminal) continue;
        
        // Ignore this item if it's not in the weak table
        map<int, int>::iterator weakItem = weakToStrong.find((*it)->item()->symbol());
        if (weakItem == weakToStrong.end()) continue;
        
        lr_action::action_type actType = (*it)->type();
        
        switch (actType) {
            case lr_action::act_reduce:
                // If the action is a reduce, then replace it with a weak reduce. Leave the symbol in the weak table.
                newActions.insert(lr_action_container(new lr_action(lr_action::act_weakreduce, (*it)->item(), (*it)->next_state(), (*it)->rule()), true));
                break;
                
            case lr_action::act_shift:
                // If the action is a shift, preserve it and remove the symbol from the weak table
                newActions.insert(*it);
                weakToStrong.erase(weakItem);
                break;
                
            default:
                // Default action is to preserve the action and leave the symbol in the weak table (so there will be a duplicate action generated by the strong symbol)
                newActions.insert(*it);
                break;
        }
    }
    
    // Finally, for any action that relates to a strong symbol, add equivalent actions for the weak symbols that are still in the weakToStrong table
    for (lr_action_set::const_iterator it = actions.begin(); it != actions.end(); it++) {
        // Skip over them non terminal items one last time
        if ((*it)->item()->type() != item::terminal) continue;

        // Ignore this item if it's not in the strong table
        item_map::const_iterator weakSyms = m_StrongToWeak.find((*it)->item());
        if (weakSyms == m_StrongToWeak.end() || weakSyms->second.empty()) continue;
        
        // Duplicate this action for every weak symbol that's equivalent to this strong symbol (unless we removed it from the list in the earlier pass)
        for (item_set::const_iterator weakId = weakSyms->second.begin(); weakId != weakSyms->second.end(); weakId++) {
            const item_container& weakItem = *weakId;

            // This weak symbol must still be in the weakToStrong table
            if (weakToStrong.find(weakItem->symbol()) == weakToStrong.end()) continue;
            
            // Create a duplicate action targeting the weak item
            newActions.insert(lr_action_container(new lr_action(**it, weakItem), true));
        }
    }
    
    // Update the action table with our results
    actions = newActions;
}

/// \brief Creates a clone of this rewriter
action_rewriter* weak_symbols::clone() const {
    return new weak_symbols(*this);
}
