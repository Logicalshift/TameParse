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
    // TODO: implement me
}

/// \brief Creates a clone of this rewriter
action_rewriter* weak_symbols::clone() const {
    return new weak_symbols(*this);
}
