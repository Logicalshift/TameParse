//
//  symbol_translator.h
//  Parse
//
//  Created by Andrew Hunter on 27/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_SYMBOL_TRANSLATOR_H
#define _DFA_SYMBOL_TRANSLATOR_H

#include "Dfa/symbol_map.h"
#include "Dfa/symbol_table.h"

namespace dfa {
    ///
    /// \brief Class used in a DFA to map an input symbol to a symbol set ID
    ///
    template<class T = int> class symbol_translator {
    private:
        /// \brief The table for this item
        symbol_table<T> m_Table;
        
    public:
        /// \brief Copy constructor
        symbol_translator(const symbol_translator& copyFrom)
        : m_Table(copyFrom.m_Table) {
        }
        
        /// \brief Creates a translator from a map containing non-overlapping symbol sets
        ///
        /// Use remapped_symbol_map::deduplicate() to create a symbol map that is appropriate for this call, or ndfa::to_ndfa_with_unique_symbols()
        /// to remap an NDFA so that it uses unique symbols.
        symbol_translator(const symbol_map& map) {
            // Iterate through the symbol sets in the map
            for (symbol_map::iterator setIt = map.begin(); setIt != map.end(); setIt++) {
                // Iterate through the ranges in each set
                for (symbol_set::iterator rangeIt = setIt->first.begin(); rangeIt != setIt->first.end(); rangeIt++) {
                    // Add each range in turn
                    add_range(*rangeIt, setIt->second);
                }
            }
        }
        
        /// \brief Sets the symbol for the specified range
        inline void add_range(const range<int>& range, int symbol) {
            m_Table.add_range(range, symbol);
        }
        
        /// \brief Returns the ID of the symbol set of the specified symbol
        inline int set_for_symbol(T symbol) const {
            return m_Table.Lookup(symbol);
        }
        
        // \brief Returns the number of bytes required by the table
        inline size_t size() const {
            return m_Table.Table.size();
        }
    };
}

#endif