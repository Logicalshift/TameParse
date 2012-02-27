//
//  character_lexer.h
//  Parse
//
//  Created by Andrew Hunter on 14/05/2011.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the \"Software\"), to 
//  deal in the Software without restriction, including without limitation the 
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
//  sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
//  IN THE SOFTWARE.
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
