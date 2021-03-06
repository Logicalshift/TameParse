//
//  weak_symbols.cpp
//  Parse
//
//  Created by Andrew Hunter on 06/05/2011.
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

#include "TameParse/Lr/weak_symbols.h"

using namespace std;
using namespace dfa;
using namespace contextfree;
using namespace lr;

/// \brief Constructs a translator with no weak symbols
weak_symbols::weak_symbols(const grammar* gram)
: m_WeakSymbols(gram)
, m_Grammar(gram) {
}

/// \brief Constructs a rewriter with the specified map of strong to weak symbols
weak_symbols::weak_symbols(const item_map& map, const grammar* gram) 
: m_StrongToWeak(map)
, m_WeakSymbols(gram)
, m_Grammar(gram) {
}

/// \brief Copy constructor
weak_symbols::weak_symbols(const weak_symbols& copyFrom)
: m_StrongToWeak(copyFrom.m_StrongToWeak)
, m_WeakSymbols(copyFrom.m_WeakSymbols)
, m_Grammar(copyFrom.m_Grammar) {
}

/// \brief Destructor
weak_symbols::~weak_symbols() {
}

/// \brief Maps the specified strong symbol to the specified set of weak symbols
void weak_symbols::add_symbols(const contextfree::item_container& strong, const contextfree::item_set& weak) {
    // Merge the set of symbols for this strong symbol
    item_map::iterator weakForStrong = m_StrongToWeak.find(strong);
    if (weakForStrong == m_StrongToWeak.end()) {
        weakForStrong = m_StrongToWeak.insert(item_map::value_type(strong, item_set(m_Grammar))).first;
    }
    
    weakForStrong->second.merge(weak);
    
    // Store these as weak symbols
    m_WeakSymbols.merge(weak);
}

