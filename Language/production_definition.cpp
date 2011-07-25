//
//  production_definition.cpp
//  Parse
//
//  Created by Andrew Hunter on 24/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "production_definition.h"

using namespace language;

/// \brief Creates a new production definition
production_definition::production_definition(position start, position end)
: block(start, end) {
}

/// \brief Creates a production definition by copying an existing definition
production_definition::production_definition(const production_definition& copyFrom) {
    (*this) = copyFrom;
}

/// \brief Destructor
production_definition::~production_definition() {
    // Destroy the items
    for (ebnf_item_list::iterator toDelete = m_Items.begin(); toDelete != m_Items.end(); toDelete++) {
        delete *toDelete;
    }
    m_Items.clear();
}

/// \brief Assigns the value of this object to that of another
production_definition& production_definition::operator=(const production_definition& copyFrom) {
    // Can't assign ourselves
    if (&copyFrom == this) return *this;

    // Destroy the existing items
    for (ebnf_item_list::iterator toDelete = m_Items.begin(); toDelete != m_Items.end(); toDelete++) {
        delete *toDelete;
    }
    m_Items.clear();
    
    // Copy from the target
    for (iterator toCopy = copyFrom.begin(); toCopy != copyFrom.end(); toCopy++) {
        m_Items.push_back(new ebnf_item(**toCopy));
    }
    
    set_start_pos(copyFrom.start_pos());
    set_end_pos(copyFrom.end_pos());
    
    return *this;
}

/// \brief Adds an EBNF item to this production definition
///
/// This object will become responsible for destroying the item that is passed in
void production_definition::add_item(ebnf_item* newItem) {
    m_Items.push_back(newItem);
}
