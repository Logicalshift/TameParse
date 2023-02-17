//
//  grammar_block.cpp
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

#include "TameParse/Language/grammar_block.h"

using namespace yy_language;

/// \brief Creates a new grammar block
grammar_block::grammar_block(position start, position end)
: block(start, end) {
}

/// \brief Creates a grammar block by copying an existing block
grammar_block::grammar_block(grammar_block& copyFrom) {
    (*this) = copyFrom;
}

/// \brief Destructor
grammar_block::~grammar_block() {
    // Delete the nonterminals
    for (nonterminal_definition_list::iterator toDelete = m_Nonterminals.begin(); toDelete != m_Nonterminals.end(); ++toDelete) {
        delete *toDelete;
    }
    m_Nonterminals.clear();
}

/// \brief Assigns the value of this grammar block
grammar_block& grammar_block::operator=(const grammar_block& copyFrom) {
    // Delete the nonterminals
    for (nonterminal_definition_list::iterator toDelete = m_Nonterminals.begin(); toDelete != m_Nonterminals.end(); ++toDelete) {
        delete *toDelete;
    }
    m_Nonterminals.clear();
    
    // Copy the nonterminals from the source
    for (iterator toCopy = copyFrom.begin(); toCopy != copyFrom.end(); ++toCopy) {
        m_Nonterminals.push_back(new nonterminal_definition(**toCopy));
    }
    
    return *this;
}

/// \brief Adds a new nonterminal to this object, which will become responsible for freeing it
void grammar_block::add_nonterminal(nonterminal_definition* newNonterminal) {
    m_Nonterminals.push_back(newNonterminal);
}
