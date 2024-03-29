//
//  ebnf_item.h
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

#ifndef _LANGUAGE_EBNF_ITEM_H
#define _LANGUAGE_EBNF_ITEM_H

#include <vector>
#include <string>

#include "TameParse/Language/block.h"
#include "TameParse/Language/ebnf_item_attributes.h"

namespace yy_language {
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
            
            /// \brief An alternative between 2 different items
            ebnf_alternative,
            
            /// \brief Repeat 0 or more times (Kleene star)
            ebnf_repeat_zero,
            
            /// \brief Repeat 1 or more times
            ebnf_repeat_one,
            
            /// \brief Optional item
            ebnf_optional,
            
            /// \brief A parenthesized item
            ///
            /// This is used to group items together into a single item. It does not necessarily represent where the user
            /// added parentheses. In particular, the precendence of alternatives means that they typically generate
            /// extra parenthesized items. That is, the sequence (a b | c d) should be represented as ((a b) | (c d)).
            /// As the ebnf_alternative item can only have two children, this must be done by generating new parenthesized
            /// items.
            ebnf_parenthesized
        };
        
        /// \brief A list of EBNF items
        typedef std::vector<ebnf_item*> ebnf_item_list;
        
        /// \brief Child item iterator
        typedef ebnf_item_list::const_iterator iterator;
        
    private:
        /// \brief The type of this item
        type m_Type;
        
        /// \brief The empty string, or the identifier of the source language for a terminal or nonterminal symbol
        std::wstring m_SourceIdentifier;
        
        /// \brief The identifier of this item (if it's a simple nonterminal or terminal)
        ///
        /// For a string or a character, this will be the (quoted) string or character that should be matched at this point.
        /// For guards and the various EBNF items, this will be empty
        std::wstring m_Identifier;

        /// \brief The name assigned to this item, or the empty string if it has no name
        ebnf_item_attributes m_Attributes;
        
        /// \brief For guard and EBNF items, these are the items that they are made up of
        ///
        /// Note that this should form a tree and not a graph
        ebnf_item_list m_ChildItems;
        
    public:
        /// \brief Creates an EBNF item (sourceIdentifier.identifier)
        ebnf_item(type typ, const std::wstring& sourceIdentifier, const std::wstring& identifier, const ebnf_item_attributes& attributes, position start, position end);

        /// \brief Creates an EBNF item which doesn't specify a symbol
        ebnf_item(type typ, const ebnf_item_attributes& attributes, position start = position(), position end = position());
        
        /// \brief Creates an EBNF item by copying an existing one
        ebnf_item(const ebnf_item& copyFrom);
        
        /// \brief Destructor
        virtual ~ebnf_item();
        
        /// \brief Fills the content of this item with the content of the specified item
        ebnf_item& operator=(const ebnf_item& copyFrom);
        
        /// \brief Returns the type of this item
        inline type get_type() { return m_Type; }
        
        /// \brief Adds a child item to this item
        ///
        /// This item will delete the item when it has finished with it.
        void add_child(ebnf_item* newChild);
        
        /// \brief The identifier of the source language, or the empty string for items from this language
        inline const std::wstring& source_identifier() { return m_SourceIdentifier; }
        
        /// \brief The identifier for this item (if it is a simple terminal or nonterminal item)
        ///
        /// For a string or a character, this will be the (quoted) string or character that should be matched at this point.
        /// For guards and the various EBNF items, this will be empty
        inline const std::wstring& identifier() const { return m_Identifier; }

        /// \brief The name for this item, or the empty string if it is not named
        inline const ebnf_item_attributes& attributes() const { return m_Attributes; }
        
        /// \brief The first child item for this item
        inline iterator begin() const { return m_ChildItems.begin(); }
        
        /// \brief The item immediately after the final child item
        inline iterator end() const { return m_ChildItems.end(); }

        /// \brief The number of child items for this item
        inline size_t size() const { return m_ChildItems.size(); }
        
        /// \brief Returns the child item at the specified index
        inline ebnf_item* operator[](size_t index) { return m_ChildItems[index]; }
    };
}

#endif
