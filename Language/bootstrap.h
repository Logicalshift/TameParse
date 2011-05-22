//
//  bootstrap.h
//  Parse
//
//  Created by Andrew Hunter on 21/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LANGUAGE_BOOTSTRAP_H
#define _LANGUAGE_BOOTSTRAP_H

#include "Dfa/ndfa_regex.h"
#include "Dfa/lexer.h"
#include "ContextFree/terminal_dictionary.h"
#include "Lr/ast_parser.h"

namespace language {
    ///
    /// \brief Bootstrap version of the parser language
    ///
    /// This is just enough of the language to parse the 'proper' language definition.
    ///
    class bootstrap {
    public:
        
    private:
        /// \brief The lexer for this language
        dfa::lexer* m_Lexer;
        
        /// \brief Containers for the terminals in the grammar
        struct {
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
                
                newline,
                whitespace,
                comment;
        } t;
        
        /// \brief Containers for the nonterminals making up this language
        struct {
            
        } nt;
        
        /// \brief Dictionary of all of the terminals in this language
        contextfree::terminal_dictionary m_Terminals;
        
    private:
        /// \brief Adds a new terminal item to an NDFA, and to this object
        contextfree::item_container add_terminal(dfa::ndfa_regex* ndfa, const std::wstring& name, const std::wstring& regex);
        
        /// \brief Creates the DFA for the language
        dfa::ndfa* create_dfa();
        
    public:
        /// \brief Constructs the bootstrap language
        bootstrap();
        
        /// \brief Destructor
        virtual ~bootstrap();
    };
}

#endif
