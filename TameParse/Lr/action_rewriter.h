//
//  action_rewriter.h
//  Parse
//
//  Created by Andrew Hunter on 05/05/2011.
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

#ifndef _LR_ACTION_REWRITER_H
#define _LR_ACTION_REWRITER_H

#include "TameParse/Lr/lr_action.h"
#include "TameParse/Lr/lalr_builder.h"
#include "TameParse/Util/container.h"

namespace lr {
    ///
    /// \brief Abstract base class of types that can rearrange and disambiguate LR actions
    ///
    /// Classes that inherit from this can be used with the LALR builder class to rewrite the actions for each state
    /// to remove ambiguities or to refine the parser behaviour.
    ///
    /// The most traditional use for this class is in applying operator precedence to a grammar.
    ///
    /// Subclasses might also implement 'weak' symbols (whose meaning depends on context) or even split LALR states
    /// so that a LALR parser can be extended into one that accepts full LR(1) grammars.
    ///
    class action_rewriter {
    public:
        /// \brief Destructor
        virtual ~action_rewriter();
        
        /// \brief Modifies the specified set of actions according to the rules in this rewriter
        ///
        /// This call should modify the contents of the supplied action set according to whatever rules it considers 
        /// suitable.
        /// 
        /// Typical rewriting actions might include removing actions so that ambiguous grammars can be accepted, replacing
        /// rules with alternatives (such as weak reduce actions for cases where a grammar can be parsed as LR(1)) or
        /// adding new rules.
        virtual void rewrite_actions(int state, lr_action_set& actions, const lalr_builder& builder) const = 0;
        
        /// \brief Creates a clone of this rewriter
        virtual action_rewriter* clone() const = 0;
        
        /// \brief Returns true if a given rewriter is less than another
        static inline bool compare(const action_rewriter* a, const action_rewriter* b) {
            if (a == b) return false;
            if (!a) return true;
            if (!b) return true;
            
            // Just compare pointers by default, assume rewriters are always different
            return a < b;
        }
    };
    
    /// \brief Class used to contain an action rewriter
    typedef util::container<action_rewriter> action_rewriter_container;
}

#endif
