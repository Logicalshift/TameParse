//
//  bootstrap.cpp
//  Parse
//
//  Created by Andrew Hunter on 21/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "bootstrap.h"

#include "Dfa/ndfa_regex.h"
#include "ContextFree/item.h"
#include "Lr/weak_symbols.h"

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

/// \brief Fills in the terminals structure
void bootstrap::create_terminals() {
    typedef contextfree::terminal term;
    
    t.identifier        = item_container(new term(identifier), true);
    t.nonterminal       = item_container(new term(nonterminal), true);
    t.regex             = item_container(new term(regex), true);
    t.string            = item_container(new term(string), true);
    t.character         = item_container(new term(character), true);

    t.language          = item_container(new term(language), true);
    t.grammar           = item_container(new term(grammar), true);
    t.lexersymbols      = item_container(new term(lexersymbols), true);
    t.lexer             = item_container(new term(lexer), true);
    t.ignore            = item_container(new term(ignore), true);
    t.keywords          = item_container(new term(keywords), true);

    t.equals            = item_container(new term(equals), true);
    t.question          = item_container(new term(question), true);
    t.plus              = item_container(new term(plus), true);
    t.star              = item_container(new term(star), true);
    t.colon             = item_container(new term(colon), true);
    t.openparen         = item_container(new term(openparen), true);
    t.closeparen        = item_container(new term(closeparen), true);
    t.opencurly         = item_container(new term(opencurly), true);
    t.closecurly        = item_container(new term(closecurly), true);

    t.newline           = item_container(new term(newline), true);
    t.whitespace        = item_container(new term(whitespace), true);
    t.comment           = item_container(new term(comment), true);
    
    m_Terminals.add_symbol(L"identifier", identifier);
    m_Terminals.add_symbol(L"nonterminal", nonterminal);
    m_Terminals.add_symbol(L"regex", regex);
    m_Terminals.add_symbol(L"string", string);
    m_Terminals.add_symbol(L"character", character);

    m_Terminals.add_symbol(L"language", language);
    m_Terminals.add_symbol(L"grammar", grammar);
    m_Terminals.add_symbol(L"lexersymbols", lexersymbols);
    m_Terminals.add_symbol(L"lexer", lexer);
    m_Terminals.add_symbol(L"ignore", ignore);
    m_Terminals.add_symbol(L"keywords", keywords);

    m_Terminals.add_symbol(L"equals", equals);
    m_Terminals.add_symbol(L"question", question);
    m_Terminals.add_symbol(L"plus", plus);
    m_Terminals.add_symbol(L"star", star);
    m_Terminals.add_symbol(L"colon", colon);
    m_Terminals.add_symbol(L"openparen", openparen);
    m_Terminals.add_symbol(L"closeparen", closeparen);
    m_Terminals.add_symbol(L"opencurly", opencurly);
    m_Terminals.add_symbol(L"closecurly", closecurly);

    m_Terminals.add_symbol(L"newline", newline);
    m_Terminals.add_symbol(L"whitespace", whitespace);
    m_Terminals.add_symbol(L"comment", comment);
}

/// \brief Constructs the bootstrap language
bootstrap::bootstrap() {
    // Create the DFA for this language
    ndfa* dfa = create_dfa();
    
    // Create the terminal objects
    create_terminals();
    
    // Set up the list of 'weak symbols'
    weak_symbols weak;
    
    // All of the keywords are weak
    item_set weaklings;

    weaklings.insert(t.language);
    weaklings.insert(t.grammar);
    weaklings.insert(t.lexersymbols);
    weaklings.insert(t.lexer);
    weaklings.insert(t.ignore);
    weaklings.insert(t.keywords);
    
    weak.add_symbols(*dfa, weaklings, m_Terminals);
    
    // Create the lexer for this language
    m_Lexer = new dfa::lexer(*dfa);
    
    // No longer need the DFA at this point
    delete dfa;
}

/// \brief Destructor
bootstrap::~bootstrap() {
    delete m_Lexer;
}
