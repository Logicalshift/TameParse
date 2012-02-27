//
//  production_definition.cpp
//  Parse
//
//  Created by Andrew Hunter on 24/07/2011.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the \"Software\"), to 
//  deal in the Software without restriction, including without limitation the 
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
//  sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
//  IN THE SOFTWARE.
//

#include "TameParse/Language/production_definition.h"

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
    for (ebnf_item_list::iterator toDelete = m_Items.begin(); toDelete != m_Items.end(); ++toDelete) {
        delete *toDelete;
    }
    m_Items.clear();
}

/// \brief Assigns the value of this object to that of another
production_definition& production_definition::operator=(const production_definition& copyFrom) {
    // Can't assign ourselves
    if (&copyFrom == this) return *this;

    // Destroy the existing items
    for (ebnf_item_list::iterator toDelete = m_Items.begin(); toDelete != m_Items.end(); ++toDelete) {
        delete *toDelete;
    }
    m_Items.clear();
    
    // Copy from the target
    for (iterator toCopy = copyFrom.begin(); toCopy != copyFrom.end(); ++toCopy) {
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
    
    if (start_pos() == end_pos()) {
        set_start_pos(newItem->start_pos());
        set_end_pos(newItem->end_pos());
    }
    
    if (newItem->start_pos() < start_pos()) {
        set_start_pos(newItem->end_pos());
    }
    
    if (newItem->end_pos() > end_pos()) {
        set_end_pos(newItem->end_pos());
    }
}
