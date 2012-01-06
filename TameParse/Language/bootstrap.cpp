//
//  bootstrap.cpp
//  Parse
//
//  Created by Andrew Hunter on 21/05/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#include <iostream>

#include "TameParse/Language/bootstrap.h"

#include "TameParse/Dfa/ndfa_regex.h"
#include "TameParse/ContextFree/item.h"
#include "TameParse/Lr/weak_symbols.h"
#include "TameParse/Lr/ignored_symbols.h"
#include "TameParse/Lr/lalr_builder.h"
#include "TameParse/Language/formatter.h"

using namespace std;
using namespace util;
using namespace dfa;
using namespace contextfree;
using namespace lr;
using namespace language;

/// \brief Redefinition of ebnf_item_attributes to abbreviate it a bit
typedef ebnf_item_attributes attributes;

#ifdef _WIN32

#include <windows.h>
#include "resource.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define ThisModule ((HINSTANCE)&__ImageBase)

/// \brief Retrieves a string containing the language definition for the parser language
const std::string& bootstrap::get_default_language_definition() {
    // Static string that will contain the language definition
    static std::string result;
    
    // Fill in the result if it's empty
    if (result.size() == 0) {
		HRSRC	definitionResource	= FindResource(ThisModule, MAKEINTRESOURCE(IDR_DEFINITION_FILE), "Text");
		DWORD	definitionSize		= SizeofResource(ThisModule, definitionResource);
		HGLOBAL	definitionHandle	= LoadResource(ThisModule, definitionResource);
		char*	definition			= (char*) LockResource(definitionHandle);

        result.assign(definition, definitionSize);
    }
    
    // Return the result
    return result;
}

#else

// Declare a string containing the language definition
#include "definition_tp.h"

/// \brief Retrieves a string containing the language definition for the parser language
const std::string& bootstrap::get_default_language_definition() {
    // Static string that will contain the language definition
    static std::string result;
    
    // Fill in the result if it's empty
    if (result.size() == 0) {
        result.assign((char*) definition_tp, definition_tp_len);
    }
    
    // Return the result
    return result;
}

#endif

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
    t.weak              = add_terminal(languageNdfa, L"weak", L"weak");
    t.ignore            = add_terminal(languageNdfa, L"ignore", L"ignore");
    t.keywords          = add_terminal(languageNdfa, L"keywords", L"keywords");
    t.parser            = add_terminal(languageNdfa, L"parser", L"parser");
    
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
    t.opensquare        = add_terminal(languageNdfa, L"'['", L"\\[");
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
    t.id_weak           = t.weak        ->symbol();
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
    t.id_opensquare     = t.opensquare  ->symbol();
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
    
    ndfa* dfa = uniqueSyms->to_dfa();
    delete uniqueSyms;
    
    ndfa* result = dfa->to_compact_dfa();
    delete dfa;
    
    return result;
}

