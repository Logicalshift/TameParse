//
//  remapped_symbol_set.cpp
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <map>
#include <set>

#include "remapped_symbol_map.h"
#include "range.h"

using namespace std;
using namespace dfa;

/// \brief Empty symbol set
const remapped_symbol_map::new_symbols remapped_symbol_map::s_NoSymbols;

/// \brief Generates an empty remapped symbol map
remapped_symbol_map::remapped_symbol_map() {
}

/// \brief Copy constructor
remapped_symbol_map::remapped_symbol_map(const remapped_symbol_map& copyFrom) 
: m_OldToNew(copyFrom.m_OldToNew) {
}

/// \brief Finds or adds an identifier for the specified symbol, adding the specified set of new symbols as the new symbols
int remapped_symbol_map::identifier_for_symbols(const symbol_set& symbols, const new_symbols& newSymbols) {
    // Get the set ID for these symbols
    int setId = symbol_map::identifier_for_symbols(symbols);
    
    // Try to look up the existing set
    old_to_new_map::iterator existing = m_OldToNew.find(setId);
    
    if (existing == m_OldToNew.end()) {
        // Just add these symbols
        m_OldToNew[setId] = newSymbols;
    } else {
        // Merge the sets
        for (new_symbols::iterator it = newSymbols.begin(); it != newSymbols.end(); it++) {
            existing->second.insert(*it);
        }
    }
    
    // Return this as the result
    return setId;
}

/// \brief Range of symbols
typedef range<int> symbol_range;

/// \brief Type mapping symbol ranges to the sets that contain them
typedef map<symbol_range, set<int>, symbol_range::sort_by_lower > sets_for_range;

/// \brief Splits a range that exists in sets into two.
///
/// We end up with two ranges: lower..splitPoint and splitPoint..upper
/// Returns an iterator for the start of the new ranges
static sets_for_range::iterator split(sets_for_range& sets, symbol_range original, int splitPoint) {
    // Get the values for this range
    set<int> values = sets[original];
    
    // Remove the original range
    sets.erase(original);
    
    // Create the split ranges
    sets[symbol_range(splitPoint, original.upper())] = values;
    sets_for_range::iterator result = sets.insert(sets_for_range::value_type(symbol_range(original.lower(), splitPoint), values)).first;
    return result;
}

/// \brief Adds the ranges in a particular symbol set 
static void add_set(sets_for_range& sets, const symbol_set& symbols, int identifier) {
    // Iterate through the ranges in this set
    for (symbol_set::iterator range = symbols.begin(); range != symbols.end(); range++) {
        // Find the first item in the set whose lower bound is >= this range
        sets_for_range::iterator firstGreaterThan = sets.lower_bound(*range);
        
        // If this isn't at the beginning, then it's possible that this set overlaps the preceeding set
        if (firstGreaterThan != sets.begin()) {
            // Get the previous set
            sets_for_range::iterator preceeding = firstGreaterThan;
            preceeding--;
            
            // See if there's an overlap
            if (preceeding->first.upper() >= range->lower()) {
                // Start from the preceeding set if they do
                firstGreaterThan = preceeding;
            }
        }
        
        // If we're at the end of the list here, then this is an entirely new range
        // This is also a new range if the item at firstGreaterThan doesn't overlap the range
        // Finally, we can just add to the set for a particular range if we get an exact match
        if (firstGreaterThan == sets.end() || !firstGreaterThan->first.overlaps(*range) || firstGreaterThan->first == *range) {
            // Just add this range
            sets[*range].insert(identifier);
            continue;
        }

        // We've got some overlap, but it's not exact
        symbol_range    remaining = *range;                 // Range of characters remaining to merge in
        set<int>        justUs;                             // Set containing only the new identifier
        
        justUs.insert(identifier);

        while (remaining.upper() != remaining.lower()) {
            cout << "Remaining: " << remaining.lower() << "-" << remaining.upper() << endl;
            cout << "Sets:";
            for (sets_for_range::iterator it = sets.begin(); it != sets.end(); it++) {
                cout << " " << it->first.lower() << "-" << it->first.upper();
            }
            cout << endl;
            if (firstGreaterThan == sets.end()) cout << "At end" << endl;
            else cout << "Position: " << firstGreaterThan->first.lower() << "-" << firstGreaterThan->first.upper() << endl;
            
            // If we're at the end, we can just add the remaining set and stop
            if (firstGreaterThan == sets.end()) {
                sets[remaining] = justUs;
                break;
            }
            
            // If there's a gap between the remaining set and the current position, then fill it in
            if (firstGreaterThan->first.lower() > remaining.lower()) {
                int newUpper = firstGreaterThan->first.lower();
                
                // Insert a value to fill the gap
                firstGreaterThan = sets.insert(firstGreaterThan, sets_for_range::value_type(symbol_range(remaining.lower(), newUpper), justUs));
                
                // Adjust the remaining set
                remaining = symbol_range(newUpper, remaining.upper());
                
                // Want the value after the one we just inserted
                firstGreaterThan++;
                
                // Start again
                continue;
            }
            
            // If there's a gap between the lowest point in the range we're currently splitting and the current range, then split there
            if (firstGreaterThan->first.lower() < remaining.lower()) {
                // Split the ranges and move so that the lowest point matches
                firstGreaterThan = split(sets, firstGreaterThan->first, remaining.lower());
                firstGreaterThan++;
            }
            
            // If the current range ends beyond the end of the remaining set, then split it
            if (firstGreaterThan->first.upper() > remaining.upper()) {
                firstGreaterThan = split(sets, firstGreaterThan->first, remaining.upper());
            }
            
            // Current range should overlap the range specified by firstGreaterThan: add this identifier
            firstGreaterThan->second.insert(identifier);
            
            // Adjust the remaining set
            remaining = symbol_range(firstGreaterThan->first.upper(), remaining.upper());
            
            // Move on
            firstGreaterThan++;
        }
                 
        cout << "Done adding" << endl;
        cout << "Sets:";
        for (sets_for_range::iterator it = sets.begin(); it != sets.end(); it++) {
            cout << " " << it->first.lower() << "-" << it->first.upper();
        }
        cout << endl;
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
    
    // Create the result
    remapped_symbol_map* newSet = new remapped_symbol_map();
    
    // Create a new set for each range we got in the previous step
    for (sets_for_range::iterator it = setsContainingRange.begin(); it != setsContainingRange.end(); it++) {
        newSet->identifier_for_symbols(it->first, it->second);
    }
    
    // This is the result
    return newSet;
}
