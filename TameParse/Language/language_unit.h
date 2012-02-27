//
//  language_unit.h
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

#ifndef _LANGUAGE_LANGUAGE_UNIT_H
#define _LANGUAGE_LANGUAGE_UNIT_H

#include "TameParse/Language/block.h"
#include "TameParse/Language/grammar_block.h"
#include "TameParse/Language/lexer_block.h"
#include "TameParse/Language/precedence_block.h"

namespace language {
    /// \brief Class that represents an item in a language definition
    class language_unit : public block {
    public:
        /// \brief An enumeration of the types of language unit
        enum unit_type {
            /// \brief Not a language unit
            unit_null,
            
            /// \brief A lexer symbols block (symbols that can be re-used when defining the lexer, but which don't actually produce terminal symbols)
            unit_lexer_symbols,
            
            /// \brief The keyword symbols definition
            unit_keywords_definition,
            
            /// \brief The primary lexer definition
            unit_lexer_definition,
            
            /// \brief The ignore symbols definition
            unit_ignore_definition,
            
            /// \brief A grammar definition
            unit_grammar_definition,

            /// \brief A precedence definition
            unit_precedence_definition
        };
        
    private:
        /// \brief The type of this language unit
        unit_type m_Type;
        
        /// \brief If this is one of the block types that defines lexical symbols, this will be the lexer block that it represents
        lexer_block* m_LexerBlock;
        
        /// \brief If this is a grammar block, this will be non-NULL
        grammar_block* m_Grammar;

        /// \brief If this is a precedence block, this will be non-NULL
        precedence_block* m_Precedence;

    public:
        /// \brief Defines this as a lexical lanuguage unit
        language_unit(unit_type type, lexer_block* lexer);
        
        /// \brief Defines this as a language unit with a grammar
        language_unit(grammar_block* grammar);

        /// \brief Defines this as a language unit with a precedence block
        language_unit(precedence_block* precedence);
        
        /// \brief Copies a language unit
        language_unit(const language_unit& copyFrom);
        
        /// \brief Copies the contents of another language unit into this one
        language_unit& operator=(const language_unit& copyFrom);
        
        /// \brief Destructor
        virtual ~language_unit();
        
        /// \brief The type of this language unit
        inline unit_type type() const { return m_Type; }
        
        /// \brief If this is any kind of lexer block, this will return it (use the type() call to get the type of lexer block
        /// that this is)
        inline lexer_block* any_lexer_block() const {
            return m_LexerBlock;
        }
        
        /// \brief If this is a lexer symbols block, this will return the lexer block that defines its content (otherwise null)
        inline lexer_block* lexer_symbols() const {
            if (m_Type == unit_lexer_symbols)
                return m_LexerBlock;
            else
                return NULL;
        }
        
        /// \brief If this is a lexer definition block, this will return the lexer block that defines its content (otherwise null)
        inline lexer_block* lexer_definition() const {
            if (m_Type == unit_lexer_definition)
                return m_LexerBlock;
            else
                return NULL;
        }
        
        /// \brief If this is a ignored symbols definition block, this will return the lexer block that defines its content (otherwise null)
        inline lexer_block* ignore_definition() const {
            if (m_Type == unit_ignore_definition)
                return m_LexerBlock;
            else
                return NULL;
        }
        
        /// \brief If this is a keywords definition block, this will return the lexer block that defines its content (otherwise null)
        inline lexer_block* keywords_definition() const {
            if (m_Type == unit_keywords_definition)
                return m_LexerBlock;
            else
                return NULL;
        }
        
        /// \brief If this is a grammar definition block, this will return the grammar block that defines its content
        inline grammar_block* grammar_definition() const {
            return m_Grammar;
        }

        /// \brief If this is a precedence definition block, this will return the precedence of the items
        inline precedence_block* precedence_definition() const {
            return m_Precedence;
        }
    };
}

#endif
