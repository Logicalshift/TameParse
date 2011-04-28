//
//  basic_lexer.cpp
//  Parse
//
//  Created by Andrew Hunter on 28/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "basic_lexer.h"

using namespace dfa;

/// \brief Destructor
basic_lexer::~basic_lexer() { }

/// \brief Destructor
lexer_symbol_stream::~lexer_symbol_stream() { }

/// \brief Sets the initial state to be used by the next run through of the state machine
void lexeme_stream::set_initial_state(int initialState) {
    // Default action is to do nothing
}
