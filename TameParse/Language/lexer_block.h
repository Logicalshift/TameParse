//
//  lexer_block.h
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

#ifndef _LANGUAGE_LEXER_BLOCK_H
#define _LANGUAGE_LEXER_BLOCK_H

#include <vector>

#include "TameParse/Language/block.h"
#include "TameParse/Language/lexeme_definition.h"

namespace yy_language {
    ///
    /// \brief Class representing the definition of a block that defines lexical symbols
    ///
    /// A lexical symbol consists of an identifier and a definition which can be a string, a character, a regular expression
    /// or a reference to an existing lexical symbol in another language.
    ///
    /// In keyword blocks, the definition can additionally be empty (indicating that the lexical symbol is identical to 
    /// the identifier used for the keyword)
    ///
    class lexer_block : public block {
    public:
        /// \brief List of lexeme definitions
        typedef std::vector<lexeme_definition*> lexeme_list;
        
        /// \brief Iterator for this block
        typedef lexeme_list::const_iterator iterator;
        
    private:
        /// \brief The lexemes that maek up this block
        lexeme_list m_Lexemes;

        /// \brief True if the symbols defined by this block should be treated as 'weak'
        bool m_Weak;

        /// \brief True if the symbols defined by this block should be treated as 'case insensitive'
        bool m_CaseInsensitive;

        /// \brief True if the symbols defined by this block should be explicitly treated as 'case sensitive'
        ///
        /// Lexer blocks are case-sensitive by default, but may be re-used in other
        /// blocks if they are specified as a lexer-symbols block, in which case this
        /// controls whether or not we should inherit the case sensivity of the
        /// parent item.
        bool m_CaseSensitive;
        
    public:
        /// \brief Creates a new lexer block
        lexer_block(bool weak, bool caseInsensitive, bool caseSensitive, position start = position(), position end = position());
        
        /// \brief Creates a lexer block by copying an old one
        lexer_block(const lexer_block& copyFrom);
        
        /// \brief Destructor
        virtual ~lexer_block();
        
        /// \brief Copies the content of a lexer block into this one
        lexer_block& operator=(const lexer_block& copyFrom);
        
        /// \brief Adds a lexeme definition to this object (which becomes responsible for deleting it)
        void add_definition(lexeme_definition* newDefinition);
        
        /// \brief Returns the first definition in this object
        inline iterator begin() const { return m_Lexemes.begin(); }
        
        /// \brief Returns the definition after the last definition in this object
        inline iterator end() const { return m_Lexemes.end(); }

        /// \brief Returns true if the symbols in this block should be treated as 'weak'
        inline bool is_weak() const { return m_Weak; }

        /// \brief Returns true if the symbols in this block should be treated as case insensitive
        inline bool is_case_insensitive() const { return m_CaseInsensitive; }

        /// \brief True if the symbols defined by this block should be explicitly treated as 'case sensitive'
        ///
        /// Lexer blocks are case-sensitive by default, but may be re-used in other
        /// blocks if they are specified as a lexer-symbols block, in which case this
        /// controls whether or not we should inherit or override the case sensivity 
        /// of the parent item.
        inline bool is_case_sensitive() const { return m_CaseSensitive; }
    };
}

#endif
