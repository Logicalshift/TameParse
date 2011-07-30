//
//  bootstrap.cpp
//  Parse
//
//  Created by Andrew Hunter on 21/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>

#include "bootstrap.h"

#include "Dfa/ndfa_regex.h"
#include "ContextFree/item.h"
#include "Lr/weak_symbols.h"
#include "Lr/ignored_symbols.h"
#include "Lr/lalr_builder.h"
#include "Language/formatter.h"

using namespace std;
using namespace util;
using namespace dfa;
using namespace contextfree;
using namespace lr;
using namespace language;

// Declare a string containing the language definition
#include "definition_txt.h"

/// \brief Retrieves a string containing the language definition for the parser language
const std::string& bootstrap::get_default_language_definition() {
    // Static string that will contain the language definition
    static std::string result;
    
    // Fill in the result if it's empty
    if (result.size() == 0) {
        result.assign((char*) definition_txt, definition_txt_len);
    }
    
    // Return the result
    return result;
}

/// \brief Adds a new terminal item to an NDFA, and to this object
contextfree::item_container bootstrap::add_terminal(dfa::ndfa_regex* ndfa, const std::wstring& name, const std::wstring& regex) {
    // Add to the terminal dictionary
    int termIdentifier = m_Terminals.symbol_for_name(name);
    
    if (termIdentifier == -1) {
        termIdentifier = m_Terminals.add_symbol(name);
    }
    
    // Add to the DFA
    ndfa->add_regex(0, regex, termIdentifier);
    
    // Return a new container
    return item_container(new terminal(termIdentifier), true);
}

