//
//  bootstrap.h
//  Parse
//
//  Created by Andrew Hunter on 21/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LANGUAGE_BOOTSTRAP_H
#define _LANGUAGE_BOOTSTRAP_H

#include <string>

#include "Dfa/ndfa_regex.h"
#include "Dfa/lexer.h"
#include "ContextFree/terminal_dictionary.h"
#include "Lr/lalr_builder.h"
#include "Lr/ast_parser.h"

namespace language {
    ///
    /// \brief Bootstrap version of the parser language
    ///
    /// This is just enough of the language to parse the 'proper' language definition.
    ///
    class bootstrap {
    public:
        struct terminals {
            contextfree::item_container
                identifier,
                nonterminal,
                regex,
                string,
                character,
                
                language,
                grammar,
                lexersymbols,
                lexer,
                weaklexer,
                ignore,
                keywords,
                
                equals,
                question,
                plus,
                star,
                colon,
                openparen,
                closeparen,
                opencurly,
                closecurly,
                dot,
                pipe,
                
                newline,
                whitespace,
                comment;
        };
        
    private:
        /// \brief The lexer for this language
        dfa::lexer* m_Lexer;
        
        /// \brief The grammar
        contextfree::grammar* m_Grammar;
        
        /// \brief The LALR builder
        lr::lalr_builder* m_Builder;
        
        /// \brief The parser for this language
        lr::ast_parser* m_Parser;
        
        /// \brief The initial state for the parser
        int m_InitialState;
        
        /// \brief Containers for the terminals in the grammar
        terminals t;
        
        /// \brief Containers for the nonterminals making up this language
        struct {
            contextfree::item_container
                parser_language,
                toplevel_block,
                language_block,
                language_inherits,
                language_definition,
                lexer_symbols_definition,
                lexer_definition,
                ignore_definition,
                keywords_definition,
                keyword_definition,
                weak_symbols_definition,
                lexeme_definition,
                grammar_definition,
                nonterminal_definition,
                production,
                ebnf_item,
                simple_ebnf_item,
                nonterminal,
                terminal,
                basic_terminal;
        } nt;
        
        /// \brief Dictionary of all of the terminals in this language
        contextfree::terminal_dictionary m_Terminals;
        
    private:
        /// \brief Adds a new terminal item to an NDFA, and to this object
        contextfree::item_container add_terminal(dfa::ndfa_regex* ndfa, const std::wstring& name, const std::wstring& regex);
        
    public:
        /// \brief Creates the DFA for the language
        dfa::ndfa* create_dfa();
        
        /// \brief Creates the grammar for the language
        contextfree::grammar* create_grammar();
        
    public:
        /// \brief Constructs the bootstrap language
        bootstrap();
        
        /// \brief Destructor
        virtual ~bootstrap();
        
    public:
        /// \brief Retrieves a string containing the language definition for the parser language
        static const std::string& get_default_language_definition();
        
        /// \brief The lexer for this language
        const dfa::lexer& get_lexer() const { return *m_Lexer; }
        
        /// \brief The grammar for this language
        const contextfree::grammar& get_grammar() const { return *m_Grammar; }
        
        /// \brief The LALR parser builder for this language
        const lr::lalr_builder& get_builder() const { return *m_Builder; }
        
        /// \brief The parser for this language
        const lr::ast_parser& get_parser() const { return *m_Parser; }
        
        /// \brief The dictionary mapping terminal names to their identifiers
        const contextfree::terminal_dictionary& get_terminals() const { return m_Terminals; }
        
        /// \brief Containers for each of the terminals in the bootstrap grammar
        const terminals& get_terminal_items() const { return t; }
    };
}

#endif
