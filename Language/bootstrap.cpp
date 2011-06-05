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
#include "Lr/ignored_symbols.h"
#include "Lr/lalr_builder.h"

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
    t.weaklexer         = add_terminal(languageNdfa, L"weaklexer", L"weaklexer");
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
    t.dot               = add_terminal(languageNdfa, L"'.'", L"\\.");
    t.pipe              = add_terminal(languageNdfa, L"'|'", L"\\|");
    
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
    nt.keyword_definition       = result->get_nonterminal(L"Keyword-Definition");
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
    
    // Types of definition within a language block
    ((*result) += L"Language-Definition") << nt.lexer_symbols_definition;
    ((*result) += L"Language-Definition") << nt.lexer_definition;
    ((*result) += L"Language-Definition") << nt.ignore_definition;
    ((*result) += L"Language-Definition") << nt.weak_symbols_definition;
    ((*result) += L"Language-Definition") << nt.keywords_definition;
    ((*result) += L"Language-Definition") << nt.grammar_definition;
    
    // Some simple definitions
    ebnf_repeating_optional lexemeList;
    ebnf_repeating_optional keywordDefinitionList;
    ebnf_repeating_optional lexemeOrIdentifierList;
    ebnf_alternate          lexemeOrIdentifier;
    
    (*lexemeList.get_rule()) << nt.lexeme_definition;
    (*lexemeOrIdentifier.get_rule()) << nt.lexeme_definition;
    (*lexemeOrIdentifier.add_rule()) << t.identifier;
    (*lexemeOrIdentifierList.get_rule()) << lexemeOrIdentifier;
    (*keywordDefinitionList.get_rule()) << nt.keyword_definition;
    
    ((*result) += L"Lexer-Symbols-Definition") << t.lexersymbols << t.opencurly << lexemeList << t.closecurly;
    ((*result) += L"Lexer-Definition") << t.lexer << t.opencurly << lexemeList << t.closecurly;
    ((*result) += L"Ignore-Definition") << t.ignore << t.opencurly << lexemeOrIdentifierList << t.closecurly;
    ((*result) += L"Keywords-Definition") << t.keywords << t.opencurly << keywordDefinitionList << t.closecurly;
    ((*result) += L"Keyword-Definition") << t.identifier;
    ((*result) += L"Keyword-Definition") << t.identifier << t.equals << t.string;
    ((*result) += L"Keyword-Definition") << t.identifier << t.equals << t.character;
    ((*result) += L"Weak-Symbols-Definition") << t.weaklexer << t.opencurly << lexemeList << t.closecurly;
    ((*result) += L"Lexeme-Definition") << t.identifier << t.equals << t.regex;
    ((*result) += L"Lexeme-Definition") << t.identifier << t.equals << t.identifier << t.dot << t.identifier;
    
    // Definitions for a grammar
    ebnf_repeating_optional nonterminalDefinitionList;
    ebnf_repeating_optional orProductionList;
    ebnf_repeating_optional ebnfItemList;
    ebnf_repeating_optional simpleEbnfItemList;
    
    (*nonterminalDefinitionList.get_rule()) << nt.nonterminal_definition;
    (*orProductionList.get_rule()) << t.pipe << nt.production;
    (*ebnfItemList.get_rule()) << nt.ebnf_item;
    (*simpleEbnfItemList.get_rule()) << nt.simple_ebnf_item;
    
    ((*result) += L"Grammar-Definition") << t.grammar << t.opencurly << nonterminalDefinitionList << t.closecurly;
    
    ((*result) += L"Nonterminal-Definition") << t.nonterminal << t.equals << nt.production << orProductionList;
    // (Not supporting the inheritance forms of these in the bootstrap language)
    
    // Productions
    ((*result) += L"Production") << simpleEbnfItemList;
    
    ((*result) += L"Ebnf-Item") << nt.simple_ebnf_item;
    ((*result) += L"Ebnf-Item") << nt.simple_ebnf_item << t.pipe << nt.simple_ebnf_item;
    
    ((*result) += L"Simple-Ebnf-Item") << nt.nonterminal;
    ((*result) += L"Simple-Ebnf-Item") << nt.terminal;
    ((*result) += L"Simple-Ebnf-Item") << nt.simple_ebnf_item << t.star;
    ((*result) += L"Simple-Ebnf-Item") << nt.simple_ebnf_item << t.plus;
    ((*result) += L"Simple-Ebnf-Item") << nt.simple_ebnf_item << t.question;
    ((*result) += L"Simple-Ebnf-Item") << t.openparen << ebnfItemList << t.closeparen;
    
    ((*result) += L"Nonterminal") << t.nonterminal;
    
    ((*result) += L"Terminal") << nt.basic_terminal;
    ((*result) += L"Basic-Terminal") << t.identifier;
    ((*result) += L"Basic-Terminal") << t.string;
    ((*result) += L"Basic-Terminal") << t.character;
    
    // Return the new grammar
    return result;
}

/// \brief Constructs the bootstrap language
bootstrap::bootstrap() {
    // Create the DFA for this language
    ndfa* dfa = create_dfa();
    
    // Create the grammar for this language
    m_Grammar = create_grammar();
    
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
    
    // Set up the list of ignored symbols
    ignored_symbols ignore;
    
    ignore.add_item(t.newline);
    ignore.add_item(t.whitespace);
    ignore.add_item(t.comment);
    
    // Create the lexer for this language
    m_Lexer = new dfa::lexer(*dfa);
    
    // No longer need the DFA at this point
    delete dfa;
    
    // Build the parser
    m_Builder = new lalr_builder(*m_Grammar);
    
    m_Builder->add_rewriter(weak);
    m_Builder->add_rewriter(ignore);
    
    m_Builder->add_initial_state(nt.parser_language);
    
    // Finish up the parser
    m_Builder->complete_parser();
    
    // TODO: log information about shift/reduce and reduce/reduce conflicts
    
    // Turn into the finished parser
    m_Parser = new ast_parser(*m_Builder);
}

/// \brief Destructor
bootstrap::~bootstrap() {
    delete m_Lexer;
    delete m_Grammar;
    delete m_Builder;
    delete m_Parser;
}
