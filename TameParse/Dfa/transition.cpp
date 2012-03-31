//
//  transition.cpp
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
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

#include "TameParse/Dfa/transition.h"

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
    if (m_SymbolSet < compareTo.m_SymbolSet)  return true;
    if (m_SymbolSet > compareTo.m_SymbolSet)  return false;
    
    return m_NewState < compareTo.m_NewState;
}

/// \brief Orders this symbol set
bool transition::operator<=(const transition& compareTo) const {
    return m_NewState < compareTo.m_NewState || m_SymbolSet < compareTo.m_SymbolSet || operator==(compareTo);
}
