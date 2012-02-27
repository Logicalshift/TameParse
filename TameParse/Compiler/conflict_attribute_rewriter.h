//
//  conflict_attribute_rewriter.h
//  TameParse
//
//  Created by Andrew Hunter on 08/01/2012.
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

#ifndef _COMPILER_CONFLICT_ATTRIBUTE_REWRITER_H
#define _COMPILER_CONFLICT_ATTRIBUTE_REWRITER_H

#include "TameParse/Compiler/Data/rule_item_data.h"
#include "TameParse/Lr/action_rewriter.h"

namespace compiler {
    ///
    /// \brief Rewrites shift/reduce conflicts according to the attributes specified in the item being shifted
    ///
    class conflict_attribute_rewriter : public lr::action_rewriter {
    private:
        /// \brief Data object containing the attributes applied to each rule item
        const compiler::rule_item_data* m_RuleData;

    public:
        /// \brief Creates a new conflict rewriter
        conflict_attribute_rewriter(const compiler::rule_item_data* ruleData);

        /// \brief Modifies the specified set of actions according to the rules in this rewriter
        virtual void rewrite_actions(int state, lr::lr_action_set& actions, const lr::lalr_builder& builder) const;
        
        /// \brief Creates a clone of this rewriter
        virtual lr::action_rewriter* clone() const;
    };
}

#endif
