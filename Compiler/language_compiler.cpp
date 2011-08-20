//
//  language_compiler.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <sstream>

#include "Compiler/language_compiler.h"
#include "Language/process.h"

using namespace std;
using namespace dfa;
using namespace contextfree;
using namespace compiler;
using namespace language;

/// \brief Creates a compiler that will compile the specified language block
language_compiler::language_compiler(console_container& console, const std::wstring& filename, language::language_block* block)
: compilation_stage(console, filename)
, m_Language(block) {
}

/// \brief Destructor
language_compiler::~language_compiler() {
    
}

/// \brief Compiles the language, creating the dictionary of terminals, the lexer and the grammar
void language_compiler::compile() {
    // Write out a verbose message
    cons().verbose_stream() << endl << L"== Constructing lexer NDFA" << endl;
    
    // Find any lexer-symbols sections and add them to the lexer
    for (language_block::iterator lexerSymbols = m_Language->begin(); lexerSymbols != m_Language->end(); lexerSymbols++) {
        if ((*lexerSymbols)->type() != language_unit::unit_lexer_symbols) continue;
        
        // TODO: implement me
    }
    
    // Create symbols for all of the items defined in lexer blocks
    // TODO: make it possible to redeclare literal symbols
    for (language_block::iterator lexerBlock = m_Language->begin(); lexerBlock != m_Language->end(); lexerBlock++) {
        // Fetch the lexer block
        lexer_block* lex = (*lexerBlock)->lexer_definition();
        
        // Ignore blocks that don't define lexer symbols
        if (!lex) continue;
        
        // Fetch the type
        language_unit::unit_type blockType = (*lexerBlock)->type();
        
        // Ignore lexer symbols blocks
        if (blockType == language_unit::unit_lexer_symbols) continue;
        
        // Add the symbols to the lexer
        for (lexer_block::iterator lexerItem = lex->begin(); lexerItem != lex->end(); lexerItem++) {
            // It's an error to define the same symbol twice
            if (m_Terminals.symbol_for_name((*lexerItem)->identifier()) >= 0) {
                wstringstream msg;
                msg << L"Duplicate lexer symbol: " << (*lexerItem)->identifier();
                cons().report_error(error(error::sev_error, filename(), L"DUPLICATE_LEXER_SYMBOL", msg.str(), (*lexerItem)->start_pos()));
            }
            
            // Add the symbol ID
            int symId = m_Terminals.add_symbol((*lexerItem)->identifier());
            
            // Action depends on the type of item
            switch ((*lexerItem)->get_type()) {
                case lexeme_definition::regex:
                {
                    // Remove the '/' symbols from the regex
                    wstring withoutSlashes = (*lexerItem)->definition().substr(1, (*lexerItem)->definition().size()-2);
                    
                    // Add to the NDFA
                    m_Lexer.add_regex(0, withoutSlashes, symId);
                    break;
                }
                    
                case lexeme_definition::literal:
                {
                    // Add as a literal to the NDFA
                    m_Lexer.add_literal(0, (*lexerItem)->identifier(), symId);
                    break;
                }

                case lexeme_definition::string:
                case lexeme_definition::character:
                {
                    // Add as a literal to the NDFA
                    // We can do both characters and strings here (dequote_string will work on both kinds of item)
                    m_Lexer.add_literal(0, process::dequote_string((*lexerItem)->definition()), symId);
                    break;
                }

                default:
                    // Unknown type of lexeme definition
                    cons().report_error(error(error::sev_bug, filename(), L"UNK_LEXEME_DEFINITION", L"Unhandled type of lexeme definition", (*lexerItem)->start_pos()));
                    break;
            }
            
            // Add the symbol to the set appropriate for the block type
            switch (blockType) {
                case language_unit::unit_ignore_definition:
                    // Add as an ignored symbol
                    m_IgnoredSymbols.insert(symId);
                    break;
                    
                case language_unit::unit_weak_symbols_definition:
                    // Add as a weak symbol
                    m_WeakSymbols.insert(symId);
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    // Create symbols for any items defined in the grammar
    int implicitCount = 0;
    
    for (language_block::iterator grammarBlock = m_Language->begin(); grammarBlock != m_Language->end(); grammarBlock++) {
        // Only interested in grammar blocks here
        if ((*grammarBlock)->type() != language_unit::unit_grammar_definition) continue;
        
        // Fetch the grammar block
        grammar_block* nextBlock = (*grammarBlock)->grammar_definition();
        
        // Iterate through the nonterminals
        for (grammar_block::iterator nonterminal = nextBlock->begin(); nonterminal != nextBlock->end(); nonterminal++) {
            // Iterate through the productions
            for (nonterminal_definition::iterator production = (*nonterminal)->begin(); production != (*nonterminal)->end(); production++) {
                // Iterate through the items in this production
                for (production_definition::iterator ebnfItem = (*production)->begin(); ebnfItem != (*production)->end(); ebnfItem++) {
                    implicitCount += add_ebnf_lexer_items(*ebnfItem);
                }
            }
        }
    }
    
    // Build the grammar itself
    // If we reach here, then every terminal symbol in the grammar should be defined somewhere in the terminal dictionary
    for (language_block::iterator grammarBlock = m_Language->begin(); grammarBlock != m_Language->end(); grammarBlock++) {
        // Only interested in grammar blocks here
        if ((*grammarBlock)->type() != language_unit::unit_grammar_definition) continue;
        
        // Fetch the grammar block that we're going to compile
        grammar_block* nextBlock = (*grammarBlock)->grammar_definition();
        
        // Iterate through the nonterminals
        for (grammar_block::iterator nonterminal = nextBlock->begin(); nonterminal != nextBlock->end(); nonterminal++) {
            // Get the identifier for the nonterminal that this maps to
            int nonterminalId = m_Grammar.id_for_nonterminal((*nonterminal)->identifier());
            
            // The nonterminal is already defined if there is at least one rule for it
            // It's possible that a nonterminal will get added to the grammar early if it is referenced before it is defined
            bool alreadyDefined = m_Grammar.rules_for_nonterminal(nonterminalId).size() > 0;
            
            // It's an error to use '=' definitions to redefine a nonterminal with existing rules
            if ((*nonterminal)->get_type() == nonterminal_definition::assignment && alreadyDefined) {
                wstringstream msg;
                msg << L"Duplicate nonterminal definition: " << (*nonterminal)->identifier();
                cons().report_error(error(error::sev_error, filename(), L"DUPLICATE_NONTERMINAL_DEFINITION", msg.str(), (*nonterminal)->start_pos()));
            }
            
            // The 'replace' operator should empty the existing rules
            if ((*nonterminal)->get_type() == nonterminal_definition::replace && alreadyDefined) {
                m_Grammar.rules_for_nonterminal(nonterminalId).clear();
            }
            
            // Define the productions associated with this nonterminal
            for (nonterminal_definition::iterator production = (*nonterminal)->begin(); production != (*nonterminal)->end(); production++) {
                // Get a rule for this production
                rule_container newRule(new rule(nonterminalId), true);
                
                // Append the items in this production
                for (production_definition::iterator ebnfItem = (*production)->begin(); ebnfItem != (*production)->end(); ebnfItem++) {
                    // Compile each item in turn and append them to the rule
                    compile_item(*newRule, *ebnfItem);
                }
                
                // Add the rule to the list for this nonterminal
                m_Grammar.rules_for_nonterminal(nonterminalId).push_back(newRule);
            }
        }
    }
    
    // TODO: display errors if there are any nonterminals used in the defintions that do not reference any existing item
    // TODO: display a summary of what the grammar and NDFA contains if we're in verbose mode
}

/// \brief Adds any lexer items that are defined by a specific EBNF item to this object
int language_compiler::add_ebnf_lexer_items(language::ebnf_item* item) {
    int count = 0;
    
    switch (item->get_type()) {
        case ebnf_item::ebnf_guard:
        case ebnf_item::ebnf_alternative:
        case ebnf_item::ebnf_repeat_zero:
        case ebnf_item::ebnf_repeat_one:
        case ebnf_item::ebnf_optional:
        case ebnf_item::ebnf_parenthesized:
            // Process the child items for these types of object
            for (ebnf_item::iterator childItem = item->begin(); childItem != item->end(); childItem++) {
                count += add_ebnf_lexer_items(*childItem);
            }
            break;
            
        case ebnf_item::ebnf_terminal:
        {
            // Nothing to do if this item is from another language
            if (item->source_identifier().size() > 0) {
                break;
            }
            
            // Check if this terminal already has an identifier
            if (m_Terminals.symbol_for_name(item->identifier()) >= 0) {
                // Already defined
                break;
            }
            
            // Defining literal symbols in this way produces a warning
            wstringstream warningMsg;
            warningMsg << L"Implicitly defining keyword: " << item->identifier();
            cons().report_error(error(error::sev_warning, filename(), L"IMPLICIT_LEXER_SYMBOL", warningMsg.str(), item->start_pos()));
            
            // Define a new literal string
            int symId = m_Terminals.add_symbol(item->identifier());
            m_Lexer.add_literal(0, item->identifier(), symId);
            
            // Symbols defined within the parser grammar count as weak symbols
            m_WeakSymbols.insert(symId);

            count++;
            break;
        }
            
        case ebnf_item::ebnf_terminal_character:
        case ebnf_item::ebnf_terminal_string:
        {
            // Strings and characters always create a new definition in the lexer if they don't already exist
            if (m_Terminals.symbol_for_name(item->identifier()) >= 0) {
                // Already defined in the lexer
                break;
            }
            
            // Define a new symbol
            int symId = m_Terminals.add_symbol(item->identifier());
            m_Lexer.add_literal(0, process::dequote_string(item->identifier()), symId);
            
            // Symbols defined within the parser count as weak symbols
            m_WeakSymbols.insert(symId);
            
            count++;
            break;
        }
            
        case ebnf_item::ebnf_nonterminal:
            // Nothing to do
            break;
    }
    
    // Return the count
    return count;
}


/// \brief Compiles an EBNF item from the language into a context-free grammar item
///
/// The lexer items should already be compiled before this call is made; it's a bug if any terminal items are found
/// to be missing from the terminal dictionary.
void language_compiler::compile_item(rule& rule, ebnf_item* item) {
    switch (item->get_type()) {
        case ebnf_item::ebnf_terminal:
        case ebnf_item::ebnf_terminal_character:
        case ebnf_item::ebnf_terminal_string:
        {
            // Get the ID of this terminal. We can just use the identifier supplied in the item, as it will be unique
            int terminalId = m_Terminals.symbol_for_name(item->identifier());
            
            // Add a new terminal item
            rule << item_container(new terminal(terminalId), true);
            break;
        }
            
        case ebnf_item::ebnf_nonterminal:
        {
            // Get or create the ID for this nonterminal.
            int nonterminalId = m_Grammar.id_for_nonterminal(item->identifier());
            
            // Return a new nonterminal item
            rule << item_container(new nonterminal(nonterminalId), true);
            break;
        }
            
        case ebnf_item::ebnf_parenthesized:
        {
            // Just append the items inside this one to the rule
            for (ebnf_item::iterator childItem = item->begin(); childItem != item->end(); childItem++) {
                compile_item(rule, *childItem);
            }
            break;
        }
        
        case ebnf_item::ebnf_optional:
        {
            // Compile into an optional item
            ebnf_optional* newItem = new ebnf_optional();
            compile_item(*newItem->get_rule(), (*item)[0]);
            
            // Append to the rule
            rule << item_container(newItem, true);
            break;
        }
            
        case ebnf_item::ebnf_repeat_one:
        {
            // Compile into a repeating item
            ebnf_repeating* newItem = new ebnf_repeating();
            compile_item(*newItem->get_rule(), (*item)[0]);
            
            // Append to the rule
            rule << item_container(newItem, true);
            break;
        }

        case ebnf_item::ebnf_repeat_zero:
        {
            // Compile into a repeating item
            ebnf_repeating_optional* newItem = new ebnf_repeating_optional();
            compile_item(*newItem->get_rule(), (*item)[0]);
            
            // Append to the rule
            rule << item_container(newItem, true);
            break;
        }
            
        case ebnf_item::ebnf_guard:
        {
            // Compile into a guard item
            guard* newItem = new guard();
            compile_item(*newItem->get_rule(), (*item)[0]);
            
            // Append to the rule
            rule << item_container(newItem, true);
            break;
        }
            
        case ebnf_item::ebnf_alternative:
        {
            // Compile into an alternate item
            ebnf_alternate* newItem = new ebnf_alternate();
            
            // Left-hand side
            compile_item(*newItem->get_rule(), (*item)[0]);
            
            // Right-hand side
            compile_item(*newItem->add_rule(), (*item)[1]);
            
            // Append to the rule
            rule << item_container(newItem, true);
            break;
        }

        default:
            // Unknown item type
            cons().report_error(error(error::sev_bug, filename(), L"UNKNOWN_EBNF_ITEM_TYPE", L"Unknown type of EBNF item", item->start_pos()));
            break;
    }
}
