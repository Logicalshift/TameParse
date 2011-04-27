//
//  symbol_translator.cpp
//  Parse
//
//  Created by Andrew Hunter on 27/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "symbol_translator.h"

using namespace dfa;

/// \brief Copy constructor
symbol_translator::symbol_translator(const symbol_translator& copyFrom) 
: m_Table(copyFrom.m_Table) {
}

/// \brief Creates a translator from a map containing non-overlapping symbol sets
///
/// Use remapped_symbol_map::deduplicate() to create a symbol map that is appropriate for this call, or ndfa::to_ndfa_with_unique_symbols()
/// to remap an NDFA so that it uses unique symbols.
symbol_translator::symbol_translator(const symbol_map& map) {
    // Iterate through the symbol sets in the map
    for (symbol_map::iterator setIt = map.begin(); setIt != map.end(); setIt++) {
        // Iterate through the rangefs in each set
        for (symbol_set::iterator rangeIt = setIt->first.begin(); rangeIt != setIt->first.end(); setIt++) {
            // Add each range in turn
            add_range(*rangeIt, setIt->second);
        }
    }
}
