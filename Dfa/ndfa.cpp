//
//  ndfa.cpp
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stack>

#include "ndfa.h"
#include "transition.h"
#include "remapped_symbol_map.h"

using namespace std;
using namespace dfa;

/// \brief Empty action list
const ndfa::accept_action_list ndfa::s_NoActions;

/// \brief A placeholder 'no state' representation
const state ndfa::s_NoState(-1);

/// \brief Copies an existing NDFA
ndfa::ndfa(const ndfa& copyFrom)
: m_States(new state_list())
, m_Symbols(new symbol_map(*copyFrom.m_Symbols))
, m_Accept(new accept_action_for_state())
, m_CurrentState(0) {
    // Copy the states
    for (state_list::const_iterator it = copyFrom.m_States->begin(); it != copyFrom.m_States->end(); it++) {
        m_States->push_back(new state(**it));
    }
    
    // Copy the accept actions
    for (accept_action_for_state::const_iterator it = copyFrom.m_Accept->begin(); it != copyFrom.m_Accept->end(); it++) {
        accept_action_list& action = (*m_Accept)[it->first];
        for (accept_action_list::const_iterator actionIt = it->second.begin(); actionIt != it->second.end(); actionIt++) {
            action.push_back((*actionIt)->clone());
        }
    }
}

// \brief Creates a new NDFA
ndfa::ndfa() 
: m_CurrentState(0)
, m_States(new state_list())
, m_Accept(new accept_action_for_state())
, m_Symbols(new symbol_map()) {
    m_States->push_back(new state(0));
}

/// \brief Creates a new NDFA with the specified data
ndfa::ndfa(state_list* states, symbol_map* symbols, accept_action_for_state* accept)
: m_CurrentState(0)
, m_States(states)
, m_Symbols(symbols)
, m_Accept(accept) {
    
}

// \brief Destructor
ndfa::~ndfa() {
    // Destroy any accept actions that might be in this NDFA
    for (accept_action_for_state::iterator acceptState = m_Accept->begin(); acceptState != m_Accept->end(); acceptState++) {
        for (accept_action_list::iterator actionList = acceptState->second.begin(); actionList != acceptState->second.end(); actionList++) {
            delete *actionList;
        }
    }
    
    // Destroy any states
    for (state_list::iterator stateIt = m_States->begin(); stateIt != m_States->end(); stateIt++) {
        delete *stateIt;
    }
    
    // Destroy the symbol map
    delete m_Symbols;
    delete m_States;
    delete m_Accept;
}

/// \brief While building an NDFA: moves to the specified state
int ndfa::goto_state(int newState) {
    // Set the current state
    m_CurrentState = newState;
    
    // Set the current state to 0 if an out of range state is specified
    if (m_CurrentState < 0 || m_CurrentState >= m_States->size()) {
        m_CurrentState = 0;
    }
    
    // The current state is the return value
    return m_CurrentState;
}

/// \brief While building an NDFA: moves to the start state
int ndfa::goto_start() {
    m_CurrentState = 0;
    return m_CurrentState;
}

/// \brief Adds a new state (with no transitions), and returns the ID for that state
int ndfa::add_state() {
    int     newStateId  = (int)m_States->size();
    state*  newState    = new state(newStateId);
    
    m_States->push_back(newState);
    
    return newStateId;
}

/// \brief Adds a transition from the current to a new state, moves to the state, and returns the ID of the state
int ndfa::add_transition(const symbol_set& transition) {
    // Add a new state
    int newStateId = add_state();
    
    // Add a transition to this state
    add_transition(transition, newStateId);
    
    // Return the ID of the new state
    return newStateId;
}

/// \brief Adds a transition from the current state to the specified state, and moves to that state
void ndfa::add_transition(const symbol_set& symbols, int newState) {
    /// \brief Add the transition for this state
    add_transition(m_CurrentState, symbols, newState);
    
    // Update the current state
    m_CurrentState = newState;
}

