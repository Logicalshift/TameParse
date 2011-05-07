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
#include <stack>

#include "Dfa/state.h"
#include "Dfa/accept_action.h"
#include "Dfa/symbol_set.h"
#include "Dfa/symbol_map.h"
#include "Dfa/epsilon.h"

namespace dfa {
    ///
    /// \brief Class representing a NDFA (non-deterministic finite state automaton)
    ///
    /// This class is used to build representations of NDFAs and convert them to deterministic automatas. The most common use
    /// of an NDFA is as an intermediate stage in building a lexer; however, it is possible to apply this class to other cases
    /// where a state machine can be used.
    ///
    /// This class is more generally applicable than the DFAs typically found in regular expression engines: it can distinguish
    /// accepting states (so it is suitable for use for building lexers), and it can have multiple start states (so it can
    /// describe multiple languages in a compact fashion). Additionally, the input symbol set is any positive integer, so this
    /// can be used to build state machines for entities other than strings.
    ///
    class ndfa {
    public:
        /// \brief List of acceptance actions for a state
        typedef std::vector<accept_action*> accept_action_list;
        
    protected:
        /// \brief A structure containing the states making up this ndfa
        typedef std::vector<state*> state_list;
        
        /// \brief List of acceptance actions for a state
        typedef std::map<int, accept_action_list> accept_action_for_state;
        
    private:
        /// \brief Empty action list
        static const accept_action_list s_NoActions;
        
        /// \brief A placeholder 'no state' representation
        static const state s_NoState;
        
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
        
        /// \brief Marks the specified state as non-accepting
        void clear_accept(int state);

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
            return *((*m_States)[stateNum]);
        }
        
    public:
        ///
        /// \brief Class used for convenient construction of an NDFA
        ///
        class constructor {
        private:
            friend class ndfa;
            
            /// \brief The NDFA that this constructor belongs to
            mutable ndfa* m_Ndfa;
            
            /// \brief The current state ID for this constructor
            int m_CurrentState;
            
            /// \brief -1, or the state that should be reached by the next transition
            int m_NextState;
            
            /// \brief The state before the most recent transition, or sequence of transitions
            int m_PreviousState;
            
            /// \brief Entry on the state stack
            ///
            /// First state is the pushed state ('initial' state)
            /// Second state is either -1 or the final state (used for constructing or expressions)
            typedef std::pair<int, int> stack_entry;
            
            /// \brief Stack of states
            std::stack<stack_entry> m_Stack;
            
            /// \brief Creates a new constructor
            inline constructor(ndfa* dfa)
            : m_CurrentState(0)
            , m_PreviousState(0)
            , m_NextState(-1)
            , m_Ndfa(dfa) {
            }
            
        public:
            /// \brief Copy constructor
            inline constructor(const constructor& copyFrom)
            : m_CurrentState(copyFrom.m_CurrentState)
            , m_NextState(copyFrom.m_NextState)
            , m_PreviousState(copyFrom.m_PreviousState)
            , m_Ndfa(copyFrom.m_Ndfa)
            , m_Stack(copyFrom.m_Stack) { 
            }
            
            /// \brief Moves to a new state when the specified range of symbols are encountered
            inline constructor& operator>>(const symbol_set& symbols) {
                int nextState = m_NextState;
                m_NextState = -1;
                
                if (nextState == -1) {
                    nextState = m_Ndfa->add_state();
                }
                
                m_Ndfa->add_transition(m_CurrentState, symbols, nextState);
                m_PreviousState = m_CurrentState;
                m_CurrentState  = nextState;
                
                return *this;
            }
            
            inline constructor& operator>>(char c)                  { return operator>>(range<int>((int)c, (int)c+1)); }
            inline constructor& operator>>(wchar_t c)               { return operator>>(range<int>((int)c, (int)c+1)); }
            inline constructor& operator>>(int c)                   { return operator>>(range<int>((int)c, (int)c+1)); }
            
