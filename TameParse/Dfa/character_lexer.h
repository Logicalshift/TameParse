//
//  character_lexer.h
//  Parse
//
//  Created by Andrew Hunter on 14/05/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#ifndef _DFA_CHARACTER_LEXER_H
#define _DFA_CHARACTER_LEXER_H

#include "TameParse/Dfa/basic_lexer.h"

namespace dfa {
    ///
    /// \brief Lexer that splits a stream by character
    ///
    class character_lexer : public basic_lexer {
    private:
        class lstream : public lexeme_stream {
        private:
            /// \brief The symbol stream that this will read from
            lexer_symbol_stream* m_Stream;
            
            /// \brief Position tracker
            position_tracker m_Position;
            
        public:
            /// \brief Constructor
            lstream(lexer_symbol_stream* source);
            
            virtual ~lstream();
            
            /// \brief Fills in the contents of the specified pointer with the next lexeme (or NULL if the end of input has been reached)
            ///
            /// The caller needs to delete the resulting lexeme object
            virtual lexeme_stream& operator>>(lexeme*& result);
        };
        
    public:
        ///
        /// \brief Creates a new lexer to process the specified symbol stream
        ///
        /// The lexeme_stream should take ownership of the supplied lexer_symbol_stream and delete it once it has finished with it
        ///
        virtual lexeme_stream* create_stream(lexer_symbol_stream* stream) const;
        
        ///
        /// \brief The number of bytes used by this lexer
        ///
        virtual size_t size() const;
    };
}

#endif
