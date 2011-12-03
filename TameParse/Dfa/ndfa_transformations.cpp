//
//  ndfa_transformations.cpp
//  TameParse
//
//  Created by Andrew Hunter on 11/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

//
// The various to_x methods for the NDFA (seperated out as they are rather large and there's a lot of them)
//

#include <stack>

#include "TameParse/Dfa/ndfa.h"
#include "TameParse/Dfa/transition.h"
#include "TameParse/Dfa/remapped_symbol_map.h"

using namespace std;
using namespace dfa;

/// \brief Class used to compare accepting actions
class order_actions {
public:
    /// Returns true if one accept action is less than another
    inline bool operator()(accept_action* a, accept_action* b) const {
        if (a == b)     return false;
        if (a == NULL)  return true;
        if (b == NULL)  return false;
        
        return *a < *b;
    }
};

/// \brief Creates a DFA from this NDFA
///
/// Note that if further transitions are added to the DFA, it may no longer be deterministic.
/// Use this call on the result of calling to_ndfa_with_unique_symbols.
ndfa* ndfa::to_dfa(const vector<int>& initialState) const {
    // Empty NDFA if no states were supplied
    if (initialState.size() == 0) { 
        return new ndfa();
    }
    
    // Some types used by this method
    typedef set<int>                        state_set;                                  // Set of states in this NDFA (maps onto a single state in the final NDFA)
    typedef pair<int, state*>               remaining_entry;                            // State that's waiting to be processed
    typedef map<int, state_set>             transition_for_symbol;                      // Maps a symbol_set to the states that would be reached in this NDFA
    
    // Create the structures for the new DFA. Symbols are preserved (and state 0 remains the same), but we regenerate everything else
    symbol_map*                 symbols     = new symbol_map(*m_Symbols);
    state_list*                 states      = new state_list();
    accept_action_for_state*    accept      = new accept_action_for_state();
    vector<state_set>           stateSets;
    
    // Get the epsilon set
    int epsilonSymbolSet = m_Symbols->identifier_for_symbols(epsilon());
    
    // Create a map saying which of our states are represented in each new state
    map<state_set, int> stateMap;
    
    // Create the stack of states to process
    stack<remaining_entry> remainingStates;

    // Create the set of initial states
    for (vector<int>::const_iterator initialIt = initialState.begin(); initialIt != initialState.end(); ++initialIt) {
        // Create a set for this initial state
        state_set thisStateSet;
        thisStateSet.insert(*initialIt);
        closure(thisStateSet);
        
        // Put it in the state map
        int stateId = (int)states->size();
        
        // Generate a state for this ID
        states->push_back(new state(stateId));
        stateSets.push_back(thisStateSet);
        
        // Add to the map
        if (stateMap.find(thisStateSet) == stateMap.end()) {
            // (We create a new state if there's a duplicate initial state, but the first state we created becomes the 'canonical' one)
            stateMap[thisStateSet] = stateId;
        }
        
        // Add to the list of states to process
        remainingStates.push(remaining_entry(stateId, (*states)[stateId]));
    }
    
    // Keep processing states until we stop generating new ones
    while (!remainingStates.empty()) {
        // Get the next state to process
        remaining_entry next = remainingStates.top();
        remainingStates.pop();
        
        // Find the set of states reached by each symbol set for this transition
        transition_for_symbol statesForSymbol;
        
        // For each state making up this state...
        bool isEager = false;
        const state_set& nextSet = stateSets[next.first];
        
        for (state_set::const_iterator stateIt = nextSet.begin(); stateIt != nextSet.end(); ++stateIt) {
            // Get this state
            state& thisState = *(*m_States)[*stateIt];
            
            // For each transition in this state, add to the appropriate set
            for (state::iterator transit = thisState.begin(); transit != thisState.end(); ++transit) {
                // Ignore the epsilon set (covered by performing the closure)
                if (transit->symbol_set() == epsilonSymbolSet) continue;
                
                // Otherwise, add this transition
                statesForSymbol[transit->symbol_set()].insert(transit->new_state());
            }
            
            // Add the accepting actions for this state, if there are any
            accept_action_for_state::const_iterator acceptForState = m_Accept->find(*stateIt);
            if (acceptForState != m_Accept->end()) {
                for (accept_action_list::const_iterator acceptIt = acceptForState->second.begin(); acceptIt != acceptForState->second.end(); ++acceptIt) {
                    // Add a clone of this action
                    (*accept)[next.second->identifier()].push_back((*acceptIt)->clone());
                    
                    // Mark if it's eager
                    if ((*acceptIt)->eager()) {
                        isEager = true;
                    }
                }
            }
        }
        
        // If this state is 'eager' (ie, accepts immediately), then there's no point in generating any transitions from it
        if (isEager) continue;
        
        // Generate the closures for epsilon transitions
        for (transition_for_symbol::iterator it = statesForSymbol.begin(); it != statesForSymbol.end(); ++it) {
            closure(it->second);
        }
        
        // Generate new transitions for each symbol
        for (transition_for_symbol::const_iterator transit = statesForSymbol.begin(); transit != statesForSymbol.end(); ++transit) {
            // Try to find state that this transition is targeting
            map<state_set, int>::const_iterator targetState = stateMap.find(transit->second);
            
            // Create a new state if there's no existing state
            if (targetState == stateMap.end()) {
                // Work on the new state ID
                int newStateId = (int) states->size();
                
                // Add to the state map
                targetState = stateMap.insert(pair<state_set, int>(transit->second, newStateId)).first;
                
                // Create the new state
                states->push_back(new state(newStateId));
                stateSets.push_back(transit->second);
                
                // Add the new state to the list that need processiing
                remainingStates.push(remaining_entry(newStateId, (*states)[newStateId]));
            }
            
            // Add this transition
            next.second->add(transition(transit->first, targetState->second));
        }
    }
    
    // Create the new NDFA from the result
    ndfa* result = new ndfa(states, symbols, accept);
    
    // Mark it as deterministic
    result->m_IsDeterministic = true;
    
    // Return it
    return result;
}

