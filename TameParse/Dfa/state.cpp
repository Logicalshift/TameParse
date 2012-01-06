//
//  State.cpp
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#include "TameParse/Dfa/state.h"

using namespace dfa;

/// \brief Destructor
state::~state() {
}

/// \brief Copies a state
state::state(const state& copyFrom) 
: m_Identifier(copyFrom.m_Identifier) 
, m_Transitions(copyFrom.m_Transitions) {
}

/// \brief Creates a new state with the specified identifier
state::state(int identifier)
: m_Identifier(identifier) {
    
}

/// \brief Adds a new transition to this state
void state::add(const transition& newTransition) {
    m_Transitions.insert(newTransition);
}
