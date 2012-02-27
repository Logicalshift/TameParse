//
//  grammar_block.h
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
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

#ifndef _LANGUAGE_GRAMMAR_BLOCK_H
#define _LANGUAGE_GRAMMAR_BLOCK_H

#include <vector>

#include "TameParse/Language/block.h"
#include "TameParse/Language/nonterminal_definition.h"

namespace language {
    ///
    /// \brief Representation of a block that defines the grammar for a language
    ///
    class grammar_block : public block {
    public:
        /// \brief List of nonterminal definitions (which is essentially what a grammar definition is)
        typedef std::vector<nonterminal_definition*> nonterminal_definition_list;
        
        /// \brief Iterator for the nonterminals in this object
        typedef nonterminal_definition_list::const_iterator iterator;
        
    private:
        /// \brief The nonterminals that make up this grammar
        nonterminal_definition_list m_Nonterminals;
        
    public:
        /// \brief Creates a new grammar block
        grammar_block(position start = position(), position end = position());
        
        /// \brief Creates a grammar block by copying an existing block
        grammar_block(grammar_block& copyFrom);
        
        /// \brief Destructor
        virtual ~grammar_block();
        
        /// \brief Assigns the value of this grammar block
        grammar_block& operator=(const grammar_block& copyFrom);
        
        /// \brief Adds a new nonterminal to this object, which will become responsible for freeing it
        void add_nonterminal(nonterminal_definition* newNonterminal);
        
        /// \brief The first nonterminal in this block
        inline iterator begin() const { return m_Nonterminals.begin(); }
        
        /// \brief The nonterminal after the last nonterminal in this block
        inline iterator end() const { return m_Nonterminals.end(); }
    };
}

#endif
