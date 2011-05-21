//
//  bootstrap.cpp
//  Parse
//
//  Created by Andrew Hunter on 21/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "bootstrap.h"

#include "Dfa/ndfa_regex.h"

using namespace dfa;
using namespace contextfree;
using namespace lr;
using namespace language;

/// \brief Creates the lexer for the language
dfa::ndfa* bootstrap::create_dfa() {
    // Create the NDFA (which we'll eventually turn into the lexer)
    ndfa_regex* languageNdfa = new ndfa_regex();
    
    // The lexical constructs
    languageNdfa->add_regex(0, "[A-Za-z\\-][A-Za-z\\-0-9]*", identifier);
    languageNdfa->add_regex(0, "\\<[A-Za-z\\-][A-Za-z\\-0-9]*\\>", nonterminal);
    languageNdfa->add_regex(0, "/([^/]|(\\\\/))*/", regex);
    languageNdfa->add_regex(0, "\"([^\"]|(\\\"))*\"", string);
    languageNdfa->add_regex(0, "'(.|(\\.))'", character);

    // The weak keywords
    languageNdfa->add_regex(0, "language", language);
    languageNdfa->add_regex(0, "grammar", grammar);
    languageNdfa->add_regex(0, "lexer-symbols", lexersymbols);
    languageNdfa->add_regex(0, "lexer", lexer);
    languageNdfa->add_regex(0, "ignore", ignore);
    languageNdfa->add_regex(0, "keywords", keywords);
    
    // Single character elements (these are also weak)
    languageNdfa->add_regex(0, "\\=", equals);
    languageNdfa->add_regex(0, "\\?", question);
    languageNdfa->add_regex(0, "\\+", plus);
    languageNdfa->add_regex(0, "\\*", star);
    languageNdfa->add_regex(0, "\\:", colon);
    languageNdfa->add_regex(0, "\\(", openparen);
    languageNdfa->add_regex(0, "\\)", closeparen);
    languageNdfa->add_regex(0, "\\{", opencurly);
    languageNdfa->add_regex(0, "\\}", closecurly);
    
    // Ignored elements
    languageNdfa->add_regex(0, "[\n\r]", newline);
    languageNdfa->add_regex(0, "[ ]", whitespace);
    languageNdfa->add_regex(0, "//[^\n\r]*", comment);
    
    // Build into a DFA
    ndfa* uniqueSyms = languageNdfa->to_ndfa_with_unique_symbols();
    delete languageNdfa;
    
    ndfa* result = uniqueSyms->to_dfa();
    delete uniqueSyms;
    
    return result;
}

/// \brief Constructs the bootstrap language
bootstrap::bootstrap() {
    // Create the DFA for this language
    ndfa* dfa = create_dfa();
    
    // Create the lexer for this language
    m_Lexer = new dfa::lexer(*dfa);
    
    // No longer need the DFA at this point
    delete dfa;
}

/// \brief Destructor
bootstrap::~bootstrap() {
    delete m_Lexer;
}
