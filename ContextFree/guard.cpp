//
//  guard.cpp
//  Parse
//
//  Created by Andrew Hunter on 29/06/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "guard.h"

using namespace contextfree;

/// \brief Creates a guard item that matches the rule with the specified identifier
guard::guard(int sym)
: item(sym) {
}

/// \brief Creates a clone of this item
item* guard::clone() const {
    return new guard(symbol());
}

/// \brief The type of this item
item::kind guard::type() const {
    return item::guard;
}

/// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
///
/// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
/// set for the rules that make them up.
///
/// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
/// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
item_set guard::first(const grammar& gram) const {
    // Just this item
    item_set result;
    result.insert(this);
    return result;
}
