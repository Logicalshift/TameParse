//
//  ebnf_item_attributes.cpp
//  Parse
//
//  Created by Andrew Hunter on 06/01/2012.
//  Copyright 2012 Andrew Hunter. All rights reserved.
//

#include "TameParse/Language/ebnf_item_attributes.h"

using namespace language;

/// \brief Creates an empty attributes object
ebnf_item_attributes::ebnf_item_attributes()
: name()
, conflict_action(conflict_notspecified)
, guard_can_clash(false) {
}

/// \brief Creates an attribute for an object with just a name
ebnf_item_attributes::ebnf_item_attributes(std::wstring& newName)
: name(newName)
, conflict_action(conflict_notspecified)
, guard_can_clash(false) {
}
