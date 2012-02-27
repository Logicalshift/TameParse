//
//  astnode.cpp
//  Parse
//
//  Created by Andrew Hunter on 15/05/2011.
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

#include "TameParse/Util/astnode.h"

using namespace dfa;
using namespace util;

/// \brief Creates an empty AST node
astnode::astnode()
: m_ItemIdentifier(-1) 
, m_Rule(-1) 
, m_Lexeme()
, m_Children() {
}

/// \brief Creates an AST node with a nonterminal ID
astnode::astnode(int itemIdentifier, int rule)
: m_ItemIdentifier(itemIdentifier)
, m_Rule(rule)
, m_Lexeme(NULL, false) {
}

/// \brief Creates an AST node from a lexeme
astnode::astnode(const dfa::lexeme_container& terminal)
: m_ItemIdentifier(-1)
, m_Lexeme(terminal) {
}

/// \brief Adds a new child node to this item
void astnode::add_child(const astnode_container& newChild) {
    m_Children.push_back(newChild);
}
/// \brief Clones this AST node
astnode* astnode::clone() const {
    return new astnode(*this);
}

/// \brief Orders this node relative to another
bool astnode::operator<(const astnode& compareTo) const {
    if (&compareTo == this) return false;
    
    if (m_ItemIdentifier < compareTo.m_ItemIdentifier || m_Lexeme < compareTo.m_Lexeme) return true;
    if (compareTo.m_ItemIdentifier < m_ItemIdentifier || compareTo.m_Lexeme < m_Lexeme) return false;
    
    if (children().size() < compareTo.children().size()) return true;
    if (children().size() > compareTo.children().size()) return false;
    
    for (size_t x=0; x<children().size(); ++x) {
        if (children()[x] < compareTo.children()[x]) return true;
        if (compareTo.children()[x] < children()[x]) return false;
    }
    
    return false;
}