/// \brief Creates the grammar for the language
contextfree::grammar* bootstrap::create_grammar() {
    // Create a new grammar
    grammar* result = new grammar();
    
    // Generate nonterminals
    nt.parser_language              = result->get_nonterminal(L"Parser-Language");
    nt.toplevel_block               = result->get_nonterminal(L"TopLevel-Block");
    nt.language_block               = result->get_nonterminal(L"Language-Block");
    nt.language_inherits            = result->get_nonterminal(L"Language-Inherits");
    nt.language_definition          = result->get_nonterminal(L"Language-Definition");
    nt.lexer_symbols_definition     = result->get_nonterminal(L"Lexer-Symbols-Definition");
    nt.lexer_definition             = result->get_nonterminal(L"Lexer-Definition");
    nt.ignore_definition            = result->get_nonterminal(L"Ignore-Definition");
    nt.keywords_definition          = result->get_nonterminal(L"Keywords-Definition");
    nt.keyword_definition           = result->get_nonterminal(L"Keyword-Definition");
    nt.lexeme_definition            = result->get_nonterminal(L"Lexeme-Definition");
    nt.grammar_definition           = result->get_nonterminal(L"Grammar-Definition");
    nt.nonterminal_definition       = result->get_nonterminal(L"Nonterminal-Definition");
    nt.production                   = result->get_nonterminal(L"Production");
    nt.ebnf_item                    = result->get_nonterminal(L"Ebnf-Item");
    nt.simple_ebnf_item             = result->get_nonterminal(L"Simple-Ebnf-Item");
    nt.guard                        = result->get_nonterminal(L"Guard");
    nt.nonterminal                  = result->get_nonterminal(L"Nonterminal");
    nt.terminal                     = result->get_nonterminal(L"Terminal");
    nt.basic_terminal               = result->get_nonterminal(L"Basic-Terminal");
    nt.parser_block                 = result->get_nonterminal(L"Parser-Block");
    nt.parser_startsymbol           = result->get_nonterminal(L"Parser-StartSymbol");
    nt.item_name                    = result->get_nonterminal(L"Item-Name");

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
    nt.id_parser_block              = nt.parser_block            ->symbol();
    nt.id_parser_startsymbol        = nt.parser_startsymbol      ->symbol();
    nt.id_item_name                 = nt.item_name               ->symbol();
    
    // Generate productions
    ebnf_repeating_optional listToplevel;
    (*listToplevel.get_rule()) << nt.toplevel_block;
    ((*result) += L"Parser-Language") << listToplevel;
    
    // Top level block (language)
    ((*result) += L"TopLevel-Block") << nt.language_block;
    ((*result) += L"TopLevel-Block") << nt.parser_block;
    
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
    ((*result) += L"Language-Definition") << nt.keywords_definition;
    ((*result) += L"Language-Definition") << nt.grammar_definition;
    
    // Some simple definitions
    ebnf_optional           optionalWeak;
    ebnf_repeating_optional lexemeList;
    ebnf_repeating_optional keywordDefinitionList;
    
    (*lexemeList.get_rule()) << nt.lexeme_definition;
    (*keywordDefinitionList.get_rule()) << nt.keyword_definition;
    (*optionalWeak.get_rule()) << t.weak;
    
    ((*result) += L"Lexer-Symbols-Definition") << t.lexersymbols << t.opencurly << lexemeList << t.closecurly;
    ((*result) += L"Lexer-Definition") << optionalWeak << t.lexer << t.opencurly << lexemeList << t.closecurly;
    ((*result) += L"Ignore-Definition") << t.ignore << t.opencurly << keywordDefinitionList << t.closecurly;
    ((*result) += L"Keywords-Definition") << optionalWeak << t.keywords << t.opencurly << keywordDefinitionList << t.closecurly;
    ((*result) += L"Keyword-Definition") << t.identifier;
    ((*result) += L"Keyword-Definition") << t.identifier << t.equals << t.string;
    ((*result) += L"Keyword-Definition") << t.identifier << t.equals << t.character;
    ((*result) += L"Keyword-Definition") << t.identifier << t.equals << t.regex;
    ((*result) += L"Lexeme-Definition") << t.identifier << t.equals << t.regex;
    ((*result) += L"Lexeme-Definition") << t.identifier << t.equals << t.identifier << t.dot << t.identifier;
    
    // Definitions for a grammar
    ebnf_repeating_optional nonterminalDefinitionList;
    ebnf_repeating_optional orProductionList;
    ebnf_repeating_optional ebnfItemList;
    ebnf_repeating_optional simpleEbnfItemList;
    ebnf_optional           optionalItemName;
    guard                   productionGuard;

    (*nonterminalDefinitionList.get_rule()) << nt.nonterminal_definition;
    (*orProductionList.get_rule()) << t.pipe << nt.production;
    (*ebnfItemList.get_rule()) << nt.ebnf_item;
    (*simpleEbnfItemList.get_rule()) << nt.simple_ebnf_item;
    (*optionalItemName.get_rule()) << nt.item_name;
    
    (*productionGuard.get_rule()) << t.nonterminal << t.equals;
    
    ((*result) += L"Grammar-Definition") << t.grammar << t.opencurly << nonterminalDefinitionList << t.closecurly;
    
    ((*result) += L"Nonterminal-Definition") << productionGuard << t.nonterminal << t.equals << nt.production << orProductionList;
    // (Not supporting the inheritance forms of these in the bootstrap language)

    // Named items
    ((*result) += L"Item-Name") << t.opensquare << t.identifier << t.closesquare;
    
    // Productions
    ((*result) += L"Production") << simpleEbnfItemList;
    
    ((*result) += L"Ebnf-Item") << simpleEbnfItemList;
    ((*result) += L"Ebnf-Item") << simpleEbnfItemList << t.pipe << nt.ebnf_item;
    
    ((*result) += L"Simple-Ebnf-Item") << nt.nonterminal << optionalItemName;
    ((*result) += L"Simple-Ebnf-Item") << nt.terminal << optionalItemName;
    ((*result) += L"Simple-Ebnf-Item") << nt.simple_ebnf_item << t.star << optionalItemName;
    ((*result) += L"Simple-Ebnf-Item") << nt.simple_ebnf_item << t.plus << optionalItemName;
    ((*result) += L"Simple-Ebnf-Item") << nt.simple_ebnf_item << t.question << optionalItemName;
    ((*result) += L"Simple-Ebnf-Item") << t.openparen << nt.ebnf_item << t.closeparen;
    ((*result) += L"Simple-Ebnf-Item") << nt.guard;
    
    ((*result) += L"Nonterminal") << t.nonterminal;
    
    ((*result) += L"Guard") << t.openguard << nt.ebnf_item << t.closesquare;
    
    ((*result) += L"Terminal") << nt.basic_terminal;
    ((*result) += L"Basic-Terminal") << t.identifier;
    ((*result) += L"Basic-Terminal") << t.string;
    ((*result) += L"Basic-Terminal") << t.character;

    // The parser block
    ebnf_repeating startSymbolList;

    (*startSymbolList.get_rule()) << nt.parser_startsymbol;
    ((*result) += L"Parser-Block") << t.parser << t.identifier << t.colon << t.identifier 
                                   << t.opencurly << startSymbolList << t.closecurly;
    ((*result) += L"Parser-StartSymbol") << t.nonterminal;
    
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
    weak_symbols weak(m_Grammar);
    
    // All of the keywords are weak
    item_set weaklings(m_Grammar);

    weaklings.insert(t.language);
    weaklings.insert(t.grammar);
    weaklings.insert(t.lexersymbols);
    weaklings.insert(t.lexer);
    weaklings.insert(t.ignore);
    weaklings.insert(t.weak);
    weaklings.insert(t.keywords);
    weaklings.insert(t.parser);
    
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
    m_Parser = new ast_parser(*m_Builder, &weak);
}

