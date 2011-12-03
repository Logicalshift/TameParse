//
//  language_stage.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <sstream>

#include "TameParse/Compiler/language_stage.h"
#include "TameParse/Language/process.h"
#include "TameParse/Language/formatter.h"

using namespace std;
using namespace dfa;
using namespace contextfree;
using namespace language;
using namespace compiler;

/// \brief Creates a compiler that will compile the specified language block
language_stage::language_stage(console_container& console, const std::wstring& filename, const language::language_block* block, const import_stage* importStage)
: compilation_stage(console, filename)
, m_Language(block)
, m_Import(importStage)
, m_InheritsFrom(NULL) {
}

/// \brief Destructor
language_stage::~language_stage() {
    // Destroy the cached filenames
    for (map<wstring, wstring*>::iterator filename = m_Filenames.begin(); filename != m_Filenames.end(); filename++) {
        delete filename->second;
    }
    m_Filenames.clear();
    
    // Destroy the inherited stage
    if (m_InheritsFrom) {
        delete m_InheritsFrom;
    }
}

/// \brief Removes any terminal symbols used in the specified rule from the unused list
void language_stage::process_rule_symbols(const contextfree::rule& rule) {
    // Iterate through the rules in this item
    for (rule::iterator item = rule.begin(); item != rule.end(); item++) {
        // Remove terminal items from the unused list
        if ((*item)->type() == item::terminal) {
            m_UnusedSymbols.erase((*item)->symbol());

            // Remember which ignored symbols are sometimes used in the language
            if (m_IgnoredSymbols.find((*item)->symbol()) != m_IgnoredSymbols.end()) {
                m_UsedIgnoredSymbols.insert((*item)->symbol());
            }
        }

        // Recurse into EBNF items
        const ebnf* ebnfItem = (*item)->cast_ebnf();
        if (ebnfItem) {
            // Also remove items for any contained EBNF rules
            for (ebnf::rule_iterator ebnfRule = ebnfItem->first_rule(); ebnfRule != ebnfItem->last_rule(); ebnfRule++) {
                process_rule_symbols(**ebnfRule);
            }
        }
    }
}

