//
//  nonterminal_definition.cpp
//  Parse
//
//  Created by Andrew Hunter on 24/07/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
//

#include "TameParse/Language/nonterminal_definition.h"

using namespace language;

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
