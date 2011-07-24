//
//  ebnf_item.h
//  Parse
//
//  Created by Andrew Hunter on 24/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LANGUAGE_EBNF_ITEM_H
#define _LANGUAGE_EBNF_ITEM_H

#include <vector>
#include <string>

#include "Language/block.h"

namespace language {
    ///
    /// \brief Representation of a single EBNF item in a production
    ///
    class ebnf_item : public block {
    public:
        /// \brief The possible types of EBNF item
        enum type {
            /// \brief A nonterminal symbol
            ebnf_nonterminal,
            
            /// \brief A terminal symbol
            ebnf_terminal,
            
            /// \brief A terminal represented as a string constant
            ebnf_terminal_string,
            
            /// \brief A terminal represented as a character constant
            ebnf_terminal_character,
            
            /// \brief A guard item
            ebnf_guard,
            
            /// \brief Repeat 0 or more times (Kleene star)
            ebnf_repeat_zero,
            
            /// \brief Repeat 1 or more times
            ebnf_repeat_one,
            
            /// \brief Optional item
            ebnf_optional,
            
            /// \brief A parenthesized item
            ebnf_parenthesized
        };
        
        /// \brief A list of EBNF items
        typedef std::vector<ebnf_item*> ebnf_item_list;
        
    private:
        /// \brief The type of this item
        type m_Type;
        
        /// \brief The empty string, or the identifier of the source language for a terminal or nonterminal symbol
        std::wstring m_SourceIdentifier;
        
        /// \brief The identifier of this item (if it's a simple nonterminal or terminal)
        ///
        /// For a string or a character, this will be the (unquoted) string or character that should be matched at this point.
        /// For guards and the various EBNF items, this will be empty
        std::wstring m_Identifier;
        
        /// \brief For guard and EBNF items, these are the items that they are made up of
        ///
        /// Note that this should form a tree and not a graph
        ebnf_item_list m_ChildItems;
        
    public:
        
    };
}

#endif
