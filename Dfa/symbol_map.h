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
    /// \brief Class that maps symbol sets to identifiers
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
        
        /// \brief Destructor
        virtual ~symbol_map();
        
        /// \brief Returns an identifier for a set of symbols (assigning a new one as needed)
        int identifier_for_symbols(const symbol_set& symbols);
        
        /// \brief Returns the symbol set for a particular identifier
        const symbol_set& operator[](int identifier) const;
        
        /// \brief First symbol set/ID pair
        inline iterator begin() const { return m_IdForSymbols.begin(); }
        
        /// \brief Last symbol set/ID pair
        iterator end() const { return m_IdForSymbols.end(); }
    };
}

#endif
