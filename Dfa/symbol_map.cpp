//
//  symbol_map.cpp
//  Parse
//
//  Created by Andrew Hunter on 19/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "symbol_map.h"

using namespace std;
using namespace dfa;

static symbol_set nosymbols;

/// \brief Destructor
symbol_map::~symbol_map() {
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
    symbol_id_map::iterator newSymbols = m_IdForSymbols.insert(pair<symbol_set, int>(symbols, newId)).first;
    
    // Add to the ID vector
    m_SymbolsForId.push_back(&newSymbols->first);
    
    // Return the new ID
    return newId;
}

/// \brief Returns the symbol set for a particular identifier
const symbol_set& symbol_map::operator[](int identifier) const {
    if (identifier < 0 || identifier >= m_SymbolsForId.size()) return nosymbols;
    return *m_SymbolsForId[identifier];
}
