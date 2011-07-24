//
//  language_block.cpp
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "language_block.h"

using namespace std;
using namespace dfa;
using namespace language;

/// \brief Creates a new empty language block with the specified identifier
language_block::language_block(const std::wstring& identifier, position start, position end)
: block(start, end)
, m_Identifier(identifier) {
}

/// \brief Creates a copy of this language block
language_block::language_block(const language_block& copyFrom) {
    (*this) = copyFrom;
}

/// \brief Copies a language block into this object
language_block& language_block::operator=(const language_block& copyFrom) {
    // Nothing to do if copyFrom is the same as this object
    if (&copyFrom == this) return *this;
    
    // Clear out the existing units
    for (unit_list::iterator toDelete = m_Units.begin(); toDelete != m_Units.end(); toDelete++) {
        delete *toDelete;
    }
    m_Units.clear();
    
    // Strings can just be copied
    m_Identifier    = copyFrom.m_Identifier;
    m_Inherits      = copyFrom.m_Inherits;
    
    set_start_pos(copyFrom.start_pos());
    set_end_pos(copyFrom.end_pos());
    
    // Units need to be copied separately
    for (language_block::iterator unit = copyFrom.begin(); unit != copyFrom.end(); unit++) {
        m_Units.push_back(new language_unit(**unit));
    }
    
    // Done
    return *this;
}

/// \brief Destructor
language_block::~language_block() {
    // Clear out the existing units
    for (unit_list::iterator toDelete = m_Units.begin(); toDelete != m_Units.end(); toDelete++) {
        delete *toDelete;
    }
    m_Units.clear();
}

/// \brief Adds the identifier of a language that this language inherits from
void language_block::add_inherits(const std::wstring& identifier) {
    m_Inherits.push_back(identifier);
}

/// \brief Adds a language unit to this object (the unit becomes owned by this object, which will destroy it when it is itself destroyed)
void language_block::add_unit(language_unit* unit) {
    m_Units.push_back(unit);
}
