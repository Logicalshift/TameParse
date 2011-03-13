//
//  symbol_set.cpp
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "symbol_set.h"

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

/// \brief Merges this symbol set with another
symbol_set& symbol_set::operator|=(const symbol_set& mergeWith) {
    return *this;
}

/// \brief Merges this symbol set with a range of symbols
symbol_set& symbol_set::operator|=(const symbol_range& mergeWith) {
    return *this;
}

/// \brief Excludes a range of symbols from this set
symbol_set& symbol_set::operator&=(const symbol_set& exclude) {
    return *this;
}

/// \brief Excludes a range of symbols from this set
symbol_set& symbol_set::operator&=(const symbol_range& exclude) {
    return *this;
}

/// \brief True if the specified symbol is in this set
bool symbol_set::operator[](int symbol) {
    return false;
}

/// \brief Determines if this set represents the same as another set
bool symbol_set::operator==(const symbol_set& compareTo) const {
    if (&compareTo == this) return true;
    
    return false;
}

/// \brief Orders this symbol set
bool symbol_set::operator<(const symbol_set& compareTo) const {
    if (&compareTo == this) return false;

    return false;
}

/// \brief Orders this symbol set
bool symbol_set::operator<=(const symbol_set& compareTo) const {
    if (&compareTo == this) return true;
    
    return false;
}