/// \brief Given a set of weak symbols and a DFA (note: NOT an NDFA), determines the appropriate strong symbols and adds them
void weak_symbols::add_symbols(ndfa& dfa, const item_set& weak, terminal_dictionary& terminals) {
    // Nothing to do if there are no weak symbols
    if (weak.empty()) {
        return;
    }
    
    // Create a set of the identifiers of the weak terminals in the set
    set<int> weakIdentifiers;
    
    for (item_set::const_iterator weakItem = weak.begin(); weakItem != weak.end(); ++weakItem) {
        // Only terminal symbols are returned by a NDFA
        if ((*weakItem)->type() != item::terminal) continue;
        
        // Map this item
        weakIdentifiers.insert((*weakItem)->symbol());
    }
    
    // Map of weak to strong symbols
    map<int, int> weakToStrong;
    
    // Iterate through all of the DFA states
    typedef ndfa::accept_action_list accept_actions;
    for (int state = 0; state < dfa.count_states(); ++state) {
        // Get the accepting actions for this state
        const accept_actions& accept = dfa.actions_for_state(state);
        
        // Ignore this state if it has no accepting actions
        if (accept.empty()) continue;
        
        // Iterate through the accepting action list and determine the 'strongest' symbol
        int         strongest   = 0x7fffffff;
        set<int>    usedWeak;
        
        for (accept_actions::const_iterator action = accept.begin(); action != accept.end(); ++action) {
            int sym = (*action)->symbol();
            
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
        for (set<int>::iterator weakSymbol = usedWeak.begin(); weakSymbol != usedWeak.end(); ++weakSymbol) {
            if (weakToStrong.find(*weakSymbol) == weakToStrong.end()) {
                // This is the first conflict between the two symbols. Just mark out the mapping.
                weakToStrong[*weakSymbol] = strongest;
            } else if (weakToStrong[*weakSymbol] != strongest) {
                // The weak symbol is used somewhere else with a different meaning
                // NOTE: this isn't ideal, as if there's an identical conflict somewhere else we generate more and more symbols for each conflicting
                // state, while we only need one new symbol for each (weak, strong) pair.
                
                // Split the weak symbol so that we have two different meanings
                int splitSymbolId = terminals.split(*weakSymbol);
                
                // Change the accepting action for this state so that it accepts the new symbol
                dfa.clear_accept(state);
                dfa.accept(state, accept_action(splitSymbolId));
                
                // Map this in the weakToStrong table
                weakToStrong[splitSymbolId] = strongest;
            }
        }
    }
    
    // TODO: if a weak symbol maps to several strong identifiers, it needs to be split into several symbols
    
    // Fill in the strong map for each weak symbol
    for (map<int, int>::iterator weakStrong = weakToStrong.begin(); weakStrong != weakToStrong.end(); ++weakStrong) {
        // Use the first strong symbol for this weak symbol
        item_container strongTerm(new terminal(weakStrong->second), true);
        item_container weakTerm(new terminal(weakStrong->first), true);

        item_map::iterator weakForStrong = m_StrongToWeak.find(strongTerm);
        if (weakForStrong == m_StrongToWeak.end()) {
            weakForStrong = m_StrongToWeak.insert(item_map::value_type(strongTerm, item_set(m_Grammar))).first;
        }

        weakForStrong->second.insert(weakTerm);
    }
    
    // Add to the list of weak symbols
    m_WeakSymbols.merge(weak);
}

/// \brief Modifies the specified set of actions according to the rules in this rewriter
///
/// To deal with weak symbols, several rewrites are performed.
///
///  * All shift actions are left intact (whether weak or strong)
///  * All actions for symbols that are 'strong' (not in the m_WeakSymbols table) are left intact
///  * Reduce actions on weak symbols where there is an equivalent shift or reduce action on a strong symbol
///    are changed to weak reduce actions
///  * For strong symbols, any equivalent weak symbol that does not have an action other than weak reduce, the
///    same action is added
///
/// For states that do not contain any actions containing strong symbols with weak equivalents, there is no
/// need to change the contents of the state.
///
/// Something to note is that this will change the reduce part of conflicts to refer to the action defined by
/// the strong symbol instead of the weak symbol.
///
/// Something else to note is that if a strong symbol is considered for an action before a weak symbol where
/// there is a potential clash, then it's likely that the parser will be incorrect as weak reduce actions for
/// the weak symbol will not be considered in many cases.
void weak_symbols::rewrite_actions(int state, lr_action_set& actions, const lalr_builder& builder) const {
    // Determine if there are any actions referring to strong symbols in this action set
    bool haveStrong = false;
    
    for (lr_action_set::const_iterator act = actions.begin(); act != actions.end(); ++act) {
        // This only deals with actions on terminal items
        if ((*act)->item()->type() != item::terminal) continue;
        
        // See if the symbol is marked as being 'strong' and has weak symbols associated with it
        item_map::const_iterator found = m_StrongToWeak.find((*act)->item());
        if (found != m_StrongToWeak.end() && !found->second.empty()) {
            haveStrong = true;
            break;
        }
    }
    
    // If none of the actions reference strong items with weak equivalents, then the actions can be left as they are
    if (!haveStrong) return;
    
    // Start building up a new set of actions
    lr_action_set newActions;
    
    // Add actions for any strong symbols, and remember the weak symbols that already have actions
    map<int, int>               weakSymbols;                // Maps symbols to # actions that refer to them
    stack<lr_action_container>  weakActions;                // Weak actions not processed in the first pass
    stack<lr_action_container>  strongActions;              // Strong actions that might have associated weak symbols
    
    for (lr_action_set::const_iterator act = actions.begin(); act != actions.end(); ++act) {
        // Actions on non-terminal symbols should be preserved
        if ((*act)->item()->type() != item::terminal) {
            newActions.insert(*act);
            continue;
        }
        
        // Work out if this action is on a strong symbol
        bool isStrong = !m_WeakSymbols.contains((*act)->item());
        
        // Actions on strong symbols should be preserved without alteration
        if (isStrong) {
            strongActions.push(*act);
            newActions.insert(*act);
            continue;
        }
        
        // Guard actions do not mark a weak symbol as 'used'
        if ((*act)->type() == lr_action::act_guard) {
            newActions.insert(*act);
            continue;
        }
        
        // Fetch the symbol for this action
        int sym = (*act)->item()->symbol();
        
        // Mark this weak symbol as having an existing action
        ++weakSymbols[sym];
        
        // Push on to the set of weak actions
        weakActions.push(*act);
    }
    
    // Transform the actions associated with weak symbols
    while (!weakActions.empty()) {
        // Get the next action
        const lr_action_container& act = weakActions.top();
        
        // How this is rewritten depends on the action
        switch (act->type()) {
            case lr_action::act_shift:
            case lr_action::act_shiftstrong:
            case lr_action::act_ignore:
            case lr_action::act_accept:
            case lr_action::act_guard:
                // Preserve the action
                newActions.insert(act);
                break;
                
            case lr_action::act_reduce:
            case lr_action::act_weakreduce:
            {
                // Reduce actions are rewritten as weak reduce actions
                // TODO: if the strong symbol equivalent of this weak symbol is not present in the action, then
                // leave this as a standard reduce
                lr_action_container weakReduce(new lr_action(lr_action::act_weakreduce, act->item(), act->next_state(), act->rule()), true);
                newActions.insert(weakReduce);
                
                // Remove from the weak symbols table if there are no other actions for this symbol
                // This means that if there is a strong symbol that is equivalent to this weak symbol that the actions
                // for that symbol will be generated. If the strong symbol has a higher priority than the weak action
                // (by default: has a lower symbol ID), then its actions will be considered first, which is incorrect.
                int sym = act->item()->symbol();
                
                map<int, int>::iterator found = weakSymbols.find(sym);
                if (found != weakSymbols.end()) {
                    found->second--;
                    if (found->second < 0) {
                        weakSymbols.erase(found);
                    }
                }
                break;
           }
                
            default:
                // ?? unknown action ??
                // Remove unknown actions that seem to refer to weak symbols
                break;
        }
        
        // Next action
        weakActions.pop();
    }
    
    // For any action on a strong symbol, generate the same action for equivalent weak symbols (if there is no equivalent action)
    // TODO: if a new action is the same as an existing weak symbol action (ie, the weak symbol has produced a weak reduce
    // of the same rule) then we only need one action
    for (; !strongActions.empty(); strongActions.pop()) {
        // Get the next action
        const lr_action_container& act = strongActions.top();
        
        // Look up the equivalent weak symbols for this action
        item_map::const_iterator equivalent = m_StrongToWeak.find(act->item());
        
        // Nothing to do if there are no equivalent symbols
        if (equivalent == m_StrongToWeak.end()) continue;
        
        // Iterate through the weak symbols and generate equivalent actions
        for (item_set::const_iterator weakEquiv = equivalent->second.begin(); weakEquiv != equivalent->second.end(); ++weakEquiv) {
            // Can't deal with non-terminal symbols (should be none, but you can technically add them)
            if ((*weakEquiv)->type() != item::terminal) continue;
            
            // Get the symbol ID of this equivalent symbol
            int symId = (*weakEquiv)->symbol();
            
            // Nothing to do if this symbol has an alternative action
            if (weakSymbols.find(symId) != weakSymbols.end()) continue;
            
            // Add a duplicate action referring to the weak symbol, identical to the action for the strong symbol
            lr_action_container weakEquivAct(new lr_action(*act, *weakEquiv), true);
            
            // Shift actions become shift-strong actions (so the symbol ID is substituted)
            if (weakEquivAct->type() == lr_action::act_shift) {
                weakEquivAct->set_type(lr_action::act_shiftstrong);
            }
            
            newActions.insert(weakEquivAct);
        }
    }
    
    // Update the action table with our results
    actions = newActions;
}

/// \brief Creates a clone of this rewriter
action_rewriter* weak_symbols::clone() const {
    return new weak_symbols(*this);
}
