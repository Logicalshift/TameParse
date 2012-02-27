//
//  precedence_rewriter.h
//  TameParse
//
//  Created by Andrew Hunter on 21/01/2012.
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

#ifndef _LR_PRECEDENCE_REWRITER_H
#define _LR_PRECEDENCE_REWRITER_H

#include <climits>

#include "TameParse/ContextFree/item.h"
#include "TameParse/Lr/action_rewriter.h"

namespace lr {
    ///
    /// \brief Rewriter that resolves shift/reduce rules according to operator precedence
    ///
    class precedence_rewriter : public action_rewriter {
    public:
        /// \brief Associativity of a symbol
        enum associativity {
            /// \brief Associativity is undefined (functionally equivalent to nonassociative)
            undefined,

            /// \brief Symbol is not associative
            nonassociative,

            /// \brief Symbol is left-associative
            left,

            /// \brief Symbol is right-associative
            right
        };

        /// \brief Precedence value representing 'no precedence'
        static const int no_precedence = INT_MIN;

    protected:
        /// \brief Finds the item in a rule that defines its precedence, or -1 if the rule has no such items
        int precedence_item_index(const contextfree::rule& rule) const;

    public:
        /// \brief Modifies the specified set of actions according to the rules in this rewriter
        ///
        /// This call should modify the contents of the supplied action set according to whatever rules it considers 
        /// suitable.
        /// 
        /// Typical rewriting actions might include removing actions so that ambiguous grammars can be accepted, replacing
        /// rules with alternatives (such as weak reduce actions for cases where a grammar can be parsed as LR(1)) or
        /// adding new rules.
        virtual void rewrite_actions(int state, lr_action_set& actions, const lalr_builder& builder) const;
        
        /// \brief Creates a clone of this rewriter
        virtual action_rewriter* clone() const = 0;

        /// \brief Returns the precedence for a particular rule
        ///
        /// This is used when trying to get the precedence of the reduce side of a
        /// shift/reduce conflict. The default implementation will look for the final
        /// terminal item in the rule that returns a precedence (determined by calling
        /// get_precedence with an empty item set)
        virtual int get_rule_precedence(const contextfree::rule& rule) const;

        /// \brief Returns the associativity for a particular rule
        ///
        /// This is used when trying to disambiguate a shift/reduce conflict where both
        /// sides have equal precedence. A value of nonassociative will leave the
        /// conflict intact. Left associativity will reduce a + b + c to (a + b) + c,
        /// and right associativity will reduce it to a + (b + c)
        virtual associativity get_rule_associativity(const contextfree::rule& rule) const;

        /// \brief Retrieves the precedence of a particular symbol
        ///
        /// Subclasses should implement this to supply information about symbol
        /// precedence. A value of c_NoPrecedence indicates that a symbol does not
        /// supply a precedence. Symbols with higher precedence are reduced first.
        virtual int get_precedence(const contextfree::item_container& terminal, const lr0_item_set& shiftItems) const = 0;

        /// \brief Retrieves the associativity of a particular symbol
        ///
        /// This is used when trying to disambiguate a shift/reduce conflict where
        /// both sides have equal precedence. Note that only the associativity of
        /// the rule has an effect on the result; this method is called by the default
        /// implementation of get_rule_associativity to determine the associativity of
        /// the operator it finds in the rule.
        virtual associativity get_associativity(const contextfree::item_container& terminal, const lr0_item_set& shiftItems) const = 0;
    };
}

#endif
