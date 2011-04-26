//
//  ndfa.cpp
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "ndfa.h"
#include "transition.h"

using namespace dfa;

/// \brief Copies an existing NDFA
ndfa::ndfa(const ndfa& copyFrom) 
: m_States(copyFrom.m_States)
, m_Symbols(copyFrom.m_Symbols)
, m_CurrentState(0) {
}

// \brief Creates a new NDFA
ndfa::ndfa() 
: m_CurrentState(0) {
    m_States.push_back(state(0));
}

// \brief Destructor
ndfa::~ndfa() {
    // Destroy any accept actions that might be in this NDFA
    for (accept_action_for_state::iterator acceptState = m_Accept.begin(); acceptState != m_Accept.end(); acceptState++) {
        for (accept_action_list::iterator actionList = acceptState->second.begin(); actionList != acceptState->second.end(); actionList++) {
            delete *actionList;
        }
    }
}

/// \brief While building an NDFA: moves to the specified state
int ndfa::goto_state(int newState) {
    // Set the current state
    m_CurrentState = newState;
    
    // Set the current state to 0 if an out of range state is specified
    if (m_CurrentState < 0 || m_CurrentState >= m_States.size()) {
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
    int     newStateId = (int)m_States.size();
    state   newState(newStateId);
    
    m_States.push_back(newState);
    
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
    if (oldState < 0 || oldState >= m_States.size()) return;
    if (newState < 0 || newState >= m_States.size()) return;
    
    // Get the ID for this symbol set
    int symbolId = m_Symbols.identifier_for_symbols(symbols);
    
    // Add as a transition to the current state
    m_States[oldState].add(transition(symbolId, newState));
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
    m_Accept[state].push_back(action.clone());
}
