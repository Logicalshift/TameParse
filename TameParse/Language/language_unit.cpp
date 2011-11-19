//
//  language_unit.cpp
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/common.h"

#include <cstdlib>
#include "TameParse/Language/language_unit.h"

using namespace language;

/// \brief Defines this as a lexical lanuguage unit
language_unit::language_unit(unit_type type, lexer_block* lexer)
: m_Grammar(NULL)
, m_LexerBlock(lexer)
, m_Type(type) {
    if (lexer) {
        set_start_pos(lexer->start_pos());
        set_end_pos(lexer->end_pos());
    }
}

/// \brief Defines this as a language unit with a grammar
language_unit::language_unit(grammar_block* grammar)
: m_Grammar(grammar)
, m_LexerBlock(NULL)
, m_Type(unit_grammar_definition) {
    if (grammar) {
        set_start_pos(grammar->start_pos());
        set_end_pos(grammar->end_pos());
    }
}

/// \brief Copies a language unit
language_unit::language_unit(const language_unit& copyFrom)
: m_Grammar(NULL)
, m_LexerBlock(NULL) {
    (*this) = copyFrom;
}

/// \brief Copies the contents of another language unit into this one
language_unit& language_unit::operator=(const language_unit& copyFrom) {
    // Don't copy ourselves
    if (&copyFrom == this) return *this;
    
    // Destroy the existing content
    if (m_Grammar)      delete m_Grammar;
    if (m_LexerBlock)   delete m_LexerBlock;
    
    m_Grammar       = NULL;
    m_LexerBlock    = NULL;
    
    // Copy the content
    m_Type = copyFrom.m_Type;
    
    if (copyFrom.m_Grammar)     m_Grammar       = new grammar_block(*copyFrom.m_Grammar);
    if (copyFrom.m_LexerBlock)  m_LexerBlock    = new lexer_block(*copyFrom.m_LexerBlock);
    
    set_start_pos(copyFrom.start_pos());
    set_end_pos(copyFrom.end_pos());
    
    // Return this object
    return *this;
}

/// \brief Destructor
language_unit::~language_unit() {
    if (m_Grammar)      delete m_Grammar;
    if (m_LexerBlock)   delete m_LexerBlock;
    
    m_Grammar       = NULL;
    m_LexerBlock    = NULL;
}
