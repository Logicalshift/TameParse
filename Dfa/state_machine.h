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
    /// \brief Table row for a state_machine, using a flat representation
    ///
    /// This row type is suitable for state machines that tend to have fully populated states. Lexers for most languages have this
    /// property. This will be inefficient with state machines with states that tend to be partially populated: simple regular
    /// expressions tend to work this way.
    ///
    /// Lookups in this kind of table will generally be very fast.
    ///
    /// This should really be an inner class of state_machine, but C++ just sucks too much to forward declare it so we can use
    /// it as a template argument.
    ///
    class state_machine_flat_table {
    private:
        /// \brief Row data
        int* m_Row;
        
    public:
        /// \brief Initialises an invalid row
        inline state_machine_flat_table()
        : m_Row(NULL) {
        }
        
        /// \brief Initialises this row
        void fill(int maxSet, const state& thisState) {
            // Allocate the row
            m_Row = new int[maxSet];
            
            // Fill in the default values
            for (int x=0; x<maxSet; x++) {
                m_Row[x] = -1;
            }
            
            // Fill in the transitions
            for (state::iterator transit = thisState.begin(); transit != thisState.end(); transit++) {
                m_Row[transit->symbol_set()] = transit->new_state();
            }
        }
        
        /// \brief Destroys this row
        inline ~state_machine_flat_table() {
            delete[] m_Row;
        }
        
        /// \brief Looks up the state for a given symbol set (which must be greater than 0 and less than the maxSet value passed into the constructor)
        inline int operator[](int symbolSet) const {
            return m_Row[symbolSet];
        }
    };
    
    ///
    /// \brief Class that represents a deterministic finite automaton (DFA)
    ///
    /// This class stores the state machine associated with a DFA in a way that is efficient to run. It's efficient in memory if most states have 
    /// transitions for most symbol sets (state machines for the lexers for many languages have this property, but state machines for matching
    /// single regular expressions tend not to)
    ///
    /// row_type can be adjusted to change how data for individual states are stored. The default type is 
    ///
    template<class symbol_type, class row_type = state_machine_flat_table> class state_machine {
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
        row_type* m_States;
        
    public:
        /// \brief Builds up a state machine from a DFA
        ///
        /// To prepare an NDFA for this call, you must call to_ndfa_with_unique_symbols and to_dfa on it first. This call will not produce
        /// an error if this is not done, but the state machine will not be correct. An ndfa containing transitions with invalid states or
        /// symbol set identifiers will produce a state machine that will generate a crash.
        state_machine(const ndfa& dfa) 
        : m_Translator(dfa.symbols())
        , m_MaxState(dfa.count_states())
        , m_MaxSet(dfa.symbols().count_sets()) {
            // Allocate the states array
            m_States = new row_type[m_MaxState];
            
            // Process the states
            for (int stateNum=0; stateNum<m_MaxState; stateNum++) {
                // Fetch the next state
                const state& thisState = dfa.get_state(stateNum);
                
                // Fill this row in
                m_States[stateNum].fill(m_MaxSet, thisState);
            }
        }
        
        /// \brief Destructor
        virtual ~state_machine() {
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
        
    public:
        /// \brief Row type that generates a flat table
        typedef state_machine_flat_table flat_table;

    public:
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
