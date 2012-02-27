//
//  symbol_map.h
//  Parse
//
//  Created by Andrew Hunter on 19/03/2011.
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

#ifndef _DFA_SYMBOL_MAP_H
#define _DFA_SYMBOL_MAP_H

#include <vector>
#include <map>

#include "TameParse/Dfa/symbol_set.h"

namespace dfa {
    ///
    /// \brief Class that maps symbol sets to identifiers
    ///
    class symbol_map {
    private:
        /// \brief Storage type
        typedef std::vector<symbol_set_container> symbol_set_map;
        typedef std::map<symbol_set_container, int> symbol_id_map;
        
        /// \brief Maps identifiers to symbols
        symbol_set_map m_SymbolsForId;
        
        /// \brief Maps symbol sets to identifiers
        symbol_id_map m_IdForSymbols;

    public:
        /// \brief Iterator
        typedef symbol_id_map::const_iterator iterator;
        
        /// \brief Constructor
        symbol_map();
        
        /// \brief Copy constructor
        symbol_map(const symbol_map& copyFrom);
        
        /// \brief Destructor
        virtual ~symbol_map();
        
        /// \brief Returns an identifier for a set of symbols (assigning a new one as needed)
        ///
        /// Symbol set identifiers are assigned from 0 upwards
        int identifier_for_symbols(const symbol_set& symbols);
        
        /// \brief Returns an identifier for a set of symbols (returns -1 if the symbols aren't present in this map)
        int find_identifier_for_symbols(const symbol_set& symbols) const;
        
        /// \brief Number of symbol sets in this map
        inline int count_sets() const { return (int)m_SymbolsForId.size(); }
        
        /// \brief Returns the symbol set for a particular identifier
        const symbol_set& operator[](int identifier) const;
        
        /// \brief First symbol set/ID pair
        inline iterator begin() const { return m_IdForSymbols.begin(); }
        
        /// \brief Last symbol set/ID pair
        iterator end() const { return m_IdForSymbols.end(); }
        
    public:
        /// \brief Testing method: (inefficiently) iterates through the symbols in this map and returns true if there are any duplicate symbols
        ///
        /// It is not invalid for a symbol map to have overlapping ranges in it. However, a remapped set which has been
        /// deduplicated should have no overlaps in it (ie, any symbol map that is intended for use in a DFA should not
        /// have any overlapping sets in it)
        bool has_duplicates() const;
    };
}

#endif
