//
//  item.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "item.h"

using namespace contextfree;

/// \brief Destructor
item::~item() {
}

/// \brief Compares this item to another. Returns true if they are the same
bool item::operator==(const item& compareTo) const {
    return compareTo.type() == type() && compareTo.symbol() == symbol();
}

/// \brief Orders this item relative to another item
bool item::operator<(const item& compareTo) const {
    return compareTo.type() < type() || compareTo.symbol() < symbol();
}

/// \brief Comparison function, returns true if a is less than b
bool item::compare(const item& a, const item& b) {
    // Compare the kinds
    kind aKind = a.type();
    kind bKind = b.type();
    
    if (aKind < bKind) return true;
    if (aKind > bKind) return false;
    
    // For objects with an 'other' type, use RTTI to compare the underlying types
    if (aKind >= other) {
        // Compare the types of these items
        bool isBefore = typeid(a).before(typeid(b));
        
        // a is less than b if its type is before b
        if (isBefore) return true;
        
        // a is greater than b if the types aren't equal
        if (typeid(a) != typeid(b)) return false;
        
        // Call through to the items own less-than operator
        return a < b;
    }
    
    // For EBNF items, use the comparison operator built into the item
    if (aKind >= ebnf) {
        return a < b;
    }
    
    // For well-known kinds, we can just compare the symbols
    int aSym = a.symbol();
    int bSym = b.symbol();
    
    return aSym < bSym;
}
