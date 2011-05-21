//
//  bootstrap.h
//  Parse
//
//  Created by Andrew Hunter on 21/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LANGUAGE_BOOTSTRAP_H
#define _LANGUAGE_BOOTSTRAP_H

#include "Dfa/lexer.h"
#include "Lr/ast_parser.h"

namespace language {
    ///
    /// \brief Bootstrap version of the parser language
    ///
    /// This is just enough of the language to parse the 'proper' language definition.
    ///
    class bootstrap {
    public:
        /// \brief The terminal symbols associated with this language
        enum terminal {
            identifier = 0,
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
            comment
        };
        
    private:
        /// \brief The lexer for this language
        dfa::lexer* m_Lexer;
        
    private:
        /// \brief Creates the DFA for the language
        static dfa::ndfa* create_dfa();
        
    public:
        /// \brief Constructs the bootstrap language
        bootstrap();
        
        /// \brief Destructor
        virtual ~bootstrap();
    };
}

#endif
