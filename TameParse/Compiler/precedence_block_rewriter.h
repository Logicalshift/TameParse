//
//  precedence_block_rewriter.h
//  TameParse
//
//  Created by Andrew Hunter on 22/01/2012.
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

#ifndef _COMPILER_PRECEDENCE_BLOCK_REWRITER_H
#define _COMPILER_PRECEDENCE_BLOCK_REWRITER_H

#include <map>

#include "TameParse/ContextFree/terminal_dictionary.h"
#include "TameParse/Lr/precedence_rewriter.h"
#include "TameParse/Language/precedence_block.h"

namespace compiler {
    ///
    /// \brief Rewriter that uses the contents of a precedence block to resolve shift/reduce conflicts
    ///
    class precedence_block_rewriter : public lr::precedence_rewriter {
    private:
        /// \brief Maps terminal symbols to their precedence
        std::map<int, int> m_TerminalPrecedence;

        /// \brief Maps terminal symbols to their associativity
        std::map<int, associativity> m_TerminalAssociativity;

    public:
        /// \brief Construcuts a new rewriter by interpreting a language block
        precedence_block_rewriter(const contextfree::terminal_dictionary& terminals, const yy_language::precedence_block& precedence);

        /// \brief Creates a clone of this rewriter
        virtual action_rewriter* clone() const;

        /// \brief Retrieves the precedence of a particular symbol
        ///
        /// Subclasses should implement this to supply information about symbol
        /// precedence. A value of c_NoPrecedence indicates that a symbol does not
        /// supply a precedence. Symbols with higher precedence are reduced first.
        virtual int get_precedence(const contextfree::item_container& terminal, const lr::lr0_item_set& shiftItems) const;

        /// \brief Retrieves the associativity of a particular symbol
        ///
        /// This is used when trying to disambiguate a shift/reduce conflict where
        /// both sides have equal precedence. Note that only the associativity of
        /// the rule has an effect on the result; this method is called by the default
        /// implementation of get_rule_associativity to determine the associativity of
        /// the operator it finds in the rule.
        virtual associativity get_associativity(const contextfree::item_container& terminal, const lr::lr0_item_set& shiftItems) const;
    };
}

#endif
