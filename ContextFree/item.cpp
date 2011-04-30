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
