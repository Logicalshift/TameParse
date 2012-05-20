//
//  symbol_translator.h
//  Parse
//
//  Created by Andrew Hunter on 27/04/2011.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the \"Software\"), to 
//  deal in the Software without restriction, including without limitation the 
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
//  sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
//  IN THE SOFTWARE.
//

#ifndef _DFA_SYMBOL_TRANSLATOR_H
#define _DFA_SYMBOL_TRANSLATOR_H

#include "TameParse/Dfa/symbol_map.h"
#include "TameParse/Dfa/symbol_table.h"

namespace dfa {
    ///
    /// \brief Class used in a DFA to map an input symbol to a symbol set ID
    ///
    template<class symbol_type = int, class symbol_table_type = symbol_table<symbol_type> > class symbol_translator {
    private:
        /// \brief The table for this item
        symbol_table_type m_Table;
        
        /// \brief Disabled assignment
        symbol_translator<symbol_type, symbol_table_type>& operator=(const symbol_translator<symbol_type, symbol_table_type>& assignFrom);
        
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
            for (symbol_map::iterator setIt = map.begin(); setIt != map.end(); ++setIt) {
                // Iterate through the ranges in each set
                for (symbol_set::iterator rangeIt = setIt->first->begin(); rangeIt != setIt->first->end(); ++rangeIt) {
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
        inline int set_for_symbol(symbol_type symbol) const {
            return m_Table.lookup(symbol);
        }
        
        // \brief Returns the number of bytes required by the table
        inline size_t size() const {
            return m_Table.table.size();
        }
    };
}

#endif