/// \brief Compiles the language, creating the dictionary of terminals, the lexer and the grammar
void language_stage::compile() {
#ifndef TAMEPARSE_BOOTSTRAP
    // If this language inherits from another, then try to import it and if it exists, compile it first
    if (!m_Language->inherits().empty()) {
        // The language block supports multiple items to inherit from, but this stage will only compile the first one
        const wstring& inheritFrom = m_Language->inherits().front();

        // Fetch the language from the importer
        // TODO: inheritance loops should produce an error (other than a stack overflow)
        const language_block* inheritBlock = m_Import->language_with_name(inheritFrom);

        // Report an error if the language is not defined
        if (!inheritBlock) {
            wstringstream msg;
            msg << L"Unable to find language '" << inheritFrom << "'";
            cons().report_error(error(error::sev_error, filename(), L"CANT_FIND_LANGUAGE", msg.str(), m_Language->start_pos()));
        } else {
            // Compile the language this inherits from
            console_container consCopy(cons_container());
            
            if (m_InheritsFrom) delete m_InheritsFrom;
            m_InheritsFrom = new language_stage(consCopy, m_Import->file_with_language(inheritFrom), inheritBlock, m_Import);
            m_InheritsFrom->compile();

            // Merge with this language
            m_InheritsFrom->export_to(this);
        }
    }
#else
    // Importing not supported in the bootstrapper
    if (!m_Language->inherits().empty()) {
        cons().report_error(error(error::sev_error, filename(), L"CANT_INHERIT_WHEN_BOOTSTRAPPING", L"Inheritance is not supported in the bootstrapper", m_Language->start_pos()));
    }
#endif

    // Write out a verbose message
    cons().verbose_stream() << L"  = Constructing lexer and grammar for " << m_Language->identifier() << endl;

    // Find/create a filename for this object
    wstring* ourFilename;
    if (m_Filenames.find(filename()) != m_Filenames.end()) {
        ourFilename = m_Filenames[filename()];
    } else {
        ourFilename = new wstring(filename());
        m_Filenames[filename()] = ourFilename;
    }
    
    // Find any lexer-symbols sections and add them to the lexer
    for (language_block::iterator lexerSymbols = m_Language->begin(); lexerSymbols != m_Language->end(); lexerSymbols++) {
        if ((*lexerSymbols)->type() != language_unit::unit_lexer_symbols) continue;

        // Fetch the lexer block
        lexer_block* lex = (*lexerSymbols)->any_lexer_block();
        
        // Ignore blocks that don't define lexer symbols
        if (!lex) continue;

        // Define these as expressions
        for (lexer_block::iterator lexerItem = lex->begin(); lexerItem != lex->end(); lexerItem++) {
            // Check if an expression is already defined and report an error if it is
            if (!m_Lexer.get_expressions((*lexerItem)->identifier()).empty()) {
                if (!(*lexerItem)->add_to_definition() && !(*lexerItem)->replace_definition()) {
                    // Symbol is already defined, and isn't set up to modify an existing symbol
                    wstringstream msg;
                    msg << L"Duplicate lexer symbol: " << (*lexerItem)->identifier();
                    cons().report_error(error(error::sev_error, filename(), L"DUPLICATE_LEXER_SYMBOL", msg.str(), (*lexerItem)->start_pos()));
                }
            } else {
                if ((*lexerItem)->add_to_definition()) {
                    // Can't use |= with a symbol that is not defined
                    wstringstream msg;
                    msg << L"Cannot add definitions to nonexistent symbol: " << (*lexerItem)->identifier();
                    cons().report_error(error(error::sev_error, filename(), L"MISSING_LEXER_SYMBOL_FOR_ADDING", msg.str(), (*lexerItem)->start_pos()));
                } else if ((*lexerItem)->replace_definition()) {
                    // Can't replace a nonexistent symbol
                    wstringstream msg;
                    msg << L"Cannot replace nonexistent symbol: " << (*lexerItem)->identifier();
                    cons().report_error(error(error::sev_error, filename(), L"MISSING_LEXER_SYMBOL_FOR_REPLACING", msg.str(), (*lexerItem)->start_pos()));
                }
            }

            // Check if this definition should replace another
            if ((*lexerItem)->replace_definition()) {
                // Remove the existing expression
                m_Lexer.remove_expression((*lexerItem)->identifier());
            }

            // Action depends on the type of item
            switch ((*lexerItem)->get_type()) {
                case lexeme_definition::regex:
                {
                    // Remove the '/' symbols from the regex
                    wstring withoutSlashes = (*lexerItem)->definition().substr(1, (*lexerItem)->definition().size()-2);
                    
                    // Add to the lexer
                    m_Lexer.add_expression((*lexerItem)->identifier(), lexer_item(lexer_item::regex, withoutSlashes, lex->is_case_insensitive(), ourFilename, (*lexerItem)->definition_pos()));
                    break;
                }
                    
                case lexeme_definition::literal:
                {
                    // Add as a literal to the lexer
                    m_Lexer.add_expression((*lexerItem)->identifier(), lexer_item(lexer_item::literal, (*lexerItem)->identifier(), lex->is_case_insensitive(), ourFilename, (*lexerItem)->definition_pos()));
                    break;
                }

                case lexeme_definition::string:
                case lexeme_definition::character:
                {
                    // Add as a literal to the lexer
                    // We can do both characters and strings here (dequote_string will work on both kinds of item)
                    m_Lexer.add_expression((*lexerItem)->identifier(), lexer_item(lexer_item::literal, process::dequote_string((*lexerItem)->definition()), lex->is_case_insensitive(), ourFilename, (*lexerItem)->definition_pos()));
                    break;
                }

                default:
                    // Unknown type of lexeme definition
                    cons().report_error(error(error::sev_bug, filename(), L"BUG_UNK_LEXEME_DEFINITION", L"Unhandled type of lexeme definition", (*lexerItem)->start_pos()));
                    break;
            }
        }
    }
    
    // Order that lexer blocks should be processed in (the priority of the symbols)
    language_unit::unit_type lexerDefinitionOrder[] = { 
            language_unit::unit_keywords_definition, 
            language_unit::unit_lexer_definition, 
            language_unit::unit_ignore_definition,
        
            language_unit::unit_null
        };
    
    // TODO: make it possible to redeclare literal symbols

    // Process each of the lexer block types in the appropriate order
    // This is slightly redundant; it's probably better to prioritise the lexer actions based on the type rather than the
    // terminal ID (like this assumes). This will nearly work for these terminal IDs, but will fail on strings and characters
    // defined in the grammar itself (as the grammar must be processed last)
    for (int weakness = 0; weakness < 2; weakness++) {
        // Weak blocks have the highest priority
        bool isWeak = weakness == 0;

        // Process the remaining blocks in order
        for (language_unit::unit_type* thisType = lexerDefinitionOrder; *thisType != language_unit::unit_null; thisType++) {
            // Create symbols for all of the items defined in lexer blocks
            for (language_block::iterator lexerBlock = m_Language->begin(); lexerBlock != m_Language->end(); lexerBlock++) {
                // Fetch the lexer block
                lexer_block* lex = (*lexerBlock)->any_lexer_block();
                
                // Ignore blocks that don't define lexer symbols
                if (!lex) continue;
                
                // Fetch the type
                language_unit::unit_type blockType = (*lexerBlock)->type();
                
                // Process only the block types that belong in this pass
                if (blockType != *thisType || lex->is_weak() != isWeak) continue;
                
                // Add the symbols to the lexer
                for (lexer_block::iterator lexerItem = lex->begin(); lexerItem != lex->end(); lexerItem++) {
                    // Get the ID that we'll define for this symbol
                    int symId;
                    
                    if (!(*lexerItem)->add_to_definition()) {
                        if (!(*lexerItem)->replace_definition()) {
                            // It's an error to define the same symbol twice
                            if (m_Terminals.symbol_for_name((*lexerItem)->identifier()) >= 0) {
                                wstringstream msg;
                                msg << L"Duplicate lexer symbol: " << (*lexerItem)->identifier();
                                cons().report_error(error(error::sev_error, filename(), L"DUPLICATE_LEXER_SYMBOL", msg.str(), (*lexerItem)->start_pos()));
                            }
                            
                            // Add the symbol ID
                            symId = m_Terminals.add_symbol((*lexerItem)->identifier());
                        } else {
                            // Replacing an existing definition
                            symId = m_Terminals.symbol_for_name((*lexerItem)->identifier());

                            if (symId < 0) {
                                // It's an error for the symbol not to exist
                                wstringstream msg;
                                msg << L"Cannot replace nonexistent symbol: " << (*lexerItem)->identifier();
                                cons().report_error(error(error::sev_error, filename(), L"MISSING_LEXER_SYMBOL_FOR_REPLACING", msg.str(), (*lexerItem)->start_pos()));
                            } else {
                                // Clear the existing symbol
                                m_TerminalDefinition.erase(symId);
                                m_Lexer.remove_definition((*lexerItem)->identifier());
                            }
                        }
                    } else {
                        // Get the existing symbol ID
                        symId = m_Terminals.symbol_for_name((*lexerItem)->identifier());
                        
                        // It's an error to try use the |= operator with a symbol that does not have a primary definition
                        if (symId < 0) {
                            wstringstream msg;
                            msg << L"Cannot add definitions to nonexistent symbol: " << (*lexerItem)->identifier();
                            cons().report_error(error(error::sev_error, filename(), L"MISSING_LEXER_SYMBOL_FOR_ADDING", msg.str(), (*lexerItem)->start_pos()));
                        }
                        
                        // It's an error to try to add items to a symbol of a different type
                        else if (m_TypeForTerminal[symId] != blockType) {
                            wstringstream msg;
                            msg << L"Cannot add definitions to a symbol defined in a different lexer block: " << (*lexerItem)->identifier();
                            cons().report_error(error(error::sev_error, filename(), L"CANNOT_ADD_TO_DIFFERENT_LEXER_SYMBOL_TYPE", msg.str(), (*lexerItem)->start_pos()));
                        }
                    }

                    // Can't do any more if we haven't got a valid symbol ID
                    if (symId < 0) {
                        continue;
                    }
                    
                    // Set the type of this symbol
                    m_TypeForTerminal[symId] = blockType;
                    
                    // Mark it as unused provided that we're not defining the ignored symbols (which are generally unused by definition)
                    if (blockType != language_unit::unit_ignore_definition) {
                        m_UnusedSymbols.insert(symId);
                    }
                    
                    // Store where it is defined
                    if (m_TerminalDefinition.find(symId) == m_TerminalDefinition.end()) {
                        m_TerminalDefinition[symId] = pair<block*, wstring*>(*lexerItem, ourFilename);
                    }

                    // Set whether the regexes or literals we add should be case insensitive
                    bool ci = lex->is_case_insensitive();
                    
                    // Action depends on the type of item
                    switch ((*lexerItem)->get_type()) {
                        case lexeme_definition::regex:
                        {
                            // Remove the '/' symbols from the regex
                            wstring withoutSlashes = (*lexerItem)->definition().substr(1, (*lexerItem)->definition().size()-2);
                            
                            // Add to the lexer
                            m_Lexer.add_definition((*lexerItem)->identifier(), lexer_item(lexer_item::regex, withoutSlashes, ci, symId, blockType, isWeak, ourFilename, (*lexerItem)->definition_pos()));
                            break;
                        }
                            
                        case lexeme_definition::literal:
                        {
                            // Add as a literal to the lexer
                            m_Lexer.add_definition((*lexerItem)->identifier(), lexer_item(lexer_item::literal, (*lexerItem)->identifier(), ci, symId, blockType, isWeak, ourFilename, (*lexerItem)->definition_pos()));
                            break;
                        }

                        case lexeme_definition::string:
                        case lexeme_definition::character:
                        {
                            // Add as a literal to the lexer
                            // We can do both characters and strings here (dequote_string will work on both kinds of item)
                            wstring dequoted = process::dequote_string((*lexerItem)->definition());
                            m_Lexer.add_definition((*lexerItem)->identifier(), lexer_item(lexer_item::literal, dequoted, ci, symId, blockType, isWeak, ourFilename, (*lexerItem)->definition_pos()));
                            break;
                        }

                        default:
                            // Unknown type of lexeme definition
                            cons().report_error(error(error::sev_bug, filename(), L"BUG_UNK_LEXEME_DEFINITION", L"Unhandled type of lexeme definition", (*lexerItem)->start_pos()));
                            break;
                    }
                    
                    // Add the symbol to the set appropriate for the block type
                    switch (blockType) {
                        case language_unit::unit_ignore_definition:
                            // Add as an ignored symbol
                            m_IgnoredSymbols.insert(symId);
                            break;
                            
                        default:
                            break;
                    }

                    if (lex->is_weak()) {
                        m_WeakSymbols.insert(symId);
                    }
                }
            }
        }
    }
    
    // Create symbols for any items defined in the grammar (these are all weak, so they must be defined first)
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
                    compile_item(*newRule, *ebnfItem, ourFilename);
                }
                
                // Add the rule to the list for this nonterminal
                m_Grammar.rules_for_nonterminal(nonterminalId).push_back(newRule);
                
                // Remember where this rule was defined
                m_RuleDefinition[newRule->identifier(m_Grammar)] = block_file(*production, ourFilename);
            }
        }
    }

    // Go through the grammar and remove any terminal symbols that are used in any of the productions
    for (int itemId = 0; itemId < m_Grammar.max_item_identifier(); itemId++) {
        // Get the rules for this item
        const rule_list& itemRules = m_Grammar.rules_for_nonterminal(itemId);

        // Remove any unused symbol from the list
        for (rule_list::const_iterator itemRule = itemRules.begin(); itemRule != itemRules.end(); itemRule++) {
            process_rule_symbols(**itemRule);
        }
    }
    
    // Any nonterminal with no rules is one that was referenced but not defined
    for (int nonterminalId = 0; nonterminalId < m_Grammar.max_item_identifier(); nonterminalId++) {
        // Get the item corresponding to this ID
        item_container ntItem = m_Grammar.item_with_identifier(nonterminalId);
        if (ntItem->type() != item::nonterminal) continue;
        
        // This nonterminal ID is unused if it has no rules
        if (m_Grammar.rules_for_nonterminal(nonterminalId).size() == 0) {
            // Find the place where this nonterminal was first used
            block*          firstUsage  = m_FirstNonterminalUsage[nonterminalId].first;
            position        usagePos    = firstUsage?firstUsage->start_pos():position(-1, -1, -1);
            
            // Generate the message
            wstringstream   msg;
            msg << L"Undefined nonterminal: " << formatter::to_string(*ntItem, m_Grammar, m_Terminals);
            
            cons().report_error(error(error::sev_error, filename(), L"UNDEFINED_NONTERMINAL", msg.str(), usagePos));
        }
    }

    // Any guard that has epsilon in the first set for its rule invalidates the grammar
    // These guards are always accepted, so their alternatives are never evaluated, so
    // they are almost certainly not what the user meant to do. As an exception, guards that 
    // have completely empty rules (ie are defined as [=>]) are allowed. These can specify
    // the resolution of a shift/reduce or reduce/reduce conflict (though we warn about 
    // these if an option isn't set)
    item_container emptyItem(new empty_item());
    int emptyId = m_Grammar.identifier_for_item(emptyItem);

    for (map<item_container, position_file>::const_iterator checkGuard = m_Guards.begin(); checkGuard != m_Guards.end(); ++checkGuard) {
        // Get the guard item
        const guard* theGuard = checkGuard->first->cast_guard();
        if (!theGuard) continue;

        // Get the first set for the rule
        item_set firstSet = m_Grammar.first_for_rule(*theGuard->get_rule());

        // Could be a problem if it contains epsilon
        if (firstSet.contains(emptyId)) {
            if (theGuard->get_rule()->items().empty()) {
                // The empty guard is just a warning
                if (cons().get_option(L"allow-empty-guards").empty()) {
                    cons().report_error(error(error::sev_warning, *checkGuard->second.second, L"EMPTY_GUARD", L"Empty guards will always be accepted and will supress alternative meanings of an expression", checkGuard->second.first));
                }
            } else {
                // Guards with content that evaluate to empty have no effect and are an error
                wstringstream msg;
                msg << L"Guard '" << formatter::to_string(*theGuard, m_Grammar, m_Terminals) << "' can evaluate to the empty string and will supress other meanings of any rule it is encountered in.";
                cons().report_error(error(error::sev_error, *checkGuard->second.second, L"INEFFECTIVE_GUARD", msg.str(), checkGuard->second.first));
            }
        }
    }

    // Display a summary of what the grammar and lexer contains if we're in verbose mode
    wostream& summary = cons().verbose_stream();
    
    summary << L"    Number of lexer symbols:                " << m_Terminals.count_symbols() << endl;
    summary << L"          ... which are weak:               " << (int)m_WeakSymbols.size() << endl;
    summary << L"          ... which are implicitly defined: " << implicitCount << endl;
    summary << L"          ... which are ignored:            " << (int)m_IgnoredSymbols.size() << endl;
    summary << L"    Number of nonterminals:                 " << m_Grammar.max_item_identifier() << endl;
}


