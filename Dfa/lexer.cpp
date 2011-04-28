//
//  lexer.cpp
//  Parse
//
//  Created by Andrew Hunter on 28/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "lexer.h"

using namespace dfa;

/// \brief Creates a default lexer
lexer::lexer() 
: m_Ndfa(new ndfa_regex())
, m_Lexer(NULL) {
}

/// \brief Creates an instance of this class that will use the specified NDFA for building the lexer
///
/// The supplied NDFA will be destroyed when this class is destroyed (or when it gets compiled).
lexer::lexer(ndfa_regex* ndfa)
: m_Ndfa(ndfa)
, m_Lexer(NULL) {
    if (m_Ndfa == NULL) m_Ndfa = new ndfa_regex();
}

/// \brief Creates an instance of this class that will use the specified basic_lexer
///
/// The lexer supplied to this call will be destroyed when this class is destroyed
lexer::lexer(basic_lexer* lexer)
: m_Ndfa(NULL)
, m_Lexer(lexer) {
    if (m_Lexer == NULL) {
        m_Ndfa = new ndfa_regex();
    }
}

/// \brief Destructor
lexer::~lexer() {
    if (m_Ndfa) {
        delete m_Ndfa;
    }
    
    if (m_Lexer) {
        delete m_Lexer;
    }
}

///
/// \brief Creates a new lexer to process the specified symbol stream
///
/// The lexeme_stream should take ownership of the supplied lexer_symbol_stream and delete it once it has finished with it
///
lexeme_stream* lexer::create_stream(lexer_symbol_stream* stream) const {
    if (!m_Lexer) {
        // Compile this lexer if it's not compiled already
        // (hideous, but we want compile to be unavailable if this is a const object)
        ((lexer*)this)->compile();
    }
    
    if (!m_Lexer) return NULL;
    
    return m_Lexer->create_stream(stream);
}

/// \brief Adds a new symbol to this lexer, if it isn't compiled
void lexer::add_symbol(const symbol_string& regex, int symbolId) {
    // Can't add any new regexps once we're compiled
    if (!m_Ndfa) return;
    
    // Add this regex to the NDFA
    m_Ndfa->add_regex(0, regex, accept_action(symbolId, false));
}

/// \brief Compiles this lexer so that it is ready for use
void lexer::compile() {
    if (m_Lexer) return;
    if (!m_Ndfa) return;
    
    // Turn the NDFA into a DFA. Delete the intermediate stages as we go.
    ndfa* symbols = m_Ndfa->to_ndfa_with_unique_symbols();
    delete m_Ndfa; 
    m_Ndfa = NULL;
    
    ndfa* dfa = symbols->to_dfa();
    delete symbols;
    
    // Create the lexer
    m_Lexer = new dfa_lexer<wchar_t>(*dfa);
    
    // Done with the NDFA/DFA
    delete dfa;
}
