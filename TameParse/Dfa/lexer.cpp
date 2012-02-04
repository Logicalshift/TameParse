//
//  lexer.cpp
//  Parse
//
//  Created by Andrew Hunter on 28/04/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#include "TameParse/Dfa/lexer.h"

using namespace dfa;

/// \brief Creates a default lexer
lexer::lexer() 
: m_Ndfa(new ndfa_regex())
, m_Lexer(NULL)
, m_OwnsLexer(true) {
}

/// \brief Creates an instance of this class that will use the specified NDFA for building the lexer
///
/// The supplied NDFA will be destroyed when this class is destroyed (or when it gets compiled).
lexer::lexer(ndfa_regex* ndfa)
: m_Ndfa(ndfa)
, m_Lexer(NULL)
, m_OwnsLexer(true) {
    if (m_Ndfa == NULL) m_Ndfa = new ndfa_regex();
}

/// \brief Creates an instance of this class that will use the specified DFA for building the lexer
///
/// The DFA will be compiled immediately into a lexer, and can be discarded after this call. Note that this
/// call will produce an invalid lexer if the supplied object is not deterministic.
lexer::lexer(const ndfa& dfa)
: m_Ndfa(NULL)
, m_Lexer(NULL)
, m_OwnsLexer(true) {
    m_Lexer = new dfa_lexer<wchar_t, state_machine_flat_table>(dfa);
}

/// \brief Creates an instance of this class that will use the specified basic_lexer
///
/// The lexer supplied to this call will be destroyed when this class is destroyed
lexer::lexer(basic_lexer* lexer, bool ownsLexer)
: m_Ndfa(NULL)
, m_Lexer(lexer)
, m_OwnsLexer(ownsLexer) {
    if (m_Lexer == NULL) {
        m_Ndfa = new ndfa_regex();
    }
}

/// \brief Destructor
lexer::~lexer() {
    if (m_Ndfa) {
        delete m_Ndfa;
    }
    
    if (m_Lexer && m_OwnsLexer) {
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
void lexer::compile(bool compact) {
    if (m_Lexer) return;
    if (!m_Ndfa) return;
    
    // Turn the NDFA into a DFA. Delete the intermediate stages as we go.
    ndfa* symbols = m_Ndfa->to_ndfa_with_unique_symbols();
    delete m_Ndfa; 
    m_Ndfa = NULL;
    
    ndfa* dfa = symbols->to_dfa();
    delete symbols;
    
    // Create the lexer
    if (compact) {
        m_Lexer = new dfa_lexer<wchar_t, state_machine_compact_table<> >(*dfa);        
    } else {
        m_Lexer = new dfa_lexer<wchar_t, state_machine_flat_table>(*dfa);
    }
    
    // Done with the NDFA/DFA
    delete dfa;
}

/// \brief Verifies that this lexer will compile into a valid DFA
bool lexer::verify() {
    if (m_Lexer) return false;
    if (!m_Ndfa) return false;
    
    // Turn the NDFA into a DFA. Delete the intermediate stages as we go.
    ndfa* symbols = m_Ndfa->to_ndfa_with_unique_symbols();
    ndfa* dfa = symbols->to_dfa();

    // Check that the DFA and no overlap symbols NDFA have been generated correctly
    bool isOk = true;

    if (!symbols->verify_no_symbol_overlap()) {
        isOk = false;
    }
    if (!dfa->verify_no_symbol_overlap()) {
        isOk = false;
    }
    if (!dfa->verify_is_dfa()) {
        isOk = false;
    }
    
    // Finished with the DFA and symbols objects
    delete symbols;
    delete dfa;
    
    return isOk;
}

/// \brief Estimation of the size of this lexer
size_t lexer::size() const {
    if (!m_Lexer) return 0;
    return m_Lexer->size();
}