/// \brief Reports which terminal symbols are unused in this language (and any languages that it inherits from)
void language_stage::report_unused_symbols() {
    // Display warnings for unused symbols
    for (set<int>::iterator unused = m_UnusedSymbols.begin(); unused != m_UnusedSymbols.end(); unused++) {
        // Ignore symbols that we don't have a definition location for
        if (!m_TerminalDefinition[*unused].first) {
            cons().report_error(error(error::sev_bug, filename(), L"BUG_UNKNOWN_SYMBOL", L"Unknown unused symbol", position(-1, -1, -1)));
            continue;
        }        

        // Indicate that this symbol was defined but not used in the grammar
        wstringstream msg;
        
        msg << L"Unused terminal symbol definition: " << m_Terminals.name_for_symbol(*unused);
        
        cons().report_error(error(error::sev_warning, *m_TerminalDefinition[*unused].second, L"UNUSED_TERMINAL_SYMBOL", msg.str(), m_TerminalDefinition[*unused].first->start_pos()));
    }
}

/// \brief Adds any lexer items that are defined by a specific EBNF item to this object
int language_stage::add_ebnf_lexer_items(language::ebnf_item* item) {
    int count = 0;

    // Find/create a filename for this object
    wstring* ourFilename;
    if (m_Filenames.find(filename()) != m_Filenames.end()) {
        ourFilename = m_Filenames[filename()];
    } else {
        ourFilename = new wstring(filename());
        m_Filenames[filename()] = ourFilename;
    }
    
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

            m_Lexer.add_definition(item->identifier(), lexer_item(lexer_item::regex, item->identifier(), false, symId, language_unit::unit_keywords_definition, true, ourFilename, item->start_pos()));
            m_UnusedSymbols.insert(symId);

            // Set the type of this symbol
            m_TypeForTerminal[symId] = language_unit::unit_keywords_definition;

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
            int     symId   = m_Terminals.add_symbol(item->identifier());
            wstring dequote = process::dequote_string(item->identifier());

            m_Lexer.add_definition(item->identifier(), lexer_item(lexer_item::literal, dequote, false, symId, language_unit::unit_keywords_definition, true, ourFilename, item->start_pos()));
            m_UnusedSymbols.insert(symId);
            
            // Set the type of this symbol
            m_TypeForTerminal[symId] = language_unit::unit_keywords_definition;
            
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
void language_stage::compile_item(rule& rule, ebnf_item* item, wstring* ourFilename) {
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
            
            // Mark the place where this nonterminal was first used (this is later used to report an error if this nonterminal is undefined)
            if (m_FirstNonterminalUsage.find(nonterminalId) == m_FirstNonterminalUsage.end()) {
                m_FirstNonterminalUsage[nonterminalId] = pair<block*, wstring*>(item, ourFilename);
            }
            
            // Return a new nonterminal item
            rule << item_container(new nonterminal(nonterminalId), true);
            break;
        }
            
        case ebnf_item::ebnf_parenthesized:
        {
            // Just append the items inside this one to the rule
            for (ebnf_item::iterator childItem = item->begin(); childItem != item->end(); childItem++) {
                compile_item(rule, *childItem, ourFilename);
            }
            break;
        }
        
        case ebnf_item::ebnf_optional:
        {
            // Compile into an optional item
            ebnf_optional* newItem = new ebnf_optional();
            compile_item(*newItem->get_rule(), (*item)[0], ourFilename);

            // Create the item container
            item_container newContainer(newItem, true);

            // Store as the first usage of this nonterminal if it exists
            int nonterminalId = m_Grammar.identifier_for_item(newContainer);
            if (m_FirstNonterminalUsage.find(nonterminalId) == m_FirstNonterminalUsage.end()) {
                m_FirstNonterminalUsage[nonterminalId] = pair<block*, wstring*>(item, ourFilename);
            }
            
            // Append to the rule
            rule << newContainer;
            break;
        }
            
        case ebnf_item::ebnf_repeat_one:
        {
            // Compile into a repeating item
            ebnf_repeating* newItem = new ebnf_repeating();
            compile_item(*newItem->get_rule(), (*item)[0], ourFilename);

            // Create the item container
            item_container newContainer(newItem, true);

            // Store as the first usage of this nonterminal if it exists
            int nonterminalId = m_Grammar.identifier_for_item(newContainer);
            if (m_FirstNonterminalUsage.find(nonterminalId) == m_FirstNonterminalUsage.end()) {
                m_FirstNonterminalUsage[nonterminalId] = pair<block*, wstring*>(item, ourFilename);
            }
            
            // Append to the rule
            rule << newContainer;
            break;
        }

        case ebnf_item::ebnf_repeat_zero:
        {
            // Compile into a repeating item
            ebnf_repeating_optional* newItem = new ebnf_repeating_optional();
            compile_item(*newItem->get_rule(), (*item)[0], ourFilename);

            // Create the item container
            item_container newContainer(newItem, true);

            // Store as the first usage of this nonterminal if it exists
            int nonterminalId = m_Grammar.identifier_for_item(newContainer);
            if (m_FirstNonterminalUsage.find(nonterminalId) == m_FirstNonterminalUsage.end()) {
                m_FirstNonterminalUsage[nonterminalId] = pair<block*, wstring*>(item, ourFilename);
            }
            
            // Append to the rule
            rule << newContainer;
            break;
        }
            
        case ebnf_item::ebnf_guard:
        {
            // Compile into a guard item
            guard* newItem = new guard();
            compile_item(*newItem->get_rule(), (*item)[0], ourFilename);
            
            // Append to the rule
            item_container container(newItem, true);
            rule << container;

            // Store as the first usage of this nonterminal if it exists
            int nonterminalId = m_Grammar.identifier_for_item(container);
            if (m_FirstNonterminalUsage.find(nonterminalId) == m_FirstNonterminalUsage.end()) {
                m_FirstNonterminalUsage[nonterminalId] = pair<block*, wstring*>(item, ourFilename);
            }

            // Remember this as a language guard: we'll need to check if it can be empty later
            m_Guards[container] = position_file((*item)[0]->start_pos(), ourFilename);
            break;
        }
            
        case ebnf_item::ebnf_alternative:
        {
            // Compile into an alternate item
            ebnf_alternate* newItem = new ebnf_alternate();
            
            // Left-hand side
            compile_item(*newItem->get_rule(), (*item)[0], ourFilename);
            
            // Right-hand side
            compile_item(*newItem->add_rule(), (*item)[1], ourFilename);

            // Create the item container
            item_container newContainer(newItem, true);

            // Store as the first usage of this nonterminal if it exists
            int nonterminalId = m_Grammar.identifier_for_item(newContainer);
            if (m_FirstNonterminalUsage.find(nonterminalId) == m_FirstNonterminalUsage.end()) {
                m_FirstNonterminalUsage[nonterminalId] = pair<block*, wstring*>(item, ourFilename);
            }
            
            // Append to the rule
            rule << newContainer;
            break;
        }

        default:
            // Unknown item type
            cons().report_error(error(error::sev_bug, filename(), L"BUG_UNKNOWN_EBNF_ITEM_TYPE", L"Unknown type of EBNF item", item->start_pos()));
            break;
    }
}

