//
//  regex_error.h
//  Parse
//
//  Created by Andrew Hunter on 22/11/2011.
//  
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

#ifndef _DFA_REGEX_ERROR_H
#define _DFA_REGEX_ERROR_H

#include <string>

#include "TameParse/Dfa/position.h"

namespace dfa {
    /// \brief Class representing an error in a regular expression
    class regex_error {
    public:
        /// \brief The types of error that can be detected within a regular expression
        enum error_type {
            /// \brief Unknown type of error
            unknown,

            /// \brief Missing ')'
            missing_round_bracket,

            /// \brief Missing ']'
            missing_square_bracket,

            /// \brief Missing '}'
            missing_curly_bracket,

            /// \brief An expression that doesn't resolve to anything
            missing_expression
        };

    private:
        /// \brief The type of error
        error_type m_Type;

        /// \brief Position (relative to the regular expression)
        position m_Position;

        /// \brief Name of the symbol that the error applies to (where applicable)
        std::basic_string<int> m_Symbol;

    public:
        /// \brief Constructor
        regex_error(error_type type, const position& pos);

        /// \brief Constructor
        regex_error(error_type type, const position& pos, const std::basic_string<int>& symbol);

        /// \brief The type of error
        inline error_type type() const { return m_Type; }

        /// \brief The position of this error within the regular expression
        inline const position& pos() const { return m_Position; }

        /// \brief Name of the symbol that the error applies to (where applicable)
        const std::basic_string<int>& symbol() const { return m_Symbol; }
    };
}

#endif
