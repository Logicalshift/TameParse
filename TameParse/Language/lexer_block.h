//
//  lexer_block.h
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LANGUAGE_LEXER_BLOCK_H
#define _LANGUAGE_LEXER_BLOCK_H

#include <vector>

#include "TameParse/Language/block.h"
#include "TameParse/Language/lexeme_definition.h"

namespace language {
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
        
    public:
        /// \brief Creates a new lexer block
        lexer_block(position start = position(), position end = position());
        
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
    };
}

#endif
