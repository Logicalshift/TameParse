//
//  ebnf_item.cpp
//  Parse
//
//  Created by Andrew Hunter on 24/07/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#include "TameParse/Language/ebnf_item.h"

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
    for (ebnf_item_list::iterator toDelete = m_ChildItems.begin(); toDelete != m_ChildItems.end(); ++toDelete) {
        delete *toDelete;
    }
    m_ChildItems.clear();
}

/// \brief Fills the content of this item with the content of the specified item
ebnf_item& ebnf_item::operator=(const ebnf_item& copyFrom) {
    // Can't assign to ourselves
    if (&copyFrom == this) return *this;
    
    // Clear out the child items
    for (ebnf_item_list::iterator toDelete = m_ChildItems.begin(); toDelete != m_ChildItems.end(); ++toDelete) {
        delete *toDelete;
    }
    m_ChildItems.clear();
    
    // Copy the items from the source
    m_Identifier        = copyFrom.m_Identifier;
    m_SourceIdentifier  = copyFrom.m_SourceIdentifier;
    
    for (iterator toCopy = copyFrom.begin(); toCopy != copyFrom.end(); ++toCopy) {
        m_ChildItems.push_back(new ebnf_item(**toCopy));
    }
    
    set_start_pos(copyFrom.start_pos());
    set_end_pos(copyFrom.end_pos());
    
    return *this;
}

/// \brief Adds a child item to this item
///
/// This item will delete the item when it has finished with it.
void ebnf_item::add_child(ebnf_item* newChild) {
    // Update the position of this item
    if (newChild->start_pos() != position() || newChild->end_pos() != position()) {
        // If this item has a null position, then just set the position
        if (start_pos() == position() && end_pos() == position()) {
            set_start_pos(newChild->start_pos());
            set_end_pos(newChild->end_pos());
        } else {
            // If the start position is less than the current start position, then update it
            if (newChild->start_pos() < start_pos()) {
                set_start_pos(newChild->start_pos());
            }
            
            // If the end position if greater than the current end position, then update it
            if (newChild->end_pos() > end_pos()) {
                set_end_pos(end_pos());
            }
        }
    }
    
    // Add this as a child item
    m_ChildItems.push_back(newChild);
}
