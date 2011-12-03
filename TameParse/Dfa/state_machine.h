//
//  state_machine.h
//  Parse
//
//  Created by Andrew Hunter on 27/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_STATE_MACHINE_H
#define _DFA_STATE_MACHINE_H

#include <algorithm>

#include "TameParse/Dfa/symbol_translator.h"
#include "TameParse/Dfa/ndfa.h"
#include "TameParse/Dfa/epsilon.h"

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
    class state_machine_flat_table {
    private:
        /// \brief Row data
        int* m_Row;
        
        state_machine_flat_table(const state_machine_flat_table& copyFrom);
        state_machine_flat_table& operator=(state_machine_flat_table& copyFrom);
        
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
            for (int x=0; x<maxSet; ++x) {
                m_Row[x] = -1;
            }
            
            // Fill in the transitions
            for (state::iterator transit = thisState.begin(); transit != thisState.end(); ++transit) {
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
        
        /// \brief Total size of this row
        inline size_t size(int maxSet) const {
            return sizeof(*this) + sizeof(int) * maxSet;
        }
    };
    
    
    ///
    /// \brief Row type that generates a compact table
    ///
    /// This requires a binary search to find a symbol in a row, so the resulting state machine will be slower than one with a flat table.
    /// However, the size of the table will be much smaller in cases where the states are not fully populated (specifically, in cases where
    /// the average number of transitions per state is less than 50% of the number of symbol sets)
    ///
    /// You can supply this class as the row_type parameter in the state_machine templated class.
    ///
    template<bool deleteRows = true> class state_machine_compact_table {
    public:
        /// \brief Symbol set, state pair
        struct entry {
            int symbolSet, state; 
        };
        
    private:
        /// \brief Number of entries in this row
        int m_NumEntries;
        
        /// \brief List of symbol set, state pairs, sorted by symbol set
        entry* m_Row;
        
    public:
        /// \brief Orders two entries
        inline static bool compare_entries(const entry& a, const entry& b) {
            return a.symbolSet < b.symbolSet;
        }
        
    public:
        /// \brief Initialises an invalid row
        inline state_machine_compact_table()
        : m_Row(NULL)
        , m_NumEntries(0) {
        }

        /// \brief Initialises a row from an existing array of entries
        inline state_machine_compact_table(entry* first, entry* last)
        : m_Row(first)
        , m_NumEntries(last - first) {
        }
        
        /// \brief Initialises this row
        void fill(int maxSet, const state& thisState) {
            // Allocate the row
            m_NumEntries    = thisState.count_transitions();
            m_Row           = new entry[m_NumEntries];
            
            // Fill in the transitions
            int pos = 0;
            for (state::iterator transit = thisState.begin(); transit != thisState.end(); ++transit) {
                m_Row[pos].symbolSet    = transit->symbol_set();
                m_Row[pos].state        = transit->new_state();
                
                ++pos;
            }
            
            // Sort the entries
            std::sort(m_Row + 0, m_Row + m_NumEntries, compare_entries);
        }
        
        /// \brief Destroys this row
        inline ~state_machine_compact_table() {
            if (deleteRows) {
                delete[] m_Row;
            }
        }
        
        /// \brief Looks up the state for a given symbol set (which must be greater than 0 and less than the maxSet value passed into the constructor)
        inline int operator[](int symbolSet) const {
            // Perform a binary search for this item
            entry   searchFor   = { symbolSet, 0 };
            entry*  lowerBound  = std::lower_bound(m_Row + 0, m_Row + m_NumEntries, searchFor, compare_entries);
            
            // Return nothing if the symbol wasn't found
            if (lowerBound == m_Row + m_NumEntries) return -1;
            
            // Return nothing if we didn't find the right symbol
            if (lowerBound->symbolSet != symbolSet) return -1;
            
            // Return this transition
            return lowerBound->state;
        }
        
        /// \brief Total size of this row
        inline size_t size(int maxSet) const {
            return sizeof(*this) + sizeof(entry) * m_NumEntries;
        }
    };

    // TODO: document the requirements for the row type class and the symbol_translator class
    
    ///
    /// \brief Class that represents a deterministic finite automaton (DFA)
    ///
    /// This class stores the state machine associated with a DFA in a way that is efficient to run. It's efficient in memory if most states have 
    /// transitions for most symbol sets (state machines for the lexers for many languages have this property, but state machines for matching
    /// single regular expressions tend not to)
    ///
    /// row_type can be adjusted to change how data for individual states are stored. The default type is 
    ///
    template<class symbol_type, class row_type = state_machine_flat_table, class symbol_translator = symbol_translator<symbol_type>, bool deleteRows = true> class state_machine {
    private:
        /// \brief The translator for the symbols 
        symbol_translator m_Translator;
        
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
            for (int stateNum=0; stateNum<m_MaxState; ++stateNum) {
                // Fetch the next state
                const state& thisState = dfa.get_state(stateNum);
                
                // Fill this row in
                m_States[stateNum].fill(m_MaxSet, thisState);
            }
        }

        /// \brief Creates a state machine from a constant table value
        state_machine(const symbol_translator& translator, int maxSet, int maxState, row_type* states)
        : m_Translator(translator)
        , m_MaxSet(maxSet)
        , m_MaxState(maxState)
        , m_States(states) {
        }
        
        /// \brief Destructor
        virtual ~state_machine() {
            if (deleteRows) {
                delete[] m_States;
            }
        }
        
        /// \brief Size in bytes of this state machine
        inline size_t size() const {
            size_t mySize = sizeof(*this);
            mySize += m_Translator.size() + sizeof(m_Translator);
            
            for (int x=0; x<m_MaxState; ++x) {
                mySize += m_States[x].size(m_MaxSet);
            }

            return mySize;
        }

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

    ///
    /// \brief State machine used with hard-coded tables generated by the main parser generator
    ///
    template<class symbol_type, class symbol_translator> class state_machine_tables {
    public:
        /// \brief Type of an entry in this state machine (same as a compact table entry)
        typedef state_machine_compact_table<false>::entry entry;

    private:
        /// \brief Translates a raw symbol into the corresponding symbol set
        const symbol_translator& m_Translator;

        /// \brief The state machine represented by this object
        ///
        /// We currently only support the compact table representation. This table consists
        /// of a series of increasing pointers. m_StateEntries[x] is the location of the
        /// beginning of the entries for state x, and m_StateEntries[x+1] is the location of
        /// the end of the entries for state x.
        const entry** m_StateEntries;

        /// \brief The maximum state ID
        const int m_MaxState;
    
    public:
        state_machine_tables(const symbol_translator& translator, const entry** entries, int numStates)
        : m_Translator(translator)
        , m_StateEntries(entries)
        , m_MaxState(numStates) {
        }

    public:
        /// \brief Size in bytes of this table
        inline size_t size() const {
            // TODO, maybe: actually calculate the size from the entries
            return sizeof(*this);
        }

    public:
        /// \brief Given a state and a symbol set, returns a new state
        ///
        /// Unlike run() this performs no bounds checking so might crash or perform strangely when supplied with invalid state IDs or symbol sets
        inline int run_unsafe_set(int state, int symbolSet) const {
            // Fetch the state position
            const entry* stateStart   = m_StateEntries[state];
            const entry* stateEnd     = m_StateEntries[state+1];

            // Nothing to do if there are no entries for this state
            if (stateEnd == stateStart) return -1;

            // Search for this entry
            entry   searchFor   = { symbolSet, 0 };
            const entry*  lowerBound  = std::lower_bound(stateStart, stateEnd, searchFor, state_machine_compact_table<false>::compare_entries);
            
            // Return nothing if the symbol wasn't found
            if (lowerBound == stateEnd) return -1;
            
            // Return nothing if we didn't find the right symbol
            if (lowerBound->symbolSet != symbolSet) return -1;
            
            // Return this transition
            return lowerBound->state;
        }

        /// \brief Given a state and a symbol, returns a new state
        ///
        /// Unlike run() this performs no bounds checking so might crash or perform strangely when supplied with invalid state IDs
        ///
        /// For most DFAs, state 0 is always present, and this call will not return an invalid state (other than -1 to indicate a rejection).
        /// It is guaranteed not to crash provided you supply either state 0 or a state returned by this call that is not -1.
        inline int run_unsafe(int state, symbol_type symbol) const {
            // Get the set this symbol is in
            int set = m_Translator.lookup(symbol);
            
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
