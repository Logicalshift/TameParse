//
//  conflict.cpp
//  Parse
//
//  Created by Andrew Hunter on 04/06/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "conflict.h"

using namespace std;
using namespace contextfree;
using namespace lr;

/// \brief Creates a new conflict object (describing a non-conflict)
conflict::conflict(int stateId, const item_container& token)
: m_StateId(stateId)
, m_Token(token) {
}

/// \brief Copy constructor
conflict::conflict(const conflict& copyFrom)
: m_StateId(copyFrom.m_StateId)
, m_Token(copyFrom.m_Token)
, m_Shift(copyFrom.m_Shift)
, m_Reduce(copyFrom.m_Reduce) {
}

/// \brief Destroys this conflict object
conflict::~conflict() { }

/// \brief Clones this conflict
conflict* conflict::clone() const {
    return new conflict(*this);
}

/// \brief Returns true if conflict a is less than conflict b
bool conflict::compare(const conflict* a, const conflict* b) {
    if (a == b) return false;
    if (!a) return true;
    if (!b) return false;
    
    return (*a) < (*b);
}

/// \brief Orders this conflict relative to another
bool conflict::operator<(const conflict& compareTo) const {
    if (m_StateId < compareTo.m_StateId) return true;
    if (m_StateId > compareTo.m_StateId) return false;
    
    if (m_Token < compareTo.m_Token) return true;
    if (m_Token > compareTo.m_Token) return false;
    
    if (m_Shift < compareTo.m_Shift) return true;
    if (m_Shift > compareTo.m_Shift) return false;
    
    if (m_Reduce < compareTo.m_Reduce) return true;
    
    return false;
}

/// \brief Adds an LR(0) item that will be followed if the token is shifted
void conflict::add_shift_item(const lr0_item_container& item) {
    m_Shift.insert(item);
}

/// \brief Adds an LR(0) item that will be reduced if the token is in the lookahead
///
/// This returns an item that the caller can add the set of reduce states for this item
conflict::possible_reduce_states& conflict::add_reduce_item(const lr0_item_container& item) {
    return m_Reduce[item];
}
