//
//  lalr_state.cpp
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "lalr_state.h"

using namespace lr;

/// \brief Constructs an empty state
lalr_state::lalr_state() {
}

/// \brief Copies this state
lalr_state::lalr_state(lalr_state& copyFrom)
: m_State(copyFrom.m_State) {
}

bool lalr_state::operator<(const lalr_state& compareTo) const {
    return m_State < compareTo.m_State;
}

bool lalr_state::operator==(const lalr_state& compareTo) const {
    return m_State == compareTo.m_State;
}

/// \brief Adds a new item to this object. Returns true if the operation modified this container
bool lalr_state::add(const container& newItem) {
    return m_State.add(newItem);
}

/// \brief The first item in this state
lalr_state::iterator lalr_state::begin() const {
    return m_State.begin();
}

/// \brief The final item in this state
lalr_state::iterator lalr_state::end() const {
    return m_State.end();
}