/// \brief Destructor
bootstrap::~bootstrap() {
    delete m_Lexer;
    delete m_Grammar;
    delete m_Builder;
    delete m_Parser;
}

/// \brief Turns an AST generated by the bootstrap parser into a language definition
definition_file_container bootstrap::get_definition(const util::astnode* ast) {
    // Result is null if the AST node is empty
    if (!ast) return definition_file_container();
    
    // Should be a parser-language AST node
    if (ast->item_identifier() != nt.id_parser_language) return definition_file_container();
    
    // Create the block
    definition_file* result = new definition_file();
    
    // One level: a toplevel block list
    if (!get_toplevel_list(result, (*ast)[0])) {
        delete result;
        return definition_file_container(NULL, true);
    }
    
    return definition_file_container(result, true);
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
    } else if (child->item_identifier() == nt.id_parser_block) {
        // Parser block
        parser_block* parser = get_parser_block(child);
        if (parser) {
            // New toplevel block
            return new toplevel_block(parser);
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

parser_block* bootstrap::get_parser_block(const util::astnode* parserBlock) {
    // Sanity check
    if (!parserBlock)                                           return NULL;
    if (parserBlock->item_identifier() != nt.id_parser_block)   return NULL;
    if (parserBlock->children().size() != 7)                    return NULL;
    
    // <Parser-Block> = parser identifier ':' identifier '{' (<Parser-StartSymbol>)+ '}'
    const astnode* parserKeyword        = (*parserBlock)[0];
    const astnode* parserIdentifier     = (*parserBlock)[1];
    const astnode* languageIdentifier   = (*parserBlock)[3];
    const astnode* startSymbolList      = (*parserBlock)[5];
    
    // Fill up the list of start symbols
    vector<wstring> startSymbols;
    
    const astnode* curNode = startSymbolList;
    while (curNode->children().size() == 2) {
        startSymbols.push_back((*(*curNode)[1])[0]->lexeme()->content<wchar_t>());
        curNode = (*curNode)[0];
    }
    startSymbols.push_back((*(*curNode)[0])[0]->lexeme()->content<wchar_t>());
    
    // Create the result
    return new parser_block(parserIdentifier->lexeme()->content<wchar_t>(), languageIdentifier->lexeme()->content<wchar_t>(), startSymbols, parserKeyword->lexeme()->pos(), (*parserBlock)[6]->lexeme()->final_pos());
}

language_block* bootstrap::get_language(const util::astnode* language) {
    // Sanity check
    if (!language)                                              return NULL;
    if (language->item_identifier() != nt.id_language_block)    return NULL;
    
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
        // Lexer symbols block
        lexer_block* result = new lexer_block(false, false, false, (*child)[0]->lexeme()->pos(), (*child)[3]->lexeme()->final_pos());
        
        if (!get_lexer_block(result, (*child)[2])) {
            delete result;
            return NULL;
        }
        
        return new language_unit(language_unit::unit_lexer_symbols, result);
    } else if (childId == nt.id_lexer_definition) {
        // Lexer definition block
        lexeme_container    lexerKw         = (*child)[1]->lexeme();
        lexeme_container    closingCurly    = (*child)[4]->lexeme();
        bool                isWeak          = (*child)[0]->children().size() > 0;
        
        lexer_block* result = new lexer_block(isWeak, false, false, lexerKw->pos(), closingCurly->final_pos());
        
        if (!get_lexer_block(result, (*child)[3])) {
            delete result;
            return NULL;
        }
        
        return new language_unit(language_unit::unit_lexer_definition, result);
    } else if (childId == nt.id_ignore_definition) {
        // Ignore definition block
        lexer_block* result = new lexer_block(false, false, false, (*child)[0]->lexeme()->pos(), (*child)[3]->lexeme()->final_pos());
        
        if (!get_lexer_block(result, (*child)[2])) {
            delete result;
            return NULL;
        }
        
        return new language_unit(language_unit::unit_ignore_definition, result);
    } else if (childId == nt.id_keywords_definition) {
        // Keywords definition block
        lexeme_container    lexerKw         = (*child)[1]->lexeme();
        lexeme_container    closingCurly    = (*child)[4]->lexeme();
        bool                isWeak          = (*child)[0]->children().size() > 0;
        
        lexer_block* result = new lexer_block(isWeak, false, false, lexerKw->pos(), closingCurly->final_pos());
        
        if (!get_lexer_block(result, (*child)[3])) {
            delete result;
            return NULL;
        }
        
        return new language_unit(language_unit::unit_keywords_definition, result);
    } else if (childId == nt.id_grammar_definition) {
        // Grammar block
        grammar_block* result = new grammar_block((*child)[0]->lexeme()->pos(), (*child)[3]->lexeme()->final_pos());
        
        if (!get_grammar_block(result, (*child)[2])) {
            delete result;
            return NULL;
        }
        
        return new language_unit(result);
    }
    
    return NULL;
}

bool bootstrap::get_lexer_block(lexer_block* block, const util::astnode* defn) {
    // Sanity check
    if (!defn)                          return false;
    
    // Reached the end of this block if the number of child items is 0
    if (defn->children().size() == 0)   return true;
    
    // If the size is 2, then this is a repeating item
    const astnode* dataItem         = NULL;
    const astnode* preceedingItem   = NULL;
    
    if (defn->children().size() == 1) {
        dataItem        = (*defn)[0];
    } else if (defn->children().size() == 2) {
        preceedingItem  = (*defn)[0];
        dataItem        = (*defn)[1];
    } else {
        return false;
    }
    
    // Populate the block with any items that preceed this one
    if (preceedingItem) {
        if (!get_lexer_block(block, preceedingItem)) {
            return false;
        }
    }
    
    // Deal with the data item
    int dataItemId = dataItem->item_identifier();
    
    if (dataItemId == nt.id_lexeme_definition) {
        // Lexeme definition
        const astnode* lexemeIdentifier = (*dataItem)[0];
        
        if (dataItem->children().size() == 3) {
            // <Lexeme-Definition> = identifier '=' regex
            const astnode* regex = (*dataItem)[2];
            
            lexeme_definition* newLexeme = new lexeme_definition(lexeme_definition::regex, 
                                                                 lexemeIdentifier->lexeme()->content<wchar_t>(), 
                                                                 regex->lexeme()->content<wchar_t>(), 
                                                                 false, // Not supported in the bootstrap language
                                                                 false, // Not supported in the bootstrap language
                                                                 lexemeIdentifier->lexeme()->pos(), 
                                                                 regex->lexeme()->final_pos(),
                                                                 regex->lexeme()->pos());

            block->add_definition(newLexeme);
            return true;
        } else if (dataItem->children().size() == 5) {
            // <Lexeme-Definition> = identifier '=' identifier '.' identifier
            return false;       // Not supported yet
        } else {
            // Unknown
            return false;
        }
    }
    
    else if (dataItemId == nt.id_keyword_definition) {
        // Keyword definition
        const astnode* keywordIdentifier = (*dataItem)[0];
        
        if (dataItem->children().size() == 1) {
            // Just an identifier
            lexeme_definition* newKeyword = new lexeme_definition(lexeme_definition::literal,
                                                                  keywordIdentifier->lexeme()->content<wchar_t>(),
                                                                  keywordIdentifier->lexeme()->content<wchar_t>(),
                                                                  false, // Not supported in the bootstrap language
                                                                  false, // Not supported in the bootstrap language
                                                                  keywordIdentifier->lexeme()->pos(),
                                                                  keywordIdentifier->lexeme()->final_pos(),
                                                                  keywordIdentifier->lexeme()->pos());

            block->add_definition(newKeyword);
            return true;
        } else if (dataItem->children().size() == 3) {
            // identifier '=' string/character/regex
            const astnode*          defn    = (*dataItem)[2];
            lexeme_definition::type type    = lexeme_definition::string;
            
            if (defn->lexeme()->matched() == t.id_string) {
                type = lexeme_definition::string;
            } else if (defn->lexeme()->matched() == t.id_character) {
                type = lexeme_definition::character;
            } else if (defn->lexeme()->matched() == t.id_regex) {
                type = lexeme_definition::regex;
            } else {
                return false;
            }
            
            lexeme_definition* newKeyword = new lexeme_definition(type,
                                                                  keywordIdentifier->lexeme()->content<wchar_t>(),
                                                                  defn->lexeme()->content<wchar_t>(),
                                                                  false, // Not supported in the bootstrap language
                                                                  false, // Not supported in the bootstrap language
                                                                  keywordIdentifier->lexeme()->pos(),
                                                                  defn->lexeme()->final_pos(),
                                                                  defn->lexeme()->pos());
            
            block->add_definition(newKeyword);
            return true;               // Not supported
        } else {
            // Unknown
            return false;
        }
    }
    
    else if (dataItem->lexeme().item() && dataItem->lexeme()->matched() == t.id_identifier) {
        // Literal identifier (in ignore list): works like keyword
        lexeme_definition* newKeyword = new lexeme_definition(lexeme_definition::literal,
                                                              dataItem->lexeme()->content<wchar_t>(),
                                                              dataItem->lexeme()->content<wchar_t>(),
                                                              false, // Not supported in the bootstrap language
                                                              false, // Not supported in the bootstrap language
                                                              dataItem->lexeme()->pos(),
                                                              dataItem->lexeme()->final_pos(),
                                                              dataItem->lexeme()->pos());
        
        block->add_definition(newKeyword);
        return true;
    }
    
    // Unknown
    return false;
}

bool bootstrap::get_grammar_block(grammar_block* block, const util::astnode* nonterminal_list) {
    // Sanity check
    if (nonterminal_list == NULL)       return false;
    
    // If the list has no children, then we've reached the end
    if (nonterminal_list->children().size() == 0) return true;
    
    // The list should have 2 children now
    if (nonterminal_list->children().size() != 2) return false;
    
    // Recurse on the left-hand side
    if (!get_grammar_block(block, (*nonterminal_list)[0])) {
        return false;
    }
    
    // RHS = <NonTerminal>
    // The bootstrap language only supports <NonTerminal> = [=>] nonterminal '=' <Production> (| <Production)*
    const astnode* nonterminal          = (*nonterminal_list)[1];
    const astnode* ntIdentifier         = (*nonterminal)[1];
    const astnode* ntFirstProduction    = (*nonterminal)[3];
    const astnode* ntMoreProductions    = (*nonterminal)[4];
    
    // Create the new nonterminal definition
    nonterminal_definition* newDefn = new nonterminal_definition(nonterminal_definition::assignment, ntIdentifier->lexeme()->content<wchar_t>());
    
    // Process the initial production
    production_definition* initialProduction = get_production_definition(ntFirstProduction);
    if (initialProduction == NULL) {
        delete newDefn;
        return false;
    }
    
    newDefn->add_production(initialProduction);
    
    // Process the production list
    if (!get_production_list(newDefn, ntMoreProductions)) {
        delete newDefn;
        return false;
    }
    
    // Productions all OK: add and return
    block->add_nonterminal(newDefn);
    return true;
}

bool bootstrap::get_production_list(nonterminal_definition* nonterm, const util::astnode* production_list) {
    // Sanity check
    if (!production_list)   return false;
    
    // Finished if there are no child nodes
    if (production_list->children().size() == 0) return true;
    
    // Should be three nodes
    if (production_list->children().size() != 3) return false;
    
    // Process the list continuation
    if (!get_production_list(nonterm, (*production_list)[0])) {
        return false;
    }
    
    // Process the next production
    production_definition* nextDefn = get_production_definition((*production_list)[2]);
    
    if (nextDefn == NULL) {
        return false;
    }
    
    nonterm->add_production(nextDefn);
    return true;
}

production_definition* bootstrap::get_production_definition(const util::astnode* production_defn) {
    // Sanity check
    if (production_defn == NULL)                                return NULL;
    if (production_defn->item_identifier() != nt.id_production) return NULL;
    
    // A production is just a list of EBNF items
    production_definition* newProd = new production_definition();
    
    if (!get_ebnf_list(newProd, (*production_defn)[0])) {
        delete newProd;
        return NULL;
    }
    
    return newProd;
}

bool bootstrap::get_ebnf_list(production_definition* defn, const util::astnode* ebnf_item_list) {
    // Sanity check
    if (ebnf_item_list == NULL)         return false;
    
    // Finished once the list is empty
    if (ebnf_item_list->children().size() == 0) {
        return true;
    }
    
    // Should be two items
    if (ebnf_item_list->children().size() != 2) {
        return false;
    }
    
    // Process the LHS of the definition
    if (!get_ebnf_list(defn, (*ebnf_item_list)[0])) {
        return false;
    }
    
    // Process the RHS of the definition
    ebnf_item* newItem = get_ebnf_item((*ebnf_item_list)[1], NULL);
    if (newItem == NULL) {
        return false;
    }
    
    // Add this item
    defn->add_item(newItem);
    return true;
}

bool bootstrap::get_ebnf_list(ebnf_item* defn, const util::astnode* ebnf_item_list) {
    // Sanity check
    if (ebnf_item_list == NULL)         return false;
    
    // Finished once the list is empty
    if (ebnf_item_list->children().size() == 0) {
        return true;
    }
    
    // Should be two items
    if (ebnf_item_list->children().size() != 2) {
        return false;
    }
    
    // Process the LHS of the definition
    if (!get_ebnf_list(defn, (*ebnf_item_list)[0])) {
        return false;
    }
    
    // Process the RHS of the definition
    ebnf_item* newItem = get_ebnf_item((*ebnf_item_list)[1], NULL);
    if (newItem == NULL) {
        return false;
    }
    
    // Add this item
    defn->add_child(newItem);
    return true;
}

ebnf_item* bootstrap::get_ebnf_item(const util::astnode* ebnf, const util::astnode* optionalItemName) {
    // Sanity check
    if (ebnf == NULL) return NULL;

    // The name of this item (empty by default)
    wstring name;

    // Get the name from the optional item name if it was passed in
    if (optionalItemName) {
        // If this has children, then we matched the item: the format is '[ identifier ]', so retrieve the content of the identifier lexeme
        if (optionalItemName->children().size() == 1) {
            name = (*(*optionalItemName)[0])[1]->lexeme()->content<wchar_t>();
        }
    }
    
    // Action depends on the type of this node
    int nodeType = ebnf->item_identifier();
    
    if (nodeType == nt.id_ebnf_item) {
        // First item is always a simple item list
        ebnf_item* leftHandSide = new ebnf_item(ebnf_item::ebnf_parenthesized, attributes(name));
        
        if (!get_ebnf_list(leftHandSide, (*ebnf)[0])) {
            delete leftHandSide;
            return NULL;
        }
        
        // Might be of the form (a | b)
        if (ebnf->children().size() == 3) {
            // An alternative item
            ebnf_item* alternative = get_ebnf_item((*ebnf)[2], NULL);
            
            if (alternative == NULL) {
                delete leftHandSide;
                return NULL;
            }
            
            // Generate the final result
            ebnf_item* newItem = new ebnf_item(ebnf_item::ebnf_alternative, attributes(name));
            newItem->add_child(leftHandSide);
            newItem->add_child(alternative);
            
            return newItem;
        } else {
            // Just the item list
            return leftHandSide;
        }
    }
    
    else if (nodeType == nt.id_simple_ebnf_item) {
        // Might be a simple nonterminal, terminal, one of the various closures, a parenthesized list or a guard
        if (ebnf->children().size() == 1) {
            return get_ebnf_item((*ebnf)[0], NULL);
        } 

        else if (ebnf->children().size() == 2) {
            // Nonterminal or terminal or guard
            return get_ebnf_item((*ebnf)[0], (*ebnf)[1]);
        } 
        
        else if (ebnf->children().size() == 3 && (!(*ebnf)[0]->lexeme() || (*ebnf)[0]->lexeme()->matched() != t.id_openparen)) {
            // Closure of some variety
            ebnf_item* closedItem = get_ebnf_item((*ebnf)[0], NULL);
            
            if (closedItem == NULL) {
                return NULL;
            }

            // Get the name from the optional item name if it was passed in
            if ((*ebnf)[2]->children().size() == 1) {
                name = (*(*(*ebnf)[2])[0])[1]->lexeme()->content<wchar_t>();
            }
            
            // Closure depends on the following symbol
            int closureSymbol = (*ebnf)[1]->lexeme()->matched();

            if (closureSymbol == t.id_star) {
                // 0 or more (Kleene star)
                ebnf_item* newItem = new ebnf_item(ebnf_item::ebnf_repeat_zero, attributes(name));
                newItem->add_child(closedItem);
                return newItem;
            } else if (closureSymbol == t.id_plus) {
                // 1 or more
                ebnf_item* newItem = new ebnf_item(ebnf_item::ebnf_repeat_one, attributes(name));
                newItem->add_child(closedItem);
                return newItem;
            } else if (closureSymbol == t.id_question) {
                // 0 or 1
                ebnf_item* newItem = new ebnf_item(ebnf_item::ebnf_optional, attributes(name));
                newItem->add_child(closedItem);
                return newItem;
            } else {
                // Unknown type of thing
                delete closedItem;
                return NULL;
            }
        } 
        
        else if (ebnf->children().size() == 3) {
            // Parenthesized list
            // Just contains an EBNF item: process that separately
            return get_ebnf_item((*ebnf)[1], NULL);
        }
        
        else {
            // Unknown simple item type
            return NULL;
        }
    }
    
    else if (nodeType == nt.id_nonterminal) {
        // Just a nonterminal
        lexeme_container nt = (*ebnf)[0]->lexeme();
        return new ebnf_item(ebnf_item::ebnf_nonterminal, L"", nt->content<wchar_t>(), attributes(name), nt->pos(), nt->final_pos());
    }
    
    else if (nodeType == nt.id_terminal) {
        // Can only contain a basic_terminal
        return get_ebnf_item((*ebnf)[0], optionalItemName);
    }
    
    else if (nodeType == nt.id_basic_terminal) {
        // String, identifier or character
        const astnode*  terminal    = (*ebnf)[0];
        ebnf_item::type itemType    = ebnf_item::ebnf_terminal;
        int             lexemeType  = terminal->lexeme()->matched();
        
        if (lexemeType == t.id_identifier) {
            itemType = ebnf_item::ebnf_terminal;
        } else if (lexemeType == t.id_string) {
            itemType = ebnf_item::ebnf_terminal_string;
        } else if (lexemeType == t.id_character) {
            itemType = ebnf_item::ebnf_terminal_character;
        } else {
            return NULL;
        }
        
        lexeme_container term = terminal->lexeme();
        return new ebnf_item(itemType, L"", term->content<wchar_t>(), attributes(name), term->pos(), term->final_pos());
    }
    
    else if (nodeType == nt.id_guard) {
        // Guard (of the form [=> ebnf_item ]
        ebnf_item* newItem      = new ebnf_item(ebnf_item::ebnf_guard, attributes(name));
        ebnf_item* guardContent = get_ebnf_item((*ebnf)[1], NULL);
        
        if (guardContent == NULL) {
            delete newItem;
            return NULL;
        }
        
        // Add the guard content
        // TODO: could add the children of the guard content here (doesn't make a lot of difference to the end result, though)
        newItem->add_child(guardContent);
        
        return newItem;
    }
    
    // Unknown item type
    return NULL;
}
