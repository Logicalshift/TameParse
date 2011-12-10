//
//  symbol_map.cpp
//  Parse
//
//  Created by Andrew Hunter on 19/03/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
//

#include "TameParse/Dfa/symbol_map.h"
#include "TameParse/Dfa/epsilon.h"

using namespace std;
using namespace dfa;

static symbol_set nosymbols;

/// \brief Constructor
symbol_map::symbol_map() {
}

/// \brief Destructor
symbol_map::~symbol_map() {
}

/// \brief Copy constructor
symbol_map::symbol_map(const symbol_map& copyFrom)
: m_IdForSymbols(copyFrom.m_IdForSymbols)
, m_SymbolsForId(copyFrom.m_SymbolsForId) {
}

/// \brief Returns an identifier for a set of symbols (assigning a new one as needed)
int symbol_map::identifier_for_symbols(const symbol_set& symbols) {
    // Try to find this symbol set in this object
    symbol_id_map::const_iterator oldSymbols = m_IdForSymbols.find(symbols);
    
    // Use the old ID if there already was one
    if (oldSymbols != m_IdForSymbols.end()) {
        return oldSymbols->second;
    }
    
    // Create a new ID
    int newId = (int)m_SymbolsForId.size();
    
    // Add to the symbol map
    symbol_id_map::iterator newSymbols = m_IdForSymbols.insert(pair<symbol_set_container, int>(symbols, newId)).first;
    
    // Add to the ID vector
    m_SymbolsForId.push_back(newSymbols->first);
    
    // Return the new ID
    return newId;
}

/// \brief Returns an identifier for a set of symbols (returns -1 if the symbols aren't present in this map)
int symbol_map::find_identifier_for_symbols(const symbol_set& symbols) const {
    // Try to find this symbol set in this object
    symbol_id_map::const_iterator oldSymbols = m_IdForSymbols.find(symbols);
    
    // Use the old ID if there already was one
    if (oldSymbols != m_IdForSymbols.end()) {
        return oldSymbols->second;
    }
    
    // Return -1 if the symbols weren't found
    return -1;
}

/// \brief Returns the symbol set for a particular identifier
const symbol_set& symbol_map::operator[](int identifier) const {
    if (identifier < 0 || identifier >= (int) m_SymbolsForId.size()) return nosymbols;
    return *m_SymbolsForId[identifier];
}

/// \brief Testing method: (inefficiently) iterates through the symbols in this map and returns true if there are any duplicate symbols
bool symbol_map::has_duplicates() const {
    // Iterate through all of the symbols in the map
    for (symbol_map::iterator checkSet = begin(); checkSet != end(); ++checkSet) {
        // Iterate through all of the ranges in this set
        for (symbol_set::iterator checkRange = checkSet->first->begin(); checkRange != checkSet->first->end(); ++checkRange) {
            // Check these against each other set (which isn't the same as this one)
            for (symbol_map::iterator againstSet = begin(); againstSet != end(); ++againstSet) {
                if (againstSet == checkSet) continue;
                
                for (symbol_set::iterator againstRange = againstSet->first->begin(); againstRange != againstSet->first->end(); ++againstRange) {
                    // Must not overlap
                    if (againstRange->overlaps(*checkRange)) return true;
                }
            }
        }
    }
    
    // Looks good
    return false;
}
