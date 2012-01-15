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
void lr1_rewriter::rewrite_actions(int stateId, lr_action_set& actions, const lalr_builder& builder) const {
	// Maps symbols to the symbols they reduce as
	typedef set<item_container> item_list;
	typedef map<item_container, item_list> 	item_reductions;

	item_reductions reductions;

	// Set to true if we find a conflict in this state
	bool hasConflict = false;

	// Search for reduce/reduce conflicts
	for (lr_action_set::iterator act = actions.begin(); act != actions.end(); act++) {
		// We're only interested in reduce actions
		if ((*act)->type() != lr_action::act_reduce) continue;

		// Ignore reductions that occur on non-terminal items
		if ((*act)->item()->type() != item::terminal) continue;

		// Add this to the list of reductions for this item
		item_list& items = reductions[(*act)->item()];
		items.insert((*act)->rule()->nonterminal());

		if (!hasConflict && items.size() >= 2) {
			hasConflict = true;
		}
	}

	// Nothing more to do if we found no conflicts
	if (!hasConflict) return;

	// Fetch the state
	const lalr_state& state = *builder.machine().state_with_id(stateId);

	// Find items which have reduce/reduce conflicts
	// (LALR parsers can have reduce/reduce conflicts that LR(1) parsers do not)
	for (item_reductions::iterator reduce = reductions.begin(); reduce != reductions.end(); reduce++) {
		// Only process items with reduce/reduce conflicts
		if (reduce->second.size() < 2) continue;
	
		// Get the terminal that the reduction happens on
		item_container reduceTerminal(reduce->first);

		// Find the target states for this item and nonterminal
		map<int, set<int> > targetState;

		for (int lrItemId = 0; lrItemId < state.count_items(); lrItemId++) {
			// Fetch this item
			const lr0_item& lrItem = *state[lrItemId];

			// This item must cause a reduction
			if (!lrItem.at_end()) continue;

			// It must contain the target terminal in the lookahead
			const item_set& lookahead = state.lookahead_for(lrItemId);
			if (!lookahead.contains(reduceTerminal)) continue;

			// Find the lookahead source for this item
			typedef lalr_builder::lr_item_id lr_item_id;
			set<lr_item_id> sourceItems;
			builder.find_lookahead_source(stateId, lrItemId, reduceTerminal, sourceItems);
		}
	}
}

/// \brief Creates a clone of this rewriter
action_rewriter* lr1_rewriter::clone() const {
	return new lr1_rewriter();
}