/// \brief Adds a transition from the old state to the new state
void ndfa::add_transition(int oldState, const symbol_set& symbols, int newState) {
    // Nothing to do if the state ID is invalid
    if (oldState < 0 || oldState >= m_States->size()) return;
    if (newState < 0 || newState >= m_States->size()) return;
    
    // Get the ID for this symbol set
    int symbolId = m_Symbols->identifier_for_symbols(symbols);
    
    // Add as a transition to the current state
    (*m_States)[oldState]->add(transition(symbolId, newState));
}

/// \brief Marks the current state as an accepting state (for the specified symbol)
///
/// If eager is set, then the state is marked as an 'eager' accepting state - ie, the resulting DFA will not consider states 'following' this one
/// when it is evaluated.
void ndfa::accept(int symbol, bool eager) {
    accept(accept_action(symbol, eager));
}

/// \brief Marks the current state as an accepting state (for the specified symbol)
///
/// If eager is set, then the state is marked as an 'eager' accepting state - ie, the resulting DFA will not consider states 'following' this one
/// when it is evaluated.
void ndfa::accept(const accept_action& action) {
    accept(m_CurrentState, action);
}


/// \brief Marks the current state as an accepting state, returning the specified symbol
///
/// If eager is set, then the state is marked as an 'eager' accepting state - ie, the resulting DFA will not consider states 'following' this one
/// when it is evaluated.
void ndfa::accept(int state, const accept_action& action) {
    // Clone the action and store it
    (*m_Accept)[state].push_back(action.clone());
}

/// \brief Marks the specified state as non-accepting
void ndfa::clear_accept(int state) {
    // Remove all of the items from this state
    (*m_Accept)[state].clear();
}

/// \brief Pops the state on top of the stack and discards it
///
/// Returns false if the stack is empty.
/// The 'previous state' becomes the state on top of the stack.
/// If an 'or' is being constructed, this will move to the final state of the 'or' in the same way that rejoin does.
bool ndfa::builder::pop() { 
    if (!m_Stack.empty()) {
        // If there's a final state, then move there from the current state
        if (m_Stack.top().second >= 0) {
            int finalState = m_Stack.top().second;
            *this >> m_Ndfa->get_state(finalState) >> epsilon();
        }
        
        // Set the previous state to the 'initial' state on top of the stack
        if (m_Stack.top().first >= 0) {
            m_PreviousState = m_Stack.top().first;
        }
        
        // Pop the state
        m_Stack.pop();
        return true;
    } else {
        return false;
    }
}

/// \brief Begins or continues an 'or' expression
void ndfa::builder::begin_or() {
    // If the stack is empty then push a fake entry
    if (m_Stack.empty()) {
        m_Stack.push(stack_entry(0, -1));
    }
    
    // The initial state is the state on top of the stack
    int initialState = m_Stack.top().first;
    
    // Remember the previous state (in case we change it)
    int previousState = m_PreviousState;
    
    if (m_Stack.top().second < 0) {
        // Create a 'final state' for the various alternatives by adding an epsilon transition
        *this >> epsilon();
        
        // Set this on the stack
        m_Stack.top().second = m_CurrentState;
        
        // Preserve the previous state
        m_PreviousState = previousState;
    } else {
        // Create a transition from the current state to the final state
        m_Ndfa->add_transition(m_CurrentState, epsilon(), m_Stack.top().second);
    }
    
    // Move back to the 'initial state' (the point where the or expression starts)
    m_CurrentState = initialState;
}

