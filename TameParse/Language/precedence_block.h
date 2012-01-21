//
//  precedence_block.h
//  TameParse
//
//  Created by Andrew Hunter on 21/01/2012.
//  Copyright (c) 2012 Andrew Hunter. All rights reserved.
//

#ifndef _LANGUAGE_PRECEDENCE_BLOCK_H
#define _LANGUAGE_PRECEDENCE_BLOCK_H

#include <vector>

#include "TameParse/Language/block.h"
#include "TameParse/Language/ebnf_item.h"

namespace language {
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
