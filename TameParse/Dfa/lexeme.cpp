//
//  lexeme.cpp
//  Parse
//
//  Created by Andrew Hunter on 28/04/2011.
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

#include "TameParse/Dfa/lexeme.h"

using namespace dfa;

/// \brief Creates a nonsensical empty lexeme
lexeme::lexeme()
: m_Matched(-1) {
    
}

/// \brief Copy constructor
lexeme::lexeme(const lexeme& copyFrom) 
: m_Position(copyFrom.m_Position) 
, m_Symbols(copyFrom.m_Symbols)
, m_Matched(copyFrom.m_Matched) {
}

/// \brief Creates a new lexeme
lexeme::lexeme(const symbols& syms, const position& pos, int matched) 
: m_Position(pos)
, m_Symbols(syms)
, m_Matched(matched) {
}

/// \brief Destructor
lexeme::~lexeme() {
}

/// \brief Clone operator (so subclasses can store extra data if they need to)
lexeme* lexeme::clone() const {
    return new lexeme(*this);
}

/// \brief The final position of this lexeme
///
/// Note that the line count will be off by 1 if the symbol preceeding this lexeme is a carriage return
position lexeme::final_pos() const {
    // Use a position tracker to calculate the final position
    position_tracker tracker(m_Position);
    tracker.update_position(m_Symbols.begin(), m_Symbols.end());

    return tracker.current_position();
}

/// \brief Ordering operator
bool lexeme::operator<(const lexeme& compareTo) const {
    if (m_Matched < compareTo.m_Matched) return true;
    if (m_Matched > compareTo.m_Matched) return false;
    
    if (m_Symbols < compareTo.m_Symbols) return true;
    if (m_Symbols > compareTo.m_Symbols) return false;
    
    if (m_Position < compareTo.m_Position) return true;
    
    return false;
}