/// \brief Moves to the state after the current 'or' expression, if there is one
void ndfa::builder::rejoin() {
    // Nothing to do if the top of the stack isn't building an 'or' expression
    if (m_Stack.empty() || m_Stack.top().second < 0) return;
    
    // Move to the final state
    int finalState = m_Stack.top().second;
    *this >> m_Ndfa->get_state(finalState) >> epsilon();
    
    // Unset the final state
    m_Stack.top().second = -1;
    
    // Pop the top stack entry if it was added by an or operation
    if (m_Stack.top().first == -1) {
        m_Stack.pop();
    }
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
    for (accept_action_for_state::const_iterator it = m_Accept->begin(); it != m_Accept->end(); it++) {
        accept_action_list& action = (*accept)[it->first];
        for (accept_action_list::const_iterator actionIt = it->second.begin(); actionIt != it->second.end(); actionIt++) {
            action.push_back((*actionIt)->clone());
        }
    }

    // Recreate the states
    for (state_list::const_iterator stateIt = m_States->begin(); stateIt != m_States->end(); stateIt++) {
        // Create a new state
        states->push_back(new state((int)states->size()));
        state& newState = **(states->rbegin());
        
        // Create transitions for this state
        for (state::iterator transit = (*stateIt)->begin(); transit != (*stateIt)->end(); transit++) {
            // Get the new symbols for this transition
            int transitSet      = transit->symbol_set();
            int transitState    = transit->new_state();
            
            const remapped_symbol_map::new_symbol_set& newSyms = symbols->new_symbols(transitSet);
            
            for (remapped_symbol_map::new_symbol_set::const_iterator symIt = newSyms.begin(); symIt != newSyms.end(); symIt++) {
                newState.add(transition(*symIt, transitState));
            }
        }
    }
    
    // Create the new NDFA
    return new ndfa(states, symbols, accept);
}

/// \brief Internal method: computes the closure of the specified set of states (modifies the set to include 
/// all states reachable by epsilon transitions)
void ndfa::closure(set<int>& states) const {
    /// Set of states that need to be checked for epsilon transitions
    set<int> newStates = states;
    
    // Get the symbol ID of the epsilon set
    int epsSymbol = m_Symbols->identifier_for_symbols(epsilon());
    if (epsSymbol == -1) return;
    
    // Iterate until we've added no new states
    while (!newStates.empty()) {
        // Create a set of states we're going to add
        set<int> addedStates;
        
        for (set<int>::iterator stateIt = newStates.begin(); stateIt != newStates.end(); stateIt++) {
            // Find any epsilon transitions in this state
            const state& thisState = get_state(*stateIt);
            
            for (state::iterator transIt = thisState.begin(); transIt != thisState.end(); transIt++) {
                // Ignore non-epsilon transitions
                if (transIt->symbol_set() != epsSymbol) continue;
                
                // Ignore this state if it's already in the set
                int newState = transIt->new_state();
                if (states.find(newState) != states.end()) continue;
                
                // Add this state (and process it for further epsilons the next time through)
                states.insert(newState);
                addedStates.insert(newState);
            }
        }
        
        // The new states for the next iteration are the states we added in this iterator
        newStates = addedStates;
    }
}


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
    for (vector<int>::const_iterator initialIt = initialState.begin(); initialIt != initialState.end(); initialIt++) {
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
        
        for (state_set::const_iterator stateIt = nextSet.begin(); stateIt != nextSet.end(); stateIt++) {
            // Get this state
            state& thisState = *(*m_States)[*stateIt];
            
            // For each transition in this state, add to the appropriate set
            for (state::iterator transit = thisState.begin(); transit != thisState.end(); transit++) {
                // Ignore the epsilon set (covered by performing the closure)
                if (transit->symbol_set() == epsilonSymbolSet) continue;
                
                // Otherwise, add this transition
                statesForSymbol[transit->symbol_set()].insert(transit->new_state());
            }
            
            // Add the accepting actions for this state, if there are any
            accept_action_for_state::const_iterator acceptForState = m_Accept->find(*stateIt);
            if (acceptForState != m_Accept->end()) {
                for (accept_action_list::const_iterator acceptIt = acceptForState->second.begin(); acceptIt != acceptForState->second.end(); acceptIt++) {
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
        for (transition_for_symbol::iterator it = statesForSymbol.begin(); it != statesForSymbol.end(); it++) {
            closure(it->second);
        }
        
        // Generate new transitions for each symbol
        for (transition_for_symbol::const_iterator transit = statesForSymbol.begin(); transit != statesForSymbol.end(); transit++) {
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
    return new ndfa(states, symbols, accept);
}
