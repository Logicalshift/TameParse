//
//  production_definition.h
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

#ifndef _LANGUAGE_PRODUCTION_DEFINITION_H
#define _LANGUAGE_PRODUCTION_DEFINITION_H

#include <vector>

#include "TameParse/Language/block.h"
#include "TameParse/Language/ebnf_item.h"

namespace yy_language {
    ///
    /// \brief Class representing a production associated with a nonterminal
    ///
    class production_definition : public block {
    public:
        /// \brief List of EBNF items
        typedef std::vector<ebnf_item*> ebnf_item_list;
        
        /// \brief Iterator for accessing the values in this production definition
        typedef ebnf_item_list::const_iterator iterator;
        
    private:
        /// \brief The EBNF items that make up this production
        ebnf_item_list m_Items;
        
    public:
        /// \brief Creates a new production definition
        production_definition(position start = position(), position end = position());
        
        /// \brief Creates a production definition by copying an existing definition
        production_definition(const production_definition& copyFrom);
        
        /// \brief Destructor
        virtual ~production_definition();
        
        /// \brief Assigns the value of this object to that of another
        production_definition& operator=(const production_definition& copyFrom);
        
        /// \brief Adds an EBNF item to this production definition
        ///
        /// This object will become responsible for destroying the item that is passed in
        void add_item(ebnf_item* newItem);
        
        /// \brief The first EBNF item in this production
        inline iterator begin() const { return m_Items.begin(); }
        
        /// \brief The item after the last EBNF item in this production
        inline iterator end() const { return m_Items.end(); }
    };
}

#endif
