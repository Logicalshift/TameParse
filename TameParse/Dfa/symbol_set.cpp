//
//  symbol_set.cpp
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
//

#include "TameParse/Dfa/symbol_set.h"

using namespace std;
using namespace dfa;

/// \brief Creates an empty symbol set
symbol_set::symbol_set() {
}

/// \brief Creates set containing a range of symbols
symbol_set::symbol_set(const symbol_range& symbol) {
    m_Symbols.insert(symbol);
}

/// \brief Creates a new symbol set by copying an old one
symbol_set::symbol_set(const symbol_set& copyFrom) 
: m_Symbols(copyFrom.m_Symbols) {
}

/// \brief Creates an empty symbol set
symbol_set::symbol_set(const epsilon& empty) {
}

/// \brief Destructor
symbol_set::~symbol_set() {
}


/// \brief Merges this symbol set with another
symbol_set& symbol_set::operator|=(const symbol_set& mergeWith) {
    // Merge each of the ranges in turn
    // TODO: this will perform a search for each range, we can take advantage of the ordering to improve performance here
    for (symbol_store::const_iterator symRange = mergeWith.m_Symbols.begin(); symRange != mergeWith.m_Symbols.end(); ++symRange) {
        operator|=(*symRange);
    }
    
    return *this;
}

/// \brief Merges this symbol set with a range of symbols
symbol_set& symbol_set::operator|=(const symbol_range& mergeWith) {
    // Find the first range >= the merge point
    // We've modified the comparator used by the set so that only the lower bound is compared, so this is the first item with a lower bound >= the
    // new range (additionally, we maintain the set in such a way that there are no overlapping ranges)
    symbol_store::iterator firstGreaterThan = m_Symbols.lower_bound(mergeWith);
    
    // If this isn't at the beginning, it's possible we partially overlap the preceeding set
    if (firstGreaterThan != m_Symbols.begin()) {
        // Get the preceeding set
        symbol_store::iterator previous = firstGreaterThan;
        previous--;
        
        // See if we overlap
        if (previous->upper() >= mergeWith.lower()) {
            // They do: start merging at the earlier set
            firstGreaterThan = previous;
        }
    }
    
    // If we're at the end of the list, then we can just add the new range (there's no overlap)
    if (firstGreaterThan == m_Symbols.end() || !mergeWith.can_merge(*firstGreaterThan)) {
        m_Symbols.insert(mergeWith);
        return *this;
    }
    
    // There's some overlap between ranges
    
    // Find the first item that is > the upper value (ie, the first item we don't merge with)
    symbol_store::iterator  finalValue  = m_Symbols.upper_bound(mergeWith.upper());
    symbol_range            mergedRange = mergeWith;

    // Merge any ranges that overlap
    for (symbol_store::iterator mergeWith = firstGreaterThan; mergeWith != finalValue; ++mergeWith) {
        mergedRange = mergedRange.merge(*mergeWith);
    }
    
    // Replace the merged values
    m_Symbols.erase(firstGreaterThan, finalValue);
    m_Symbols.insert(mergedRange);
    
    return *this;
}

/// \brief Excludes a range of symbols from this set
void symbol_set::exclude(const symbol_set& toExclude) {
    // Exclude each of the ranges in turn
    // TODO: this will perform a search for each range, we can take advantage of the ordering to improve performance here
    for (symbol_store::const_iterator excluded = toExclude.m_Symbols.begin(); excluded != toExclude.m_Symbols.end(); ++excluded) {
        exclude(*excluded);
    }
}

/// \brief Excludes a range of symbols from this set
void symbol_set::exclude(const symbol_range& exclude) {
    // Find the first range >= the merge point
    symbol_store::iterator firstGreaterThan = m_Symbols.lower_bound(exclude);
    
    // If this isn't at the beginning, it's possible we partially overlap the preceeding set
    if (firstGreaterThan != m_Symbols.begin()) {
        // Get the preceeding set
        symbol_store::iterator previous = firstGreaterThan;
        previous--;
        
        // See if we overlap
        if (previous->upper() >= exclude.lower()) {
            // They do: start merging at the earlier set
            firstGreaterThan = previous;
        }
    }
    
    // If we're at the end of the list, then there's nothing to do (there's no overlap)
    if (firstGreaterThan == m_Symbols.end() || !exclude.can_merge(*firstGreaterThan)) {
        return;
    }
    
    // There's some overlap between ranges
    
    // Find the first item that is > the upper value
    symbol_store::iterator finalValue  = m_Symbols.upper_bound(exclude.upper());
    
    // Change it to the final value in the range affected by this exclusion
    symbol_store::iterator eraseTo = finalValue;
    finalValue--;
    
    // Get the initial and final ranges
    symbol_range initial = *firstGreaterThan;
    symbol_range final   = *finalValue;
    
    // Erase the ranges with excluded characters
    m_Symbols.erase(firstGreaterThan, eraseTo);
    
    // Add new ranges
    if (initial.lower() < exclude.lower()) {
        m_Symbols.insert(symbol_range(initial.lower(), exclude.lower()));
    }
    
    if (final.upper() > exclude.upper()) {
        m_Symbols.insert(symbol_range(exclude.upper(), final.upper()));
    }
}

