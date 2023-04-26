//
//  language_unit.cpp
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

#include <cstdlib>
#include "TameParse/Language/language_unit.h"

using namespace yy_language;

/// \brief Defines this as a lexical lanuguage unit
language_unit::language_unit(unit_type type, lexer_block* lexer)
: m_Grammar(NULL)
, m_LexerBlock(lexer)
, m_Precedence(NULL)
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
, m_Precedence(NULL)
, m_Type(unit_grammar_definition) {
    if (grammar) {
        set_start_pos(grammar->start_pos());
        set_end_pos(grammar->end_pos());
    }
}

/// \brief Defines this as a language unit with a precedence block
language_unit::language_unit(precedence_block* precedence)
: m_Grammar(NULL)
, m_LexerBlock(NULL)
, m_Precedence(precedence)
, m_Type(unit_precedence_definition) {
}

/// \brief Copies a language unit
language_unit::language_unit(const language_unit& copyFrom)
: m_Grammar(NULL)
, m_LexerBlock(NULL)
, m_Precedence(NULL) {
    (*this) = copyFrom;
}

/// \brief Copies the contents of another language unit into this one
language_unit& language_unit::operator=(const language_unit& copyFrom) {
    // Don't copy ourselves
    if (&copyFrom == this) return *this;
    
    // Destroy the existing content
    if (m_Grammar)      delete m_Grammar;
    if (m_LexerBlock)   delete m_LexerBlock;
    if (m_Precedence)   delete m_Precedence;
    
    m_Grammar       = NULL;
    m_LexerBlock    = NULL;
    m_Precedence    = NULL;
    
    // Copy the content
    m_Type = copyFrom.m_Type;
    
    if (copyFrom.m_Grammar)     m_Grammar       = new grammar_block(*copyFrom.m_Grammar);
    if (copyFrom.m_LexerBlock)  m_LexerBlock    = new lexer_block(*copyFrom.m_LexerBlock);
    if (copyFrom.m_Precedence)  m_Precedence    = new precedence_block(*copyFrom.m_Precedence);
    
    set_start_pos(copyFrom.start_pos());
    set_end_pos(copyFrom.end_pos());
    
    // Return this object
    return *this;
}

/// \brief Destructor
language_unit::~language_unit() {
    if (m_Grammar)      delete m_Grammar;
    if (m_LexerBlock)   delete m_LexerBlock;
    if (m_Precedence)   delete m_Precedence;
    
    m_Grammar       = NULL;
    m_LexerBlock    = NULL;
    m_Precedence    = NULL;
}
