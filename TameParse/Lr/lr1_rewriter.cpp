//
//  lr1_rewriter.cpp
//  TameParse
//
//  Created by Andrew Hunter on 13/01/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <vector>
#include <map>

#include "TameParse/Lr/lr1_rewriter.h"

using namespace std;
using namespace contextfree;
using namespace lr;

/// \brief Modifies the specified set of actions according to the rules in this rewriter
///
/// This call should modify the contents of the supplied action set according to whatever rules it considers 
/// suitable.
/// 
/// Typical rewriting actions might include removing actions so that ambiguous grammars can be accepted, replacing
/// rules with alternatives (such as weak reduce actions for cases where a grammar can be parsed as LR(1)) or
/// adding new rules.
void lr1_rewriter::rewrite_actions(int state, lr_action_set& actions, const lalr_builder& builder) const {
	// Maps symbols to the symbols they reduce as
	typedef vector<item_container> 			item_list;
	typedef map<item_container, item_list> 	item_reductions;

	item_reductions reductions;

	// Search for reduce/reduce conflicts
	for (lr_action_set::iterator act = actions.begin(); act != actions.end(); act++) {
		// We're only interested in reduce actions
		if ((*act)->type() != lr_action::act_reduce) continue;

		// Add this to the list of reductions for this item
		reductions[(*act)->item()].push_back((*act)->rule()->nonterminal());
	}

	// Find items which have reduce/reduce conflicts
	// (LALR parsers can have reduce/reduce conflicts that LR(1) parsers do not)
	for (item_reductions::iterator reduce = reductions.begin(); reduce != reductions.end(); reduce++) {
		// Only process items with reduce/reduce conflicts
		if (reduce->second.size() < 2) continue;
	}
}

/// \brief Creates a clone of this rewriter
action_rewriter* lr1_rewriter::clone() const {
	return new lr1_rewriter();
}
