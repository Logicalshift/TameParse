//
//  remapped_symbol_set.cpp
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <map>
#include <set>

#include "remapped_symbol_map.h"
#include "range.h"

using namespace std;
using namespace dfa;

/// \brief Generates an empty remapped symbol map
remapped_symbol_map::remapped_symbol_map() {
}

/// \brief Copy constructor
remapped_symbol_map::remapped_symbol_map(const remapped_symbol_map& copyFrom) 
: m_OldToNew(copyFrom.m_OldToNew) {
}

/// \brief Type mapping symbol ranges to the sets that contain them
typedef map<range<int>, set<int>, range<int>::sort_by_lower > sets_for_range;

/// \brief Adds the ranges in a particular symbol set 
static void add_set(sets_for_range& sets, const symbol_set& symbols, int identifier) {
    // Iterate through the ranges in this set
    for (symbol_set::iterator range = symbols.begin(); range != symbols.end(); range++) {
        // Find the first item in the set whose lower bound is >= this range
        sets_for_range::iterator firstGreaterThan = sets.lower_bound(*range);
        
        // If this isn't at the beginning, then it's possible we overlap the preceeding sets
        if (firstGreaterThan != sets.begin()) {
            
        }
    }
}

/// \brief Factory method that generates a remapped symbol map by removing duplicates
///
/// This finds all the symbol sets that overlap in the original, and splits them up so that any given symbol is only in one set.
/// It sets up the remapping so it is possible to find the new set IDs for any symbol set in the original.
remapped_symbol_map* remapped_symbol_map::deduplicate(const symbol_map& source) {
    // Map of symbol ranges to the symbol sets in the original that contain them
    sets_for_range setsContainingRange;
    
    // Iterate through the sets in the source
    for (symbol_map::iterator symSet = source.begin(); symSet != source.end(); symSet++) {
        // Add this into the sets containing this range
        add_set(setsContainingRange, symSet->first, symSet->second);
    }
    
    return NULL;
}
