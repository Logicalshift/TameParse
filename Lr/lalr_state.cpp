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
lalr_state::lalr_state(const lalr_state& copyFrom)
: m_State(copyFrom.m_State) {
    for (lookahead_for_item::const_iterator it=copyFrom.m_Lookahead.begin(); it != copyFrom.m_Lookahead.end(); it++) {
        m_Lookahead.push_back(new lr1_item::lookahead_set(**it));
    }    
}

/// \brief Destroys this state
lalr_state::~lalr_state() {
    // Destroy all the lookahead sets
    for (lookahead_for_item::iterator it=m_Lookahead.begin(); it != m_Lookahead.end(); it++) {
        delete *it;
    }
}

bool lalr_state::operator<(const lalr_state& compareTo) const {
    // Comparison only works on the state, not on the lookahead
    return m_State < compareTo.m_State;
}

bool lalr_state::operator==(const lalr_state& compareTo) const {
    return m_State == compareTo.m_State;
}

/// \brief Adds a new item to this object. Returns true if the operation modified this container
int lalr_state::add(const container& newItem) {
    // Add the LR(0) state to this object
    // Identifiers are the same as in the state, we assume they increase monotonically from 0
    int newId = m_State.add(newItem);
    
    // Create a lookahead set for this item
    while (newId >= m_Lookahead.size()) {
        m_Lookahead.push_back(new lr1_item::lookahead_set());
    }
    
    // Return the identifier
    return newId;
}

/// \brief The first item in this state
lalr_state::all_iterator lalr_state::begin() const {
    return m_State.begin();
}

/// \brief The final item in this state
lalr_state::all_iterator lalr_state::end() const {
    return m_State.end();
}

/// \brief Finds the identifier for the specified LR(0) item
int lalr_state::find_identifier(const container& item) const {
    return m_State.find_identifier(item);
}

/// \brief Returns the lookahead set for the item with the specified ID
lr1_item::lookahead_set& lalr_state::lookahead_for(int identifier) {
    return *m_Lookahead[identifier];
}

/// \brief Returns the lookahead set for the item with the specified ID
const lr1_item::lookahead_set& lalr_state::lookahead_for(int identifier) const {
    return *m_Lookahead[identifier];
}
