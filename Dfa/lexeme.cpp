//
//  lexeme.cpp
//  Parse
//
//  Created by Andrew Hunter on 28/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "lexeme.h"

using namespace dfa;

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
