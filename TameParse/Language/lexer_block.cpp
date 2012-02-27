//
//  lexer_block.cpp
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

#include "TameParse/Language/lexer_block.h"

using namespace language;

/// \brief Creates a new lexer block
lexer_block::lexer_block(bool weak, bool caseInsensitive, bool caseSensitive, position start, position end)
: block(start, end)
, m_Weak(weak)
, m_CaseSensitive(caseSensitive)
, m_CaseInsensitive(caseInsensitive) {
}

/// \brief Creates a lexer block by copying an old one
lexer_block::lexer_block(const lexer_block& copyFrom) {
    (*this) = copyFrom;
}

/// \brief Destructor
lexer_block::~lexer_block() {
    // Delete all of the existing items
    for (lexeme_list::iterator toDelete = m_Lexemes.begin(); toDelete != m_Lexemes.end(); ++toDelete) {
        delete *toDelete;
    }
    m_Lexemes.clear();
}

/// \brief Copies the content of a lexer block into this one
lexer_block& lexer_block::operator=(const lexer_block& copyFrom) {
    // Can't copy ourselves
    if (&copyFrom == this) return *this;
    
    // Delete all of the existing items
    for (lexeme_list::iterator toDelete = m_Lexemes.begin(); toDelete != m_Lexemes.end(); ++toDelete) {
        delete *toDelete;
    }
    m_Lexemes.clear();
    
    // Copy the items from the source
    for (iterator toCopy = copyFrom.begin(); toCopy != copyFrom.end(); ++toCopy) {
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
