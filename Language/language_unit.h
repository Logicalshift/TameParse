//
//  language_unit.h
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LANGUAGE_LANGUAGE_UNIT_H
#define _LANGUAGE_LANGUAGE_UNIT_H

#include "Language/grammar_block.h"
#include "Language/lexer_block.h"

namespace language {
    /// \brief Class that represents an item in a language definition
    class language_unit {
    public:
        /// \brief An enumeration of the types of language unit
        enum unit_type {
            /// \brief A lexer symbols block (symbols that can be re-used when defining the lexer, but which don't actually produce terminal symbols)
            unit_lexer_symbols,
            
            /// \brief The primary lexer definition
            unit_lexer_definition,
            
            /// \brief The ignore symbols definition
            unit_ignore_definition,
            
            /// \brief The weak symbols definition
            unit_weak_symbols_definition,
            
            /// \brief The keyword symbols definition
            unit_keywords_definition,
            
            /// \brief A grammar definition
            unit_grammar_definition,
        };
        
    private:
        /// \brief The type of this language unit
        unit_type m_Type;
        
        /// \brief If this is one of the block types that defines lexical symbols, this will be the lexer block that it represents
        lexer_block* m_LexerBlock;
        
        /// \brief If this is a grammar block, this will be non-NULL
        grammar_block* m_Grammar;
        
    public:
        /// \brief Defines this as a lexical lanuguage unit
        language_unit(unit_type type, lexer_block* lexer);
        
        /// \brief Defines this as a language unit with a grammar
        language_unit(grammar_block* grammar);
        
        /// \brief Copies a language unit
        language_unit(const language_unit& copyFrom);
        
        /// \brief Copies the contents of another language unit into this one
        language_unit& operator=(const language_unit& copyFrom);
        
        /// \brief Destructor
        virtual ~language_unit();
        
        /// \brief The type of this language unit
        inline unit_type type() const { return m_Type; }
        
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
        
        /// \brief If this is a weak symbols block, this will return the lexer block that defines its content (otherwise null)
        inline lexer_block* weak_symbols_definition() const {
            if (m_Type == unit_weak_symbols_definition)
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
    };
}

#endif
