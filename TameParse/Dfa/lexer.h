//
//  lexer.h
//  Parse
//
//  Created by Andrew Hunter on 28/04/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#ifndef _DFA_LEXER_H
#define _DFA_LEXER_H

#include <string>

#include "TameParse/Dfa/ndfa_regex.h"
#include "TameParse/Dfa/basic_lexer.h"

namespace dfa {
    /// \brief Class used to build and run lexers
    class lexer : public basic_lexer {
    private:
        /// \brief NULL if the NDFA is compiled, or the NDFA associated with this lexer
        ndfa_regex* m_Ndfa;
        
        /// \brief NULL, or the compiled lexer
        basic_lexer* m_Lexer;

        /// \brief True if this object owns its lexer
        const bool m_OwnsLexer;
        
        /// \brief No copying for this class
        inline lexer(const lexer& copyFrom);

        /// \brief Disabled assignment
        lexer& operator=(const lexer& assignFrom);

    public:
        /// \brief Creates a default lexer
        lexer();
        
        /// \brief Creates an instance of this class that will use the specified NDFA for building the lexer
        ///
        /// The supplied NDFA will be destroyed when this class is destroyed (or when it gets compiled).
        lexer(ndfa_regex* ndfa);
        
        /// \brief Creates an instance of this class that will use the specified DFA for building the lexer
        ///
        /// The DFA will be compiled immediately into a lexer, and can be discarded after this call. Note that this
        /// call will produce an invalid lexer if the supplied object is not deterministic.
        explicit lexer(const ndfa& dfa);
        
        /// \brief Creates an instance of this class that will use the specified basic_lexer
        ///
        /// The lexer supplied to this call will be destroyed when this class is destroyed
        /// if ownsLexer is set to true.
        lexer(basic_lexer* lexer, bool ownsLexer = true);
        
        /// \brief Destructor
        virtual ~lexer();
        
        ///
        /// \brief Creates a new lexer to process the specified symbol stream
        ///
        /// The lexeme_stream should take ownership of the supplied lexer_symbol_stream and delete it once it has finished 
        /// with it. If the lexer is not yet compiled, then it will be compiled by this call.
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
        /// Set compact to true if you want to build a compact lexer (these are smaller if the transition table is less than 
        /// 50% full, but execute more slowly)
        void compile(bool compact = false);
        
    public:
        /// \brief Verifies that this lexer will compile into a valid DFA
        ///
        /// This should be called before compile(). If compile() has already been called, this will return false regardless.
        /// This is primarily intended for testing purposes.
        bool verify();
        
    public:
        /// \brief Estimation of the size of this lexer
        virtual size_t size() const;
    };
}

#endif
