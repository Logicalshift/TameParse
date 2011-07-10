//
//  transition.cpp
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "transition.h"

using namespace dfa;

/// \brief Creates a new DFA transition
transition::transition(int symbolSet, int newState)
: m_SymbolSet(symbolSet)
, m_NewState(newState) {
}

/// \brief Determines if this set represents the same as another set
bool transition::operator==(const transition& compareTo) const {
    return compareTo.m_NewState == m_NewState && compareTo.m_SymbolSet == m_SymbolSet;
}

/// \brief Orders this symbol set
bool transition::operator<(const transition& compareTo) const {
    if (m_NewState < compareTo.m_NewState)  return true;
    if (m_NewState > compareTo.m_NewState)  return false;
    
    return m_SymbolSet < compareTo.m_SymbolSet;
}

/// \brief Orders this symbol set
bool transition::operator<=(const transition& compareTo) const {
    return m_NewState < compareTo.m_NewState || m_SymbolSet < compareTo.m_SymbolSet || operator==(compareTo);
}
