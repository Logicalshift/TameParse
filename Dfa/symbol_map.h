//
//  symbol_map.h
//  Parse
//
//  Created by Andrew Hunter on 19/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_SYMBOL_MAP_H
#define _DFA_SYMBOL_MAP_H

#include <vector>
#include <map>

#include "Dfa/symbol_set.h"

namespace dfa {
    ///
    /// \brief Class that maps symbol sets to identifiers
    ///
    class symbol_map {
    private:
        /// \brief Storage type
        typedef std::vector<const symbol_set*> symbol_set_map;
        typedef std::map<const symbol_set, int> symbol_id_map;
        
        /// \brief Maps identifiers to symbols
        symbol_set_map m_SymbolsForId;
        
        /// \brief Maps symbol sets to identifiers
        symbol_id_map m_IdForSymbols;

    public:
        /// \brief Iterator
        typedef symbol_id_map::const_iterator iterator;
        
        /// \brief Constructor
        symbol_map();
        
        /// \brief Destructor
        virtual ~symbol_map();
        
        /// \brief Returns an identifier for a set of symbols (assigning a new one as needed)
        int identifier_for_symbols(const symbol_set& symbols);
        
        /// \brief Returns an identifier for a set of symbols (returns -1 if the symbols aren't present in this map)
        int identifier_for_symbols(const symbol_set& symbols) const;
        
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
