//
//  conflict_attribute_rewriter.h
//  TameParse
//
//  Created by Andrew Hunter on 08/01/2012.
//  Copyright (c) 2012 Andrew Hunter. All rights reserved.
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