/// \brief Creates a new NDFA that is equivalent to this one, except there will be no overlapping symbol sets
///
/// Note that if further transitions are added to the new NDFA, it may lose the unique symbol sets
ndfa* ndfa::to_ndfa_with_unique_symbols() const {
    // Remap the symbols so that there are no duplicates
    remapped_symbol_map* symbols = remapped_symbol_map::deduplicate(*m_Symbols);
    
    // Rebuild the transition table with the new symbols
    state_list*                 states  = new state_list();
    accept_action_for_state*    accept  = new accept_action_for_state();

    // Copy the accept actions
    for (accept_action_for_state::const_iterator it = m_Accept->begin(); it != m_Accept->end(); ++it) {
        accept_action_list& action = (*accept)[it->first];
        for (accept_action_list::const_iterator actionIt = it->second.begin(); actionIt != it->second.end(); ++actionIt) {
            action.push_back((*actionIt)->clone());
        }
    }

    // Recreate the states
    for (state_list::const_iterator stateIt = m_States->begin(); stateIt != m_States->end(); ++stateIt) {
        // Create a new state
        states->push_back(new state((int)states->size()));
        state& newState = **(states->rbegin());
        
        // Create transitions for this state
        for (state::iterator transit = (*stateIt)->begin(); transit != (*stateIt)->end(); ++transit) {
            // Get the new symbols for this transition
            int transitSet      = transit->symbol_set();
            int transitState    = transit->new_state();
            
            const remapped_symbol_map::new_symbol_set& newSyms = symbols->new_symbols(transitSet);
            
            for (remapped_symbol_map::new_symbol_set::const_iterator symIt = newSyms.begin(); symIt != newSyms.end(); ++symIt) {
                newState.add(transition(*symIt, transitState));
            }
        }
    }
    
    // Create the new NDFA
    return new ndfa(states, symbols, accept);
}

