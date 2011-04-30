//
//  ebnf_items.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "ebnf_items.h"
#include "symbol_set.h"

using namespace dfa;
using namespace contextfree;

/// \brief Creates a new EBNF item
ebnf::ebnf()
: m_Rule(new rule(this))
, item(symbol_set::null) {
}

/// \brief Creates a new EBNF item from a rule
ebnf::ebnf(const rule& copyFrom) 
: item(symbol_set::null)
, m_Rule(new rule(copyFrom, this)) {
}

/// \brief Compares this item to another. Returns true if they are the same
bool ebnf::operator==(const item& compareTo) const {
    const ebnf* compareEbnf = dynamic_cast<const ebnf*>(&compareTo);
    if (!compareEbnf) return false;
    
    return (*m_Rule) == compareEbnf->get_rule();
}

/// \brief Orders this item relative to another item
bool ebnf::operator<(const item& compareTo) const {
    return true;
}
