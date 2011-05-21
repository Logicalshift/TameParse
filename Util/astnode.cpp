//
//  astnode.cpp
//  Parse
//
//  Created by Andrew Hunter on 15/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "astnode.h"

using namespace dfa;
using namespace util;

/// \brief Creates an AST node with a nonterminal ID
astnode::astnode(int nonterminal, int rule)
: m_Nonterminal(nonterminal)
, m_Rule(rule)
, m_Lexeme(NULL, false) {
}

/// \brief Creates an AST node from a lexeme
astnode::astnode(const dfa::lexeme_container& terminal)
: m_Nonterminal(-1)
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
    
    if (m_Nonterminal < compareTo.m_Nonterminal || m_Lexeme < compareTo.m_Lexeme) return true;
    if (m_Nonterminal > compareTo.m_Nonterminal || m_Lexeme > compareTo.m_Lexeme) return false;
    
    if (children().size() < compareTo.children().size()) return true;
    if (children().size() > compareTo.children().size()) return false;
    
    for (int x=0; x<children().size(); x++) {
        if (children()[x] < compareTo.children()[x]) return true;
        if (children()[x] > compareTo.children()[x]) return false;
    }
    
    return false;
}
