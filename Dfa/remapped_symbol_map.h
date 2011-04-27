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
    ///
    /// \brief Class that represents a symbol map that has been remapped from another symbol map
    ///
    class remapped_symbol_map : public symbol_map {
    public:
        /// \brief Represents the set of new symbol sets that an old symbol set was remapped to
        typedef std::set<int> new_symbols;

    private:
        /// \brief Constant value indicating 'no symbols'
        static const new_symbols s_NoSymbols;
        
        /// \brief Maps IDs in the old symbol set to IDs in this map
        typedef std::map<int, new_symbols> old_to_new_map;
        
        /// \brief Maps IDs in the old set to IDs in this map
        old_to_new_map m_OldToNew;
        
    public:
        /// \brief Generates an empty remapped symbol map
        remapped_symbol_map();
        
        /// \brief Copy constructor
        remapped_symbol_map(const remapped_symbol_map& copyFrom);
        
        /// \brief Finds or adds an identifier for the specified symbol, adding the specified set of new symbols as the new symbols for this set
        int identifier_for_symbols(const symbol_set& symbols, const new_symbols& newSymbols);
        
        /// \brief Given a symbol set ID in this set, returns the old sets that contained it
        inline const new_symbols& old_symbols(int newSetId) const { 
            old_to_new_map::const_iterator found = m_OldToNew.find(newSetId);
            if (found != m_OldToNew.end()) return found->second;
            return s_NoSymbols;
        }
        
        /// \brief Factory method that generates a remapped symbol map by removing duplicates
        ///
        /// This finds all the symbol sets that overlap in the original, and splits them up so that any given symbol is only in one set.
        /// It sets up the remapping so it is possible to find the new set IDs for any symbol set in the original.
        static remapped_symbol_map* deduplicate(const symbol_map& source);
    };
}

#endif