/// \brief Copies a general symbol/filename block to another
template<class sm> static void copy_symbols(map<wstring, wstring*>& filenames, const sm& source, sm& target) {
    typedef typename sm::const_iterator iterator;
    typedef typename sm::mapped_type    value_type;
    
    // Iterate through the items in the map
    for (iterator sourceItem = source.begin(); sourceItem != source.end(); sourceItem++) {
        // Get/create the filename pointer for this item
        wstring* filenamePtr;
        map<wstring, wstring*>::iterator found = filenames.find(*sourceItem->second.second);

        if (found == filenames.end()) {
            filenamePtr = new wstring(*sourceItem->second.second);
            filenames[*sourceItem->second.second] = filenamePtr;
        } else {
            filenamePtr = found->second;
        }

        // Copy this item (assuming the block won't get freed, the language_stage object generally doesn't own it anyway)
        target[sourceItem->first] = value_type(sourceItem->second.first, filenamePtr);
    }
}

/// \brief Exports the results of this language stage into another
void language_stage::export_to(language_stage* target) {
    // Copy the items that can be simply copied
    target->m_Terminals             = m_Terminals;
    target->m_Lexer                 = m_Lexer;
    target->m_Grammar               = m_Grammar;
    target->m_WeakSymbols           = m_WeakSymbols;
    target->m_IgnoredSymbols        = m_IgnoredSymbols;
    target->m_TypeForTerminal       = m_TypeForTerminal;
    target->m_UnusedSymbols         = m_UnusedSymbols;
    target->m_UsedIgnoredSymbols    = m_UsedIgnoredSymbols;

    // Copy the symbol maps
    copy_symbols(target->m_Filenames, m_TerminalDefinition,     target->m_TerminalDefinition);
    copy_symbols(target->m_Filenames, m_FirstNonterminalUsage,  target->m_FirstNonterminalUsage);
    copy_symbols(target->m_Filenames, m_RuleDefinition,         target->m_RuleDefinition);

    // Copy the guard items (which are in a different format)
    copy_symbols(target->m_Filenames, m_Guards,                 target->m_Guards);
}