/// \brief Compacts a DFA, reducing the number of states
///
/// For DFAs with only a single initial state, this may have one extra state than is required. If firstAction is set
/// to true, then the resulting DFA will only have final states that contain the first action (rather than all possible
/// actions): this will generally result in a smaller DFA, at the cost of being able to distinguish states that are
/// ambiguous.
ndfa* ndfa::to_compact_dfa(const vector<int>& initialState, bool firstAction) const {
    // TODO: we can further compact the DFA by looking for symbol sets that always produce the same transition and merging them
    
    // Set of states from the original (this) DFA
    typedef set<int> state_set;
    
    // Vector of new states
    typedef vector<state_set> new_states;

    // Maps state IDs in the original (this) set to IDs in the 'new' set
    typedef map<int, int> state_map;
    
    // Set of actions
    typedef set<accept_action*, order_actions> action_set;

    // Variables used to represent our new state machine
    new_states  newStates;
    state_map   oldToNew;
    
    // Create the set of initial states. We assume that each state can only appear once in the initialState vector; the result
    // will be incorrect if they appear more than once
    for (vector<int>::const_iterator initial = initialState.begin(); initial != initialState.end(); ++initial) {
        // Each initial state becomes an initial state in the new DFA
        newStates.push_back(state_set());
        state_set& thisState = newStates.back();
        
        // Create a new state containing just this item
        oldToNew[*initial] = (int) newStates.size()-1;
        thisState.insert(*initial);
    }
    
    // Bundle all of the non-accepting states up into a single state
    // (If all of the remaining states are accepting then we'll end up with a free bonus state here :-/)
    int nonAcceptingStates = (int) newStates.size();
    newStates.push_back(state_set());
    
    // Also remember which new state contains which set of accepting actions
    map<action_set, int> stateForActions;
    
    // Iterate through the states
    for (int stateId = 0; stateId < count_states(); ++stateId) {
        // If this state is already mapped, then ignore it
        if (oldToNew.find(stateId) != oldToNew.end()) continue;
        
        // Try to fetch the accept actions for this state
        accept_action_for_state::const_iterator acceptActions = m_Accept->find(stateId);

        // If this state is not an accepting state then add it to the non-accepting set
        if (acceptActions == m_Accept->end() || acceptActions->second.empty()) {
            newStates[nonAcceptingStates].insert(stateId);
            oldToNew[stateId] = nonAcceptingStates;
        }
        
        // For accepting states, either create a new state or use an existing one
        else {
            // Build up a set of actions
            action_set actions;
            
            if (firstAction) {
                // Choose only the 'first' action (the one that compares 'highest')
                accept_action* smallestAction = acceptActions->second[0];
                for (accept_action_list::const_iterator action = acceptActions->second.begin(); 
                     action != acceptActions->second.end(); ++action) {
                    if ((*smallestAction) < (**action)) {
                        smallestAction = *action;
                    }
                }
                
                // Add this action to the set
                actions.insert(smallestAction);
            } else {
                // Create a set of all of the accept actions for this state
                for (accept_action_list::const_iterator action = acceptActions->second.begin(); 
                     action != acceptActions->second.end(); ++action) {
                    actions.insert(*action);
                }
            }
            
            // Get the state that's mapped to this set of actions
            map<action_set, int>::const_iterator existingState = stateForActions.find(actions);
            int targetState;
            
            if (existingState == stateForActions.end()) {
                // Create a new state
                targetState = (int) newStates.size()-1;
                newStates.push_back(state_set());
            } else {
                // Add to the existing state
                targetState = existingState->second;
            }
            
            // Add to the appropriate state
            newStates[targetState].insert(stateId);
            oldToNew[stateId] = targetState;
        }
    }
    
    // Now, iterate through the states until there are no changes to make
    bool changed = true;
    while (changed) {
        // No changes so far this pass
        changed = false;
        
        // Iterate through the set of 'new' states
        for (int newStateId = 0; newStateId < (int) newStates.size(); ++newStateId) {
            // Split any symbol that has a transition to more than one different (new) state
            state_set thisState = newStates[newStateId];                        // TODO: use pointers to state sets instead
            if (thisState.size() <= 1) continue;
            
            // Use the first state in the set as the template; split off any state that doesn't match it
            map<int, int> targetStateForSymbol;
            
            state_set::iterator curState    = thisState.begin();
            const state&        firstState  = get_state(*curState);
            
            for (state::iterator transit = firstState.begin(); transit != firstState.end(); ++transit) {
                targetStateForSymbol[transit->symbol_set()] = oldToNew[transit->new_state()];
            }
            
            // Iterate through the remaining states, and put any that have a different transition set into a new state
            int splitStateId    = -1;
            int numToMatch      = firstState.count_transitions();
            vector<int> toRemove;
            ++curState;
            for (; curState != thisState.end(); ++curState) {
                // Check through the items in this state
                const state&    originalState   = get_state(*curState);
                bool            different       = originalState.count_transitions() != numToMatch;
                
                if (!different) {
                    // Number of transitions are the same: need to check that they are actually the same set of transitions
                    for (state::iterator transit = originalState.begin(); transit != originalState.end(); ++transit) {
                        // Check the target state
                        map<int, int>::iterator originalTransition = targetStateForSymbol.find(transit->symbol_set());
                        
                        // ... this state is different if the first state doesn't have this symbol set
                        if (originalTransition == targetStateForSymbol.end()) {
                            different = true;
                            break;
                        }
                        
                        // ... this state is different if the target state is different
                        if (originalTransition->second != oldToNew[transit->new_state()]) {
                            different = true;
                            break;
                        }
                    }
                }
                
                // Just leave the states that are the same alone
                if (!different) continue;
                
                // This state is different to the first state, so we split it off into a new set
                if (splitStateId == -1) {
                    // Assign a new state
                    changed         = true;
                    splitStateId    = (int) newStates.size()-1;
                    newStates.push_back(state_set());
                }
                
                // Move this state into the new state
                toRemove.push_back(*curState);                  // Have to remove the state later due to iterator semantics :-/
                newStates[splitStateId].insert(*curState);
                oldToNew[*curState] = splitStateId;
            }
            
            // Remove the states that are no longer in the current state
            for (vector<int>::iterator removeState = toRemove.begin(); removeState != toRemove.end(); ++removeState) {
                // TODO: pointers!
                newStates[newStateId].erase(*removeState);
            }
        }
    }
    
    // Build the final state machine from the result

    // Build the transitions and accepting actions
    state_list*                 states      = new state_list();
    symbol_map*                 symbolMap   = new symbol_map(*m_Symbols);
    accept_action_for_state*    accept      = new accept_action_for_state();
    
    for (int newStateId = 0; newStateId < (int) newStates.size(); ++newStateId) {
        // Ignore empty states
        if (newStates[newStateId].empty()) continue;
        
        state* newState = new state(newStateId);
        states->push_back(newState);
        
        // Add the transitions for this state: we only need a single template state as the mapped transitions for each symbol
        // will be the same
        int             templateStateId = *newStates[newStateId].begin();
        const state&    templateState   = get_state(templateStateId);
        
        for (state::iterator originalTransit = templateState.begin(); originalTransit != templateState.end(); ++originalTransit) {
            int symbolSetId = originalTransit->symbol_set();
            int targetState = oldToNew[originalTransit->new_state()];
            
            newState->add(transition(symbolSetId, targetState));
        }
        
        // Copy the accept actions from the template state
        // TODO: if firstAction is set, then only copy the 'most important' action
        const accept_action_list&   actions         = actions_for_state(templateStateId);
        accept_action_list&         targetActions   = (*accept)[newStateId];
        for (accept_action_list::const_iterator act = actions.begin(); act != actions.end(); ++act) {
            targetActions.push_back((*act)->clone());
        }
    }
    
    // Mark as deterministic
    ndfa* result = new ndfa(states, symbolMap, accept);
    result->m_IsDeterministic = m_IsDeterministic;
    
    // Return the minimized DFA
    return result;
}

