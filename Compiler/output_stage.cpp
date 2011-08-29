//
//  output_stage.cpp
//  Parse
//
//  Created by Andrew Hunter on 29/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "output_stage.h"

using namespace std;
using namespace dfa;
using namespace compiler;

/// \brief Creates a new output stage
output_stage::output_stage(console_container& console, const std::wstring& filename, lexer_stage* lexer, language_stage* language, lr_parser_stage* parser)
: compilation_stage(console, filename)
, m_LexerStage(lexer)
, m_LanguageStage(language)
, m_ParserStage(parser) {
}

/// \brief Destructor
output_stage::~output_stage() {
}

/// \brief Compiles the parser specified by this stage
///
/// Subclasses can override this if they want to substantially change the way that the
/// compiler is generated.
void output_stage::compile() {
	// TODO: sanity check

	// Start writing the output
	begin_output();

	// Write out the symbol IDs, etc first
	define_symbols();

	// Next, write out the lexer definition
	define_lexer_tables();

	// Finished writing the output
	end_output();
}

/// \brief Defines the symbols associated with this language
void output_stage::define_symbols() {
	// TODO: sanity check

	// Write out the terminal symbols that are defined in this language
	begin_terminal_symbols();

	for (int symbolId = 0; symbolId < m_LanguageStage->terminals()->count_symbols(); symbolId++) {
		terminal_symbol(m_LanguageStage->terminals()->name_for_symbol(symbolId), symbolId);
	}

	end_terminal_symbols();

	// Write out the nonterminal symbols that are defined in this language
	begin_nonterminal_symbols();

	for (int symbolId = 0; symbolId < m_LanguageStage->grammar()->max_nonterminal(); symbolId++) {
		nonterminal_symbol(m_LanguageStage->grammar()->name_for_nonterminal(symbolId), symbolId);
	}

	end_nonterminal_symbols();
}

/// \brief Writes out the lexer tables (the symbol map and the state table)
void output_stage::define_lexer_tables() {
	// TODO: sanity check

	// Get the lexer DFA
	const ndfa* dfa = m_LexerStage->dfa();

	// Get the symbol table
	const class symbol_map& symbols = dfa->symbols();

	// Write to the result
	begin_lexer_symbol_map(symbols.count_sets());

	// Go through all of the symbol sets
	for (symbol_map::iterator setIt = symbols.begin(); setIt != symbols.end(); setIt++) {
		// Go through the ranges in each set
		for (symbol_set::iterator rangeIt = setIt->first->begin(); rangeIt != setIt->first->end(); rangeIt++) {
			symbol_map(*rangeIt, setIt->second);
		}
	}

	end_lexer_symbol_map();

	// Start writing out the DFA
	begin_lexer_state_machine(dfa->count_states());

	// Write out each state in turn
	for (int stateId = 0; stateId < dfa->count_states(); stateId++) {
		// Start writing out this state
		const state& state = dfa->get_state(stateId);
		begin_lexer_state(stateId);

		for (state::iterator transit = state.begin(); transit != state.end(); transit++) {
			lexer_state_transition(transit->symbol_set(), transit->new_state());
		}

		end_lexer_state();
	}

	end_lexer_state_machine();

	// Write out the accepting states
	begin_lexer_accept_table();

	for (int stateId = 0; stateId < dfa->count_states(); stateId++) {
		// Get the actions for this state
		typedef ndfa::accept_action_list accept_action_list;
		const accept_action_list& actions = dfa->actions_for_state(stateId);

		// Nothing to do if there are no actions for this state
		if (actions.begin() == actions.end()) {
			nonaccepting_state(stateId);
			continue;
		}

		// Write out the highest action
		accept_action_list::const_iterator thisAction = actions.begin();
        const accept_action* highest = *thisAction;

        thisAction++;
        for (; thisAction != actions.end(); thisAction++) {
            if ((*highest) < **thisAction) {
                highest = *thisAction;
            }
        }

        // Write out this action
        accepting_state(stateId, highest->symbol());
	}

	end_lexer_accept_table();
}

/// \brief About to begin writing out output
void output_stage::begin_output() {
	// Do nothing in the default implementation
}

/// \brief Finishing writing out output
void output_stage::end_output() {
	// Do nothing in the default implementation
}

/// \brief The output stage is about to produce a list of terminal symbols
void output_stage::begin_terminal_symbols() {
	// Do nothing in the default implementation
}

/// \brief Specifies the identifier for the terminal symbol with a given name
void output_stage::terminal_symbol(const std::wstring& name, int identifier) {
	// Do nothing in the default implementation
}

/// \brief Finished writing out the terminal symbols
void output_stage::end_terminal_symbols() {
	// Do nothing in the default implementation
}

/// \brief The output stage is about to produce a list of non-terminal symbols
void output_stage::begin_nonterminal_symbols() {
	// Do nothing in the default implementation
}

/// \brief Specifies the identifier for the non-terminal symbol with a given name
void output_stage::nonterminal_symbol(const std::wstring& name, int identifier) {
	// Do nothing in the default implementation
}

/// \brief Finished writing out the terminal symbols
void output_stage::end_nonterminal_symbols() {
	// Do nothing in the default implementation
}

/// \brief Starting to write out the symbol map for the lexer
void output_stage::begin_lexer_symbol_map(int maxSetId) {
	// Do nothing in the default implementation
}

/// \brief Specifies that a given range of symbols maps to a particular identifier
void output_stage::symbol_map(const dfa::range<int>& symbolRange, int identifier) {
	// Do nothing in the default implementation
}
		
/// \brief Finishing writing out the symbol map for the lexer
void output_stage::end_lexer_symbol_map() {
	// Do nothing in the default implementation
}

/// \brief About to begin writing out the lexer tables
void output_stage::begin_lexer_state_machine(int numStates) {
	// Do nothing in the default implementation
}

/// \brief Starting to write out the transitions for a given state
void output_stage::begin_lexer_state(int stateId) {
	// Do nothing in the default implementation
}

/// \brief Adds a transition for the current state
void output_stage::lexer_state_transition(int symbolSet, int newState) {
	// Do nothing in the default implementation
}

/// \brief Finishes writing out a lexer state
void output_stage::end_lexer_state() {
	// Do nothing in the default implementation
}

/// \brief Finished writing out the lexer table
void output_stage::end_lexer_state_machine() {
	// Do nothing in the default implementation
}

/// \brief About to write out the list of accepting states for a lexer
void output_stage::begin_lexer_accept_table() {
	// Do nothing in the default implementation
}

/// \brief The specified state is not an accepting state
void output_stage::nonaccepting_state(int stateId) {
	// Do nothing in the default implementation
}

/// \brief The specified state is an accepting state
void output_stage::accepting_state(int stateId, int acceptSymbolId) {
	// Do nothing in the default implementation
}

/// \brief Finished the lexer acceptance table
void output_stage::end_lexer_accept_table() {
	// Do nothing in the default implementation
}
