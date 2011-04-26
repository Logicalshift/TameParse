//
//  remapped_symbol_set.h
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_REMAPPED_SYMBOL_MAP_H
#define _DFA_REMAPPED_SYMBOL_MAP_H

#include <map>
#include <set>

#include "Dfa/symbol_map.h"

namespace dfa {
    /// \brief Class that represents a symbol map that has been remapped from another symbol map
    class remapped_symbol_map : public symbol_map {
    private:
        /// \brief Represents the set of new symbol sets that an old symbol set was remapped to
        typedef std::set<int> new_symbols;
        
        /// \brief Maps IDs in the old symbol set to IDs in this map
        typedef std::map<int, new_symbols> old_to_new_map;
        
        /// \brief Maps IDs in the old set to IDs in this map
        old_to_new_map m_OldToNew;
        
    public:
        /// \brief Generates an empty remapped symbol map
        remapped_symbol_map();
        
        /// \brief Copy constructor
        remapped_symbol_map(const remapped_symbol_map& copyFrom);
        
        /// \brief Factory method that generates a remapped symbol map by removing duplicates
        ///
        /// This finds all the symbol sets that overlap in the original, and splits them up so that any given symbol is only in one set.
        /// It sets up the remapping so it is possible to find the new set IDs for any symbol set in the original.
        static remapped_symbol_map* deduplicate(const symbol_map& source);
    };
}

#endif
