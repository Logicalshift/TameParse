//
//  character_lexer.cpp
//  Parse
//
//  Created by Andrew Hunter on 14/05/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
//

#include "TameParse/Dfa/character_lexer.h"

using namespace std;
using namespace dfa;

character_lexer::lstream::lstream(lexer_symbol_stream* stream) 
: m_Stream(stream) {
}

character_lexer::lstream::~lstream() {
    delete m_Stream;
}

/// \brief Fills in the contents of the specified pointer with the next lexeme (or NULL if the end of input has been reached)
lexeme_stream& character_lexer::lstream::operator>>(lexeme*& result) {
    int next;
    (*m_Stream) >> next;
    
    if (next != symbol_set::end_of_input) {
        lexeme::symbols syms;
        syms.push_back(next);
        
        result = new lexeme(syms, m_Position.current_position(), next);
        
        m_Position.update_position(next);
    } else {
        result = NULL;
    }
    
    return *this;
}

///
/// \brief Creates a new lexer to process the specified symbol stream
///
/// The lexeme_stream should take ownership of the supplied lexer_symbol_stream and delete it once it has finished with it
///
lexeme_stream* character_lexer::create_stream(lexer_symbol_stream* stream) const {
    return new lstream(stream);
}

///
/// \brief The number of bytes used by this lexer
///
size_t character_lexer::size() const {
    return sizeof(character_lexer);
}
