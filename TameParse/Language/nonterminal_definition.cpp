//
//  nonterminal_definition.cpp
//  Parse
//
//  Created by Andrew Hunter on 24/07/2011.
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

#include "TameParse/Language/nonterminal_definition.h"

using namespace yy_language;

/// \brief Creates a new nonterminal definition for a nonterminal with the specified identifier
nonterminal_definition::nonterminal_definition(type definitionType, const std::wstring& identifier, position start, position end)
: block(start, end)
, m_Type(definitionType)
, m_Identifier(identifier ){
}

/// \brief Creates a new nonterminal definition by copying an existing one
nonterminal_definition::nonterminal_definition(const nonterminal_definition& copyFrom) {
    (*this) = copyFrom;
}

/// \brief Destructor
nonterminal_definition::~nonterminal_definition() {
    // Destroy the productions
    for (production_definition_list::iterator toDelete = m_Productions.begin(); toDelete != m_Productions.end(); ++toDelete) {
        delete *toDelete;
    }
    m_Productions.clear();
}

/// \brief Assigns the value of this nonterminal by copying an existing one
nonterminal_definition& nonterminal_definition::operator=(const nonterminal_definition& copyFrom) {
    // Can't assign to ourselves
    if (&copyFrom == this) return *this;
    
    // Destroy the productions
    for (production_definition_list::iterator toDelete = m_Productions.begin(); toDelete != m_Productions.end(); ++toDelete) {
        delete *toDelete;
    }
    m_Productions.clear();

    // Assign the productions from the copy
    for (nonterminal_definition::iterator toCopy = copyFrom.begin(); toCopy != copyFrom.end(); ++toCopy) {
        m_Productions.push_back(new production_definition(**toCopy));
    }
    
    m_Identifier = copyFrom.identifier();
    set_start_pos(copyFrom.start_pos());
    set_end_pos(copyFrom.end_pos());
    
    return *this;
}

/// \brief Adds a new production to this object
///
/// This object will become responsible for destroying the production passed in to this call
void nonterminal_definition::add_production(production_definition* newProduction) {
    m_Productions.push_back(newProduction);
}
