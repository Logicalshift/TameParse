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
    // Find any lexer-symbols sections and add them to the lexer
    for (language_block::iterator lexerSymbols = m_Language->begin(); lexerSymbols != m_Language->end(); lexerSymbols++) {
        if ((*lexerSymbols)->type() != language_unit::unit_lexer_symbols) continue;
        
        // TODO: implement me
    }
    
    // Create symbols for all of the items defined in lexer blocks
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
    
    // Build the grammar
}