            /// \brief Sets the state that the next transition will move to
            inline constructor& operator>>(const state& nextState) {
                m_NextState = nextState.identifier();
                return *this;
            }
            
            /// \brief Adds an accept action for the current state
            inline void operator>>(const accept_action& accept) {
                m_Ndfa->accept(m_CurrentState, accept);
            }
            
            /// \brief Returns the current state object represented by this constructor
            inline operator const state&() { return current_state(); }
            
            /// \brief Returns the current state object represented by this constructor
            inline const state& current_state() const {
                return m_Ndfa->get_state(m_CurrentState);
            }
            
            ///
            /// \brief Returns the state this was in before the most recent transition
            ///
            /// This is the state before the last transition, or the state that was last popped off of the stack.
            ///
            inline const state& previous_state() const {
                return m_Ndfa->get_state(m_PreviousState);
            }
            
            /// \brief Sets the state this constructor is in
            inline void goto_state(const state& newState) {
                m_CurrentState  = newState.identifier();
                m_NextState     = -1;
            }
            
            /// \brief Sets the state this constructor is in
            inline void goto_state(const state& newState, const state& previousState) {
                m_PreviousState = previousState.identifier();
                m_CurrentState  = newState.identifier();
                m_NextState     = -1;
            }
            
            ///
            /// \brief Pushes the current state onto the stack
            ///
            /// The pushed state becomes the initial state for an or state, so begin_or will start a new branch at this point.
            /// Additionally, when pop is called, the pushed state becomes the 'previous' state, so this can be used to
            /// implement things like brackets in regular expressions.
            ///
            inline void push() { m_Stack.push(stack_entry(m_CurrentState, -1)); }
            
            /// \brief True if the state stack is empty
            inline bool empty() const { return m_Stack.empty(); }
            
            /// \brief Returns the state on top of the stack
            inline const state& top() const {
                return m_Ndfa->get_state(m_Stack.top().first);
            }
            
            ///
            /// \brief Pops the state on top of the stack and discards it
            ///
            /// Returns false if the stack is empty.
            /// The 'previous state' becomes the state on top of the stack before the pop.
            /// If an 'or' is being constructed, this will move to the final state of the 'or' in the same way that rejoin does.
            ///
            bool pop();
            
            ///
            /// \brief Begins or continues an 'or' expression
            ///
            /// This adds an epsilon transition to a final state, and moves back to whichever state is on top of the stack 
            /// (state 0 if the stack is empty). Ie, this will start to build up an alternative to the sequence of transitions
            /// that follow the last push operation.
            ///
            /// If begin_or() has been previously called (and pop has not been called), the final state will be re-used, so 
            /// there's no need to call rejoin() before this call.
            ///
            void begin_or();
            
            ///
            /// \brief Moves to the state after the current 'or' expression, if there is one
            ///
            /// When begin_or() is called, it creates a 'final' state, which is reached once any of the alternatives is accepted.
            /// This call creates an epsilon transition to this point, so following transitions will be after both sides of the
            /// expression.
            ///
            void rejoin();
        };
        
        /// \brief Creates a new NDFA constructor
        inline constructor get_cons() { return constructor(this); }
        
        /// \brief Starts a new chain of transitions, starting at state 0
        inline constructor operator>>(const symbol_set& symbols) {
            return constructor(this) >> symbols;
        }
        
        inline constructor operator>>(char c)      { return operator>>(range<int>((int)c, (int)c+1)); }
        inline constructor operator>>(wchar_t c)   { return operator>>(range<int>((int)c, (int)c+1)); }
        inline constructor operator>>(const state& firstState) {
            return constructor(this) >> firstState;
        }
        
    private:
        /// \brief Internal method: computes the closure of the specified set of states (modifies the set to include 
        /// all states reachable by epsilon transitions)
        void closure(std::set<int>& states) const;
        
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
