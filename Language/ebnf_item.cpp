//
//  ebnf_item.cpp
//  Parse
//
//  Created by Andrew Hunter on 24/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "ebnf_item.h"

using namespace std;
using namespace language;

/// \brief Creates an EBNF item (sourceIdentifier.identifier)
ebnf_item::ebnf_item(type typ, const std::wstring& sourceIdentifier, const std::wstring& identifier, position start, position end)
: m_Type(typ)
, m_SourceIdentifier(sourceIdentifier)
, m_Identifier(identifier)
, block(start, end) {
}

/// \brief Creates an EBNF item which doesn't specify a symbol
ebnf_item::ebnf_item(type typ, position start, position end)
: m_Type(typ)
, block(start, end) {
}

/// \brief Creates an EBNF item by copying an existing one
ebnf_item::ebnf_item(const ebnf_item& copyFrom) {
    (*this) = copyFrom;
}

/// \brief Destructor
ebnf_item::~ebnf_item() {
    // Clear out the child items
}

/// \brief Fills the content of this item with the content of the specified item
ebnf_item& ebnf_item::operator=(const ebnf_item& copyFrom) {
    // Clear out the child items
}

/// \brief Adds a child item to this item
///
/// This item will delete the item when it has finished with it.
void add_child(ebnf_item* newChild);
