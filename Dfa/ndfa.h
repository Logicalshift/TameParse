//
//  ndfa.h
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_NDFA_H
#define _DFA_NDFA_H

#include <vector>

#include "state.h"
#include "symbol_set.h"
#include "symbol_map.h"

namespace dfa {
    // \brief Class representing a NDFA (non-deterministic finite state automaton)
    class ndfa {
    private:
        /// \brief A structure containing the states making up this ndfa
        typedef std::vector<state> state_list;
        
        /// \brief The states in this NDFA
        state_list m_States;
        
        /// \brief The symbol sets in this NDFA
        symbol_map m_Symbols;
        
    private:
        /// \brief The current state (used while building up the NDFA)
        int m_CurrentState;
        
    public:
        /// \brief Copies an existing NDFA
        ndfa(const ndfa& copyFrom);
        
        // \brief Creates a new NDFA
        ndfa();
        
        // \brief Destructor
        virtual ~ndfa();
        
    public:
        /// \brief While building an NDFA: moves to the specified state
        int goto_state(int newState);
        
        /// \brief While building an NDFA: moves to the start state
        int goto_start();
        
        /// \brief Adds a new state (with no transitions), and returns the ID for that state
        int add_state();
        
        /// \brief Adds a transition from the current to a new state, moves to the state, and returns the ID of the state
        int add_transition(const symbol_set& transition);
        
        /// \brief Adds a transition from the current state to the specified state, and moves to that state
        void add_transition(const symbol_set& symbols, int newState);
        
        /// \brief Adds a transition from the old state to the new state
        void add_transition(int oldState, const symbol_set& symbols, int newState);
        
        /// \brief Marks the current state as an accepting state (for the specified symbol)
        ///
        /// If eager is set, then the state is marked as an 'eager' accepting state - ie, the resulting DFA will not consider states 'following' this one
        /// when it is evaluated.
        void accept(int symbol, bool eager = false);
        
        /// \brief Marks the current state as an accepting state (for the specified symbol)
        ///
        /// If eager is set, then the state is marked as an 'eager' accepting state - ie, the resulting DFA will not consider states 'following' this one
        /// when it is evaluated.
        void accept(int state, int symbol, bool eager = false);
    };
}
    
#endif
