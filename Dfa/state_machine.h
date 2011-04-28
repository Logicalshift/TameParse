//
//  state_machine.h
//  Parse
//
//  Created by Andrew Hunter on 27/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_STATE_MACHINE_H
#define _DFA_STATE_MACHINE_H

#include "Dfa/symbol_translator.h"
#include "Dfa/ndfa.h"
#include "Dfa/epsilon.h"

namespace dfa {
    ///
    /// \brief Class that represents a deterministic finite automaton (DFA)
    ///
    /// This class stores the 
    ///
    template<class symbol_type> class state_machine {
    private:
        /// \brief The translator for the symbols 
        symbol_translator<symbol_type> m_Translator;
        
        /// \brief The maximum symbol set
        int m_MaxSet;
        
        /// \brief The maximum state ID
        int m_MaxState;
        
        /// \brief The state table (one row per state, m_MaxSet entries per row)
        ///
        /// Each entry can be -1 to indicate a rejection, or the state to move to
        int** m_States;
        
    public:
        /// \brief Builds up a state machine from a DFA
        ///
        /// To prepare an NDFA for this call, you must call to_ndfa_with_unique_symbols and to_dfa on it first. This call will not produce
        /// an error if this is not done, but the state machine will not be correct.
        state_machine(const ndfa& dfa) 
        : m_Translator(dfa.symbols())
        , m_MaxState(dfa.count_states())
        , m_MaxSet(dfa.symbols().count_sets()) {
            // Allocate the states array
            m_States = new int*[m_MaxState];
            
            // Process the states
            for (int stateNum=0; stateNum<m_MaxState; stateNum++) {
                // Fetch the next state
                const state& thisState = dfa.get_state(stateNum);
                
                // Allocate the array for this state
                int* stateData      = new int[m_MaxSet];
                m_States[stateNum]  = stateData;
                
                // Fill in as blank
                for (int x=0; x<m_MaxSet; x++) stateData[x] = -1;
                
                // Fill in the transitions
                for (state::iterator transit = thisState.begin(); transit != thisState.end(); transit++) {
                    stateData[transit->symbol_set()] = transit->new_state();
                }
            }
        }
        
        /// \brief Destructor
        virtual ~state_machine() {
            for (int x=0; x<m_MaxState; x++) {
                delete[] m_States[x];
            }
            delete[] m_States;
        }
        
        /// \brief Size in bytes of this state machine
        inline size_t size() {
            size_t mySize = sizeof(*this);
            mySize += m_Translator.size() + sizeof(m_Translator);
            mySize += sizeof(int*[m_MaxState]);
            mySize += sizeof(int[m_MaxSet])*m_MaxState;
            return mySize;
        }

        /// \brief Given a state and a symbol set, returns a new state
        ///
        /// Unlike run() this performs no bounds checking so might crash or perform strangely when supplied with invalid state IDs or symbol sets
        inline int run_unsafe_set(int state, int symbolSet) const {
            return m_States[state][symbolSet];
        }

        /// \brief Given a state and a symbol, returns a new state
        ///
        /// Unlike run() this performs no bounds checking so might crash or perform strangely when supplied with invalid state IDs
        ///
        /// For most DFAs, state 0 is always present, and this call will not return an invalid state (other than -1 to indicate a rejection).
        /// It is guaranteed not to crash provided you supply either state 0 or a state returned by this call that is not -1.
        inline int run_unsafe(int state, symbol_type symbol) const {
            // Get the set this symbol is in
            int set = m_Translator.set_for_symbol(symbol);
            
            // Reject symbols that have no set
            if (set == symbol_set::null) return -1;
            
            // Run with this set
            return run_unsafe_set(state, set);
        }
        
        /// \brief Given a state and a symbol, returns a new state
        inline int run(int state, symbol_type symbol) const {
            if (state < 0 || state >= m_MaxState) return -1;
            return run_unsafe(state, symbol);
        }
    };
}

#endif