/// \brief Creates a new NDFA (DFA if this is a DFA), merging any symbol sets that always produce the same action.
///
/// This will reduce the number of symbol sets in use by the DFA, which will reduce the size of the tables that 
/// are generated from it. This is particularly effective after calling to_compact_dfa() to eliminate any redundant
/// symbol sets.
ndfa* ndfa::to_ndfa_with_merged_symbols() const {
    // A set of all the unique symbols in this (N)DFA
    vector<set<int> >   uniqueSymbols;

    // Maps symbols in this DFA to symbols in our new DFA
    map<int, int>       symbolForSymbol;

    // A set of all the symbols in this NDFA
    set<int> symbols;

    // Begin with a set of all symbols, as ID 0
    uniqueSymbols.push_back(set<int>());
    for (symbol_map::iterator symbolSet = m_Symbols->begin(); symbolSet != m_Symbols->end(); ++symbolSet) {
        uniqueSymbols[0].insert(symbolSet->second);
        symbolForSymbol[symbolSet->second] = 0;
        symbols.insert(symbolSet->second);
    }

    // Iterate through all of the states to find the symbol sets that are different from one another
    for (int stateId = 0; stateId < count_states(); ++stateId) {
        // Fetch this state
        const state& thisState = get_state(stateId);

        // Maps new symbol sets to the state that they transfer to
        map<int, int> stateForSet;

        // Start at the initial transition
        state::iterator transit = thisState.begin();

        // Iterate through the symbols
        set<int>::iterator oldSymbolSet = symbols.begin();

        // Iterate through the transitions for this state
        for (;oldSymbolSet != symbols.end();) {
            // Get the new symbol set for this transit
            int newSymbolSet = symbolForSymbol[*oldSymbolSet];

            // Look for the state that this symbol produces
            map<int, int>::iterator foundState = stateForSet.find(newSymbolSet);

            // Get the state this transition moves to (-1 if this is a rejecting symbol)
            int newState = -1;
            if (transit != thisState.end() && transit->symbol_set() == *oldSymbolSet) {
                newState = transit->new_state();
            }

            // Just remember this target state if it hasn't been encountered before
            if (foundState == stateForSet.end()) {
                stateForSet[newSymbolSet] = newState;
            } 

            // Otherwise, do nothing if this transition maps to the same state
            else if (foundState->second == newState) {
                // Nothing to do
            }

            // This symbol set is different: create a new set consisting of all of the symbols that map to the same set and transit to this alternate state
            else {
                // Create a new symbol set
                uniqueSymbols.push_back(set<int>());
                int         newSetId    = (int)uniqueSymbols.size()-1;
                set<int>&   newSet      = uniqueSymbols.back();

                // Add any symbol that has the same set and target state
                if (newState != -1) {
                    // Add all of the symbols that go to the same state as this one
                    for (state::iterator similarTransit = transit; similarTransit != thisState.end(); ++similarTransit) {
                        // Check that this transit goes to the same state
                        if (similarTransit->new_state() != newState) continue;

                        // It must also use the same symbol set as before
                        int originalSet = similarTransit->symbol_set();
                        int similarSet  = symbolForSymbol[originalSet];
                        if (similarSet != newSymbolSet) continue;

                        // This symbol should be remapped to the set we just created
                        uniqueSymbols[newSymbolSet].erase(originalSet);
                        newSet.insert(originalSet);
                        symbolForSymbol[originalSet] = newSetId;
                    }
                } else {
                    // Add all of the symbols that go nowhere
                    set<int> goNowhere = symbols;

                    // Remove any symbols that go somewhere
                    for (state::iterator otherTransit = thisState.begin(); otherTransit != thisState.end(); ++otherTransit) {
                        goNowhere.erase(otherTransit->symbol_set());
                    }

                    // Add all of the goNowhere symbols
                    for (set<int>::iterator nowhere = goNowhere.begin(); nowhere != goNowhere.end(); ++nowhere) {
                        // Only remap symbols with a similar set
                        int similarSet  = symbolForSymbol[*nowhere];
                        if (similarSet != newSymbolSet) continue;

                        // This symbol should be remapped to the set we just created
                        uniqueSymbols[newSymbolSet].erase(*nowhere);
                        newSet.insert(*nowhere);
                        symbolForSymbol[*nowhere] = newSetId;
                    }
                }
            }

            // Move on to the next transit and/or symbol set
            if (newState == -1) {
                // The transition applies to a later symbol set: only move the symbol set on
                ++oldSymbolSet;
            } else {
                // The transition matches the symbols: move the transition on
                ++transit;

                // Move the symbols on as well if the transition now has a different set
                if (transit == thisState.end() || transit->symbol_set() != *oldSymbolSet) {
                    ++oldSymbolSet;
                }
            }
        }
    }

    // Create the new symbol sets
    symbol_map* newSymbolMap = new symbol_map();

    // Iterate through the different symbol sets that we found
    for (int newSymbolId = 0; newSymbolId < (int) uniqueSymbols.size(); ++newSymbolId) {
        // Get the old sets that are mapped to this new set
        set<int>& newSymbols = uniqueSymbols[newSymbolId];

        // Iterate through them to build up the final symbol set
        symbol_set newSymbolSet;
        for (set<int>::iterator oldSymbolId = newSymbols.begin(); oldSymbolId != newSymbols.end(); ++oldSymbolId) {
            newSymbolSet |= (*m_Symbols)[*oldSymbolId];
        }

        // Add to the new map (relies on the symbol map counting from 0)
        newSymbolMap->identifier_for_symbols(newSymbolSet);
    }

    // Create the new set of states with the remapped symbol sets
    accept_action_for_state*    newActions  = new accept_action_for_state();
    state_list*                 newStates   = new state_list();
    for (int stateId = 0; stateId < count_states(); ++stateId) {
        // Get the old state
        const state& oldState = get_state(stateId);

        // Create the new state
        state* newState = new state(stateId);
        newStates->push_back(newState);

        // Transform the transitions
        for (state::iterator transit=oldState.begin(); transit != oldState.end(); ++transit) {
            newState->add(transition(symbolForSymbol[transit->symbol_set()], transit->new_state()));
        }

        // Copy the actions for this state
        const accept_action_list& oldActions    = actions_for_state(stateId);
        accept_action_list& newActionsForState  = (*newActions)[stateId];

        for (accept_action_list::const_iterator oldAct = oldActions.begin(); oldAct != oldActions.end(); ++oldAct) {
            newActionsForState.push_back((*oldAct)->clone());
        }
    }

    // Return the new NDFA
    ndfa* result = new ndfa(newStates, newSymbolMap, newActions);
    result->m_IsDeterministic = m_IsDeterministic;
    return result;
}
