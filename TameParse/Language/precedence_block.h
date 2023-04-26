//
//  precedence_block.h
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

#ifndef _LANGUAGE_PRECEDENCE_BLOCK_H
#define _LANGUAGE_PRECEDENCE_BLOCK_H

#include <vector>

#include "TameParse/Language/block.h"
#include "TameParse/Language/ebnf_item.h"

namespace yy_language {
    ///
    /// \brief Block that defines the precedence of the operators in a language
    ///
    class precedence_block : public block {
    public:
        ///
        /// \brief Associativity specified for an EBNF item
        ///
        enum associativity {
            left,
            right,
            nonassoc
        };
        
        /// \brief List of EBNF items (we only define terminal items for precedence)
        typedef std::vector<ebnf_item*> ebnf_item_list;
        
        ///
        /// \brief Precedence item
        ///
        struct item {
            /// \brief The associativity for this item
            associativity assoc;
            
            /// \brief The items that have this precedence
            ebnf_item_list items;
        };
        
        /// \brief List ordering items by precedence
        typedef std::vector<item> list;
        
        /// \brief Iterator for retrieving the items 
        typedef list::const_iterator iterator;
        
    private:
        /// \brief List ordering items by precedence
        list m_List;
        
    public:
        /// \brief Creates a new precedence definition
        precedence_block(position& start, position& end);
        
        /// \brief Destructor
        virtual ~precedence_block();
        
        /// \brief Adds a new item to this precedence definition
        void add_item(item& newItem);
        
        /// \brief The item with the highest precedence
        iterator begin() const;
        
        /// \brief The item after the item with the lowest precedence
        iterator end() const;
    };
}

#endif
