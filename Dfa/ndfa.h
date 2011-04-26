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
#include <set>
#include <map>

#include "Dfa/state.h"
#include "Dfa/accept_action.h"
#include "Dfa/symbol_set.h"
#include "Dfa/symbol_map.h"

namespace dfa {
    /// \brief Class representing a NDFA (non-deterministic finite state automaton)
    class ndfa {
    public:
        /// \brief List of acceptance actions for a state
        typedef std::vector<accept_action*> accept_action_list;
        
    protected:
        /// \brief A structure containing the states making up this ndfa
        typedef std::vector<state> state_list;
        
        /// \brief List of acceptance actions for a state
        typedef std::map<int, accept_action_list> accept_action_for_state;
        
    private:
        /// \brief Empty action list
        static const accept_action_list s_NoActions;
        
        /// \brief A placeholder 'no state' representation
        static const state& s_NoState;
        
        /// \brief The states in this NDFA
        state_list* m_States;
        
        /// \brief The symbol sets in this NDFA
        symbol_map* m_Symbols;
        
        /// \brief The acceptance actions for a particular state
        accept_action_for_state* m_Accept;
        
    private:
        /// \brief The current state (used while building up the NDFA)
        int m_CurrentState;
        
    protected:
        /// \brief Creates a new NDFA with the specified data
        ndfa(state_list* states, symbol_map* symbols, accept_action_for_state* accept);
        
    public:
        /// \brief Copies an existing NDFA
        ndfa(const ndfa& copyFrom);
        
        /// \brief Creates a new NDFA
        ndfa();
        
        /// \brief Destructor
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
        void accept(const accept_action& action);
        
        /// \brief Marks the current state as an accepting state, returning the specified symbol
        ///
        /// If eager is set, then the state is marked as an 'eager' accepting state - ie, the resulting DFA will not consider states 'following' this one
        /// when it is evaluated.
        void accept(int state, const accept_action& action);
        
    public:
        /// \brief Number of states in this (N)DFA
        inline int count_states() const { return (int)m_States->size(); }
        
        /// \brief The symbol map for this (N)DFA
        inline symbol_map& symbols() { return *m_Symbols; }
        
        /// \brief The symbol map for this (N)DFA
        inline const symbol_map& symbols() const { return *m_Symbols; }
        
        /// \brief The accept actions that apply to the specified state
        inline const accept_action_list& actions_for_state(int state) const {
            // No actions for an invalid state
            if (state < 0 || state >= count_states()) return s_NoActions;
            
            // Find the state
            accept_action_for_state::const_iterator found = m_Accept->find(state);
            if (found == m_Accept->end()) return s_NoActions;
         
            // Produce the results
            return found->second;
        }
        
        /// \brief The state with the specified ID
        inline const state& get_state(int stateNum) const {
            // Use the empty state if the value is out of range
            if (stateNum < 0 || stateNum >= count_states()) return s_NoState;
            
            // Find the state
            return (*m_States)[stateNum];
        }
        
    public:
        /// \brief Creates a new NDFA that is equivalent to this one, except there will be no overlapping symbol sets
        ///
        /// Note that if further transitions are added to the new NDFA, it may lose the unique symbol sets
        ndfa* to_ndfa_with_unique_symbols() const;
        
        /// \brief Creates a DFA from this NDFA
        inline ndfa* to_dfa(int initialState = 0) {
            std::vector<int> initial;
            initial.push_back(initialState);
            
            return to_dfa(initial);
        }
        
        /// \brief Creates a DFA from this NDFA
        ///
        /// Note that if further transitions are added to the DFA, it may no longer be deterministic.
        /// Use this call on the result of calling to_ndfa_with_unique_symbols.
        ///
        /// You can supply a list of initial states to create a DFA with multiple start conditions. These will become states 0, 1, 2, etc in the final DFA.
        ndfa* to_dfa(const std::vector<int>& initialState) const;
    };
}
    
#endif
