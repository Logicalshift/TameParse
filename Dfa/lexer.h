//
//  lexer.h
//  Parse
//
//  Created by Andrew Hunter on 28/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_LEXER_H
#define _DFA_LEXER_H

#include <string>

#include "Dfa/ndfa_regex.h"
#include "Dfa/basic_lexer.h"

namespace dfa {
    /// \brief Class used to build and run lexers
    ///
    /// When this class is first created, it can be used t
    class lexer : public basic_lexer {
    private:
        /// \brief NULL if the NDFA is compiled, or the NDFA associated with this lexer
        ndfa_regex* m_Ndfa;
        
        /// \brief NULL, or the compiled lexer
        basic_lexer* m_Lexer;
        
        /// \brief No copying for this class
        inline lexer(const lexer& copyFrom) { }
        
    public:
        /// \brief Creates a default lexer
        lexer();
        
        /// \brief Creates an instance of this class that will use the specified NDFA for building the lexer
        ///
        /// The supplied NDFA will be destroyed when this class is destroyed (or when it gets compiled).
        lexer(ndfa_regex* ndfa);
        
        /// \brief Creates an instance of this class that will use the specified basic_lexer
        ///
        /// The lexer supplied to this call will be destroyed when this class is destroyed
        lexer(basic_lexer* lexer);
        
        /// \brief Destructor
        virtual ~lexer();
        
        ///
        /// \brief Creates a new lexer to process the specified symbol stream
        ///
        /// The lexeme_stream should take ownership of the supplied lexer_symbol_stream and delete it once it has finished with it
        ///
        virtual lexeme_stream* create_stream(lexer_symbol_stream* stream) const;
        
        /// \brief Adds a new symbol to this lexer, if it isn't compiled
        void add_symbol(const symbol_string& regex, int symbolId);
        
        /// \brief Adds a new symbol to this lexer, if it isn't compiled
        inline void add_symbol(std::string regex, int symbolId) { add_symbol(ndfa_regex::convert(regex), symbolId); }

        /// \brief Adds a new symbol to this lexer, if it isn't compiled
        inline void add_symbol(std::wstring regex, int symbolId) { add_symbol(ndfa_regex::convert(regex), symbolId); }
        
        /// \brief Compiles this lexer so that it is ready for use
        ///
        /// Set compact to true if you want to build a compact lexer (these are smaller if the transition table is less than 50% full, but
        /// execute more slowly)
        void compile(bool compact = false);
    };
}

#endif