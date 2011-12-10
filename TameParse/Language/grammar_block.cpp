//
//  grammar_block.cpp
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
//

#include "TameParse/Language/grammar_block.h"

using namespace language;

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