/// \brief Creates the lexer for the language
dfa::ndfa* bootstrap::create_dfa() {
    // Create the NDFA (which we'll eventually turn into the lexer)
    ndfa_regex* languageNdfa = new ndfa_regex();

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
    t.openguard         = add_terminal(languageNdfa, L"\"[=>\"", L"\\[=>");
    t.closesquare       = add_terminal(languageNdfa, L"']'", L"\\]");
    
    // The lexical constructs
    t.identifier        = add_terminal(languageNdfa, L"identifier", L"[A-Za-z\\-][A-Za-z\\-0-9]*");
    t.nonterminal       = add_terminal(languageNdfa, L"nonterminal", L"\\<[A-Za-z\\-][A-Za-z\\-0-9]*\\>");
    t.regex             = add_terminal(languageNdfa, L"regex", L"/([^/]|(\\\\/))+/");
    t.string            = add_terminal(languageNdfa, L"string", L"\"([^\"]|(\\\\\"))*\"");
    t.character         = add_terminal(languageNdfa, L"character", L"'(.|(\\\\.))'");
    
    // Ignored elements
    t.newline           = add_terminal(languageNdfa, L"newline", L"[\n\r]");
    t.whitespace        = add_terminal(languageNdfa, L"whitespace", L"[ \t]+");
    t.comment           = add_terminal(languageNdfa, L"comment", L"//[^\n\r]*");
    
    // Store the IDs for the terminals
    t.id_language       = t.language	->symbol();
	t.id_grammar        = t.grammar     ->symbol();
    t.id_lexersymbols   = t.lexersymbols->symbol();
    t.id_lexer          = t.lexer       ->symbol();
    t.id_weaklexer      = t.weaklexer   ->symbol();
    t.id_ignore         = t.ignore      ->symbol();
    t.id_keywords       = t.keywords    ->symbol();
	t.id_equals         = t.equals      ->symbol();
    t.id_question       = t.question    ->symbol();
    t.id_plus           = t.plus        ->symbol();
    t.id_star           = t.star        ->symbol();
    t.id_colon          = t.colon       ->symbol();
    t.id_openparen      = t.openparen   ->symbol();
    t.id_closeparen     = t.closeparen  ->symbol();
    t.id_opencurly      = t.opencurly   ->symbol();
    t.id_closecurly     = t.closecurly  ->symbol();
    t.id_dot            = t.dot         ->symbol();
    t.id_pipe           = t.pipe        ->symbol();
    t.id_openguard      = t.openguard   ->symbol();
    t.id_closesquare    = t.closesquare ->symbol();
    t.id_identifier     = t.identifier  ->symbol();
    t.id_nonterminal    = t.nonterminal ->symbol();
    t.id_regex          = t.regex       ->symbol();
    t.id_string         = t.string      ->symbol();
    t.id_character      = t.character   ->symbol();
    t.id_newline        = t.newline     ->symbol();
    t.id_whitespace     = t.whitespace  ->symbol();
    t.id_comment        = t.comment     ->symbol();
    
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
    nt.guard                    = result->get_nonterminal(L"Guard");
    nt.nonterminal              = result->get_nonterminal(L"Nonterminal");
    nt.terminal                 = result->get_nonterminal(L"Terminal");
    nt.basic_terminal           = result->get_nonterminal(L"Basic-Terminal");

	// Store the IDs for these nonterminals
	nt.id_parser_language         	= nt.parser_language         ->symbol();
    nt.id_toplevel_block            = nt.toplevel_block          ->symbol();
    nt.id_language_block            = nt.language_block          ->symbol();
    nt.id_language_inherits         = nt.language_inherits       ->symbol();
    nt.id_language_definition       = nt.language_definition     ->symbol();
    nt.id_lexer_symbols_definition  = nt.lexer_symbols_definition->symbol();	
    nt.id_lexer_definition          = nt.lexer_definition        ->symbol();
    nt.id_ignore_definition         = nt.ignore_definition       ->symbol();
    nt.id_keywords_definition       = nt.keywords_definition     ->symbol();
    nt.id_keyword_definition        = nt.keyword_definition      ->symbol();
    nt.id_weak_symbols_definition   = nt.weak_symbols_definition ->symbol();
    nt.id_lexeme_definition         = nt.lexeme_definition       ->symbol();
    nt.id_grammar_definition        = nt.grammar_definition      ->symbol();
    nt.id_nonterminal_definition    = nt.nonterminal_definition  ->symbol();
    nt.id_production                = nt.production              ->symbol();
    nt.id_ebnf_item                 = nt.ebnf_item               ->symbol();
    nt.id_simple_ebnf_item          = nt.simple_ebnf_item        ->symbol();
    nt.id_guard                     = nt.guard                   ->symbol();
    nt.id_nonterminal               = nt.nonterminal             ->symbol();
    nt.id_terminal                  = nt.terminal                ->symbol();
    nt.id_basic_terminal            = nt.basic_terminal          ->symbol();
    
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
    guard                   productionGuard;
    
    (*nonterminalDefinitionList.get_rule()) << nt.nonterminal_definition;
    (*orProductionList.get_rule()) << t.pipe << nt.production;
    (*ebnfItemList.get_rule()) << nt.ebnf_item;
    (*simpleEbnfItemList.get_rule()) << nt.simple_ebnf_item;
    
    (*productionGuard.get_rule()) << t.nonterminal << t.equals;
    
    ((*result) += L"Grammar-Definition") << t.grammar << t.opencurly << nonterminalDefinitionList << t.closecurly;
    
    ((*result) += L"Nonterminal-Definition") << productionGuard << t.nonterminal << t.equals << nt.production << orProductionList;
    // (Not supporting the inheritance forms of these in the bootstrap language)
    
    // Productions
    ((*result) += L"Production") << simpleEbnfItemList;
    
    ((*result) += L"Ebnf-Item") << nt.simple_ebnf_item;
    ((*result) += L"Ebnf-Item") << nt.simple_ebnf_item << t.pipe << nt.ebnf_item;
    
    ((*result) += L"Simple-Ebnf-Item") << nt.nonterminal;
    ((*result) += L"Simple-Ebnf-Item") << nt.terminal;
    ((*result) += L"Simple-Ebnf-Item") << nt.simple_ebnf_item << t.star;
    ((*result) += L"Simple-Ebnf-Item") << nt.simple_ebnf_item << t.plus;
    ((*result) += L"Simple-Ebnf-Item") << nt.simple_ebnf_item << t.question;
    ((*result) += L"Simple-Ebnf-Item") << t.openparen << ebnfItemList << t.closeparen;
    ((*result) += L"Simple-Ebnf-Item") << nt.guard;
    
    ((*result) += L"Nonterminal") << t.nonterminal;
    
    ((*result) += L"Guard") << t.openguard << ebnfItemList << t.closesquare;
    
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
    m_Builder = new lalr_builder(*m_Grammar, m_Terminals);
    
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

/// \brief Turns an AST generated by the bootstrap parser into a language definition
definition_file* bootstrap::get_definition(const util::astnode* ast) {
    // Result is null if the AST node is empty
    if (!ast) return NULL;
    
    // Should be a parser-language AST node
    if (ast->item_identifier() != nt.id_parser_language) return NULL;
    
    // Create the block
    definition_file* result = new definition_file();
    
    // One level: a toplevel block list
    if (!get_toplevel_list(result, (*ast)[0])) {
        delete result;
        return NULL;
    }
    
    return result;
}

bool bootstrap::get_toplevel_list(definition_file* file, const util::astnode* toplevel_list) {
    // Can't check the block type (we don't have the item ID for ( TopLevel-Block )*)
    if (!file)          return false;
    if (!toplevel_list) return false;
    
    // If there are no children, then there's nothing more to do
    if (toplevel_list->children().size() == 0) {
        return true;
    }
    
    // Otherwise must be 2 children
    if (toplevel_list->children().size() != 2) { 
        return false;
    }
    
    // Otherwise, this block will be (repeat, toplevel_block). ASTs are in reverse!
    if (!get_toplevel_list(file, (*toplevel_list)[0])) {
        // Error further down the list
        return false;
    }
    
    // Add the next top-level block
    toplevel_block* nextBlock = get_toplevel((*toplevel_list)[1]);
    if (!nextBlock) {
        return false;
    }
    
    // Add it to the definition file
    file->add(nextBlock);
    return true;
}

toplevel_block* bootstrap::get_toplevel(const util::astnode* toplevel) {
    // Must be a toplevel block
    if (!toplevel)                                              return NULL;
    if (toplevel->item_identifier() != nt.id_toplevel_block)    return NULL;
    
    // Can either be a language or an import block
    if (toplevel->children().size() != 1)                       return NULL;
    
    const astnode* child = (*toplevel)[0];
    if (child->item_identifier() == nt.id_language_block) {
        // Language block
        language_block* language = get_language(child);
        if (language) {
            // New toplevel block
            return new toplevel_block(language);
        } else {
            // Failed to create the block
            return NULL;
        }
    } else {
        // Note: import blocks aren't actually used by the bootstrap language at the moment
        // Unknown block type
        return NULL;
    }
}

language_block* bootstrap::get_language(const util::astnode* language) {
    // Sanity check
    if (!language)                                              return NULL;
    if (language->item_identifier() != nt.id_language_block)   return NULL;
    
    // <Language-Block>		= language identifier (<Language-Inherits>)? '{' (<Language-Definition>)* '}'
    const astnode* languageKeyword      = (*language)[0];
    const astnode* languageIdentifier   = (*language)[1];
    const astnode* inherits             = (*language)[2];
    const astnode* definitionList       = (*language)[4];
    const astnode* closeCurly           = (*language)[5];
    
    // Sanity check
    if (!languageKeyword->lexeme().item())      return NULL;
    if (!languageIdentifier->lexeme().item())   return NULL;
    if (!closeCurly->lexeme().item())           return NULL;
    
    // Start building the result
    language_block* result = new language_block(languageIdentifier->lexeme()->content<wchar_t>(), languageKeyword->lexeme()->pos(), closeCurly->lexeme()->final_pos());
    
    // Fill in the inheritence list
    if (inherits->children().size() != 0) {
        // These aren't currently used by the bootstrap language, so we ignore them for now
        // Return null so we get a test failure if we ever add these
        delete result;
        return NULL;
    }
    
    // Fill in the definition list
    if (!get_language_defn_list(result, definitionList)) {
        delete result;
        return NULL;
    }
    
    // This is the result
    return result;
}

bool bootstrap::get_language_defn_list(language_block* block, const util::astnode* defn_list) {
    // Sanity check
    if (!block)     return false;
    if (!defn_list) return false;
    
    // 0 or 2 items
    if (defn_list->children().size() == 0) {
        // Final item
        return true;
    }
    
    if (defn_list->children().size() != 2) {
        // Invalid item
        return false;
    }
    
    // Process the rest of the list
    if (!get_language_defn_list(block, (*defn_list)[0])) {
        return false;
    }
    
    // Get the item at this index
    language_unit* nextUnit = get_language_defn((*defn_list)[1]);
    if (!nextUnit) {
        return false;
    }
    
    // Add to this item
    block->add_unit(nextUnit);
    return true;
}

language_unit* bootstrap::get_language_defn(const util::astnode* defn) {
    // Sanity check
    if (!defn)                                                  return NULL;
    if (defn->item_identifier() != nt.id_language_definition)   return NULL;
    if (defn->children().size() != 1)                           return NULL;
    
    // Action depends on the type of the child node
    const astnode*  child   = (*defn)[0];
    int             childId = child->item_identifier();
    
    if (childId == nt.id_lexer_symbols_definition) {
        
    } else if (childId == nt.id_lexer_definition) {
        
    } else if (childId == nt.id_ignore_definition) {
        
    } else if (childId == nt.id_weak_symbols_definition) {
        
    } else if (childId == nt.id_keywords_definition) {
        
    } else if (childId == nt.id_grammar_definition) {
        
    }
    
    return NULL;
}