/// \brief Inverts this set
void symbol_set::invert() {
    // The last symbol range that we've seen
    symbol_range last(0,0);
    symbol_store newRanges;
    
    // Add the ranges that are excluded from this set (except for the range to the maximum symbol)
    for (symbol_store::iterator excludedRange = m_Symbols.begin(); excludedRange != m_Symbols.end(); ++excludedRange) {
        // Insert a range from the last known position to the next position
        if (excludedRange->lower() != last.upper()) {
            newRanges.insert(symbol_range(last.upper(), excludedRange->lower()));
        }
        
        // Update the last position
        last = *excludedRange;
    }
    
    // Add a range from the last position to the maximum symbol number
    if (last.upper() < c_MaxSymbol) {
        newRanges.insert(symbol_range(last.upper(), c_MaxSymbol));
    }
    
    // Store in this object
    m_Symbols.swap(newRanges);
}

/// \brief True if the specified symbol is in this set
bool symbol_set::operator[](int symbol) {
    // Find the item nearest this symbol
    symbol_store::iterator nearestValue = m_Symbols.upper_bound(symbol);
    
    // Doesn't exist if this is at the start
    if (nearestValue == m_Symbols.begin()) return false;
    
    // Move back to the first value with a value <= the symbol
    nearestValue--;
    
    // See if this character is contained in this range
    return (*nearestValue)[symbol];
}

/// \brief Determines if this set represents the same as another set
bool symbol_set::operator==(const symbol_set& compareTo) const {
    if (&compareTo == this) return true;
    
    // The symbol sets must be the same size
    if (compareTo.m_Symbols.size() != m_Symbols.size()) return false;
    
    // The contents of all of the sets must be the same
    for (symbol_store::const_iterator ourSymbol = m_Symbols.begin(), theirSymbol = compareTo.m_Symbols.begin(); 
         ourSymbol != m_Symbols.end();
         ++ourSymbol, ++theirSymbol) {
        if (*ourSymbol != *theirSymbol) return false;
    }
    
    return true;
}

/// \brief Orders this symbol set
bool symbol_set::operator<(const symbol_set& compareTo) const {
    if (&compareTo == this) return false;
    
    // Is less than if there are fewer symbols
    if (m_Symbols.size() < compareTo.m_Symbols.size()) return true;
    if (compareTo.m_Symbols.size() < m_Symbols.size()) return false;
    
    // Is less than if there's a difference between any of the sets
    for (symbol_store::const_iterator ourSymbol = m_Symbols.begin(), theirSymbol = compareTo.m_Symbols.begin(); 
         ourSymbol != m_Symbols.end(); 
         ++ourSymbol, ++theirSymbol) {
        if (*ourSymbol == *theirSymbol) continue;
        return *ourSymbol < *theirSymbol;
    }

    // Sets are the same
    return false;
}

/// \brief Orders this symbol set
bool symbol_set::operator<=(const symbol_set& compareTo) const {
    if (&compareTo == this) return true;
    
    // Is less than if there are fewer symbols
    if (m_Symbols.size() < compareTo.m_Symbols.size()) return true;
    if (compareTo.m_Symbols.size() < m_Symbols.size()) return false;
    
    // Is less than if there's a difference between any of the sets
    for (symbol_store::const_iterator ourSymbol = m_Symbols.begin(), theirSymbol = compareTo.m_Symbols.begin();
         ourSymbol != m_Symbols.end(); ++ourSymbol, ++theirSymbol) {
        if (*ourSymbol == *theirSymbol) continue;
        return *ourSymbol < *theirSymbol;
    }
    
    // Sets are the same
    return true;
}
