//
//  character_lexer.cpp
//  Parse
//
//  Created by Andrew Hunter on 14/05/2011.
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
