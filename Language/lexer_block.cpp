//
//  lexer_block.cpp
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "lexer_block.h"

using namespace language;

/// \brief Creates a new lexer block
lexer_block::lexer_block(position start, position end)
: block(start, end) {
}

/// \brief Creates a lexer block by copying an old one
lexer_block::lexer_block(const lexer_block& copyFrom) {
    (*this) = copyFrom;
}

/// \brief Destructor
lexer_block::~lexer_block() {
    // Delete all of the existing items
    for (lexeme_list::iterator toDelete = m_Lexemes.begin(); toDelete != m_Lexemes.end(); toDelete++) {
        delete *toDelete;
    }
    m_Lexemes.clear();
}

/// \brief Copies the content of a lexer block into this one
lexer_block& lexer_block::operator=(const lexer_block& copyFrom) {
    // Can't copy ourselves
    if (&copyFrom == this) return *this;
    
    // Delete all of the existing items
    for (lexeme_list::iterator toDelete = m_Lexemes.begin(); toDelete != m_Lexemes.end(); toDelete++) {
        delete *toDelete;
    }
    m_Lexemes.clear();
    
    // Copy the items from the source
    for (iterator toCopy = copyFrom.begin(); toCopy != copyFrom.end(); toCopy++) {
        m_Lexemes.push_back(new lexeme_definition(**toCopy));
    }
    
    set_start_pos(copyFrom.start_pos());
    set_end_pos(copyFrom.end_pos());
    
    return *this;
}

/// \brief Adds a lexeme definition to this object (which becomes responsible for deleting it)
void lexer_block::add_definition(lexeme_definition* newDefinition) {
    m_Lexemes.push_back(newDefinition);
}
