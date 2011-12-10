//
//  accept_action.cpp
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#include "TameParse/Dfa/accept_action.h"

using namespace dfa;

/// \brief Creates a standard accept action for the specified symbol
accept_action::accept_action(int symbol)
: m_Eager(false)
, m_Symbol(symbol) { 
}

/// \brief Creates an accept action with the specified 'eagerness'
accept_action::accept_action(int symbol, bool isEager)
: m_Symbol(symbol)
, m_Eager(isEager) {
}

/// \brief For subclasses, allows the NDFA to clone this accept action for storage purposes
accept_action* accept_action::clone() const {
    return new accept_action(m_Symbol, m_Eager);
}

/// \brief Destructor
accept_action::~accept_action() { }

/// \brief Determines if this action is equivalent to another
bool accept_action::operator==(const accept_action* compareTo) const {
    // Deal with the NULL case first
    if (!compareTo) return false;
    
    // Compare the target state to this one
    if (m_Symbol != compareTo->symbol())    return false;
    if (m_Eager != compareTo->eager())      return false;
    
    return true;
}

/// \brief Determines if this action is less important than another
///
/// By default, actions with lower symbol IDs are more important than those with higher symbol IDs
bool accept_action::operator<(const accept_action& compareTo) const {
    return symbol() > compareTo.symbol();
}
