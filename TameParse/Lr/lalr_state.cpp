//
//  lalr_state.cpp
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
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

#include "TameParse/Lr/lalr_state.h"

using namespace contextfree;
using namespace lr;

/// \brief Constructs an empty state
lalr_state::lalr_state() {
}

/// \brief Copies this state
lalr_state::lalr_state(const lalr_state& copyFrom)
: m_State(copyFrom.m_State) {
    for (lookahead_for_item::const_iterator la=copyFrom.m_Lookahead.begin(); la != copyFrom.m_Lookahead.end(); ++la) {
        m_Lookahead.push_back(new lr1_item::lookahead_set(**la));
    }    
}

/// \brief Destroys this state
lalr_state::~lalr_state() {
    // Destroy all the lookahead sets
    for (lookahead_for_item::iterator la=m_Lookahead.begin(); la != m_Lookahead.end(); ++la) {
        delete *la;
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
int lalr_state::add(const container& newItem, const grammar* gram) {
    // Add the LR(0) state to this object
    // Identifiers are the same as in the state, we assume they increase monotonically from 0
    int newId = m_State.add(newItem);
    
    // Create a lookahead set for this item
    while (newId >= (int) m_Lookahead.size()) {
        m_Lookahead.push_back(new lr1_item::lookahead_set(gram));
    }
    
    // Return the identifier
    return newId;
}

/// \brief The first item in this state
lalr_state::iterator lalr_state::begin() const {
    return m_State.begin();
}

/// \brief The final item in this state
lalr_state::iterator lalr_state::end() const {
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
