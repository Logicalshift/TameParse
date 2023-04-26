//
//  precedence_block.cpp
//  TameParse
//
//  Created by Andrew Hunter on 21/01/2012.
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

#include "TameParse/Language/precedence_block.h"

using namespace std;
using namespace yy_language;

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
