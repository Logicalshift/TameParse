//
//  precedence_block.cpp
//  TameParse
//
//  Created by Andrew Hunter on 21/01/2012.
//  Copyright (c) 2012 Andrew Hunter. All rights reserved.
//

#include "TameParse/Language/precedence_block.h"

using namespace std;
using namespace language;

/// \brief Creates a new precedence definition
precedence_block::precedence_block(position& start, position& end)
: block(start, end) {
}

/// \brief Destructor
precedence_block::~precedence_block() {
    // Free up the items
    for (list::iterator item = m_List.begin(); item != m_List.end(); ++item) {
        for (ebnf_item_list::iterator ebnfItem = item->items.begin(); ebnfItem != item->items.end(); ++ebnfItem) {
            delete *ebnfItem;
        }
    }

    // Clear out the list
    m_List.clear();
}

/// \brief Adds a new item to this precedence definition
void precedence_block::add_item(item& newItem) {
    // Add this item
    m_List.push_back(newItem);
}

/// \brief The item with the highest precedence
precedence_block::iterator precedence_block::begin() const {
    return m_List.begin();
}

/// \brief The item after the item with the lowest precedence
precedence_block::iterator precedence_block::end() const {
    return m_List.end();
}
