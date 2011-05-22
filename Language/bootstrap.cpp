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

/// \brief Adds a new terminal item to an NDFA, and to this object
contextfree::item_container bootstrap::add_terminal(dfa::ndfa_regex* ndfa, const std::wstring& name, const std::wstring& regex) {
    // Add to the terminal dictionary
    int termIdentifier = m_Terminals.add_symbol(name);
    
    // Add to the DFA
    ndfa->add_regex(0, regex, termIdentifier);
    
    // Return a new container
    return item_container(new terminal(termIdentifier), true);
}

/// \brief Creates the lexer for the language
dfa::ndfa* bootstrap::create_dfa() {
    // Create the NDFA (which we'll eventually turn into the lexer)
    ndfa_regex* languageNdfa = new ndfa_regex();
    
    // The lexical constructs
    t.identifier        = add_terminal(languageNdfa, L"identifier", L"[A-Za-z\\-][A-Za-z\\-0-9]*");
    t.nonterminal       = add_terminal(languageNdfa, L"nonterminal", L"\\<[A-Za-z\\-][A-Za-z\\-0-9]*\\>");
    t.regex             = add_terminal(languageNdfa, L"regex", L"/([^/]|(\\\\/))*/");
    t.string            = add_terminal(languageNdfa, L"string", L"\"([^\"]|(\\\"))*\"");
    t.character         = add_terminal(languageNdfa, L"character", L"'(.|(\\.))'");

    // The weak keywords
    t.language          = add_terminal(languageNdfa, L"language", L"language");
    t.grammar           = add_terminal(languageNdfa, L"grammar", L"grammar");
    t.lexersymbols      = add_terminal(languageNdfa, L"lexer-symbols", L"lexer-symbols");
    t.lexer             = add_terminal(languageNdfa, L"lexer", L"lexer");
    t.ignore            = add_terminal(languageNdfa, L"ignore", L"ignore");
    t.keywords          = add_terminal(languageNdfa, L"keywords", L"keywords");
    
    // Single character elements (these are also weak)
	t.equals            = add_terminal(languageNdfa, L"'='", L"\\=");
    t.question          = add_terminal(languageNdfa, L"'?'", L"\\?");
    t.plus              = add_terminal(languageNdfa, L"'+'", L"\\+");
    t.star              = add_terminal(languageNdfa, L"'*'", L"\\*");
    t.colon             = add_terminal(languageNdfa, L"':'", L"\\:");
    t.openparen         = add_terminal(languageNdfa, L"'('", L"\\(");
    t.closeparen        = add_terminal(languageNdfa, L"')'", L"\\)");
    t.opencurly         = add_terminal(languageNdfa, L"'{'", L"\\{");
    t.closecurly        = add_terminal(languageNdfa, L"'}'", L"\\}");
    
    // Ignored elements
    t.newline           = add_terminal(languageNdfa, L"newline", L"[\n\r]");
    t.whitespace        = add_terminal(languageNdfa, L"whitespace", L"[ ]");
    t.comment           = add_terminal(languageNdfa, L"comment", L"//[^\n\r]*");
    
    // Build into a DFA
    ndfa* uniqueSyms = languageNdfa->to_ndfa_with_unique_symbols();
    delete languageNdfa;
    
    ndfa* result = uniqueSyms->to_dfa();
    delete uniqueSyms;
    
    return result;
}

/// \brief Creates the grammar for the language
contextfree::grammar* bootstrap::create_grammar() {
    // Create a new grammar
    grammar* result = new grammar();
    
    // Generate nonterminals
    nt.parser_language          = result->get_nonterminal(L"Parser-Language");
    nt.toplevel_block           = result->get_nonterminal(L"TopLevel-Block");
    nt.language_block           = result->get_nonterminal(L"Language-Block");
    nt.language_inherits        = result->get_nonterminal(L"Language-Inherits");
    nt.language_definition      = result->get_nonterminal(L"Language-Definition");
    nt.lexer_symbols_definition = result->get_nonterminal(L"Lexer-Symbols-Definition");
    nt.lexer_definition         = result->get_nonterminal(L"Lexer-Definition");
    nt.ignore_definition        = result->get_nonterminal(L"Ignore-Definition");
    nt.keywords_definition      = result->get_nonterminal(L"Keywords-Definition");
    nt.weak_symbols_definition  = result->get_nonterminal(L"Weak-Symbols-Definition");
    nt.lexeme_definition        = result->get_nonterminal(L"Lexeme-Definition");
    nt.grammar_definition       = result->get_nonterminal(L"Grammar-Definition");
    nt.nonterminal_definition   = result->get_nonterminal(L"Nonterminal-Definition");
    nt.production               = result->get_nonterminal(L"Production");
    nt.ebnf_item                = result->get_nonterminal(L"Ebnf-Item");
    nt.simple_ebnf_item         = result->get_nonterminal(L"Simple-Ebnf-Item");
    nt.nonterminal              = result->get_nonterminal(L"Nonterminal");
    nt.terminal                 = result->get_nonterminal(L"Terminal");
    nt.basic_terminal           = result->get_nonterminal(L"Basic-Terminal");
    
    // Generate productions
    ebnf_repeating_optional listToplevel;
    (*listToplevel.get_rule()) << nt.toplevel_block;
    ((*result) += L"Parser-Language") << listToplevel;
    
    // Top level block (language)
    ((*result) += L"TopLevel-Block") << nt.language_block;
    
    // Language block ('language x (: y) { ... }
    ebnf_optional           optionalLanguageInherits;
    ebnf_repeating_optional listLanguageDefinition;
    (*optionalLanguageInherits.get_rule()) << nt.language_inherits;
    (*listLanguageDefinition.get_rule()) << nt.language_definition;
    ((*result) += L"Language-Block") << t.language << t.identifier << optionalLanguageInherits << t.opencurly << listLanguageDefinition << t.closecurly;
    
    ((*result) += L"Language-Inherits") << t.colon << t.identifier;
        
    // Return the new grammar
    return result;
}

/// \brief Constructs the bootstrap language
bootstrap::bootstrap() {
    // Create the DFA for this language
    ndfa* dfa = create_dfa();
    
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
