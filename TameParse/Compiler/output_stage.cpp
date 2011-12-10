//
//  output_stage.cpp
//  Parse
//
//  Created by Andrew Hunter on 29/08/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#include "TameParse/Compiler/output_stage.h"

using namespace std;
using namespace dfa;
using namespace contextfree;
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

	// Writes out the parser tables
	define_parser_tables();

	// Writes out the AST tables
	define_ast_tables();

	// Finished writing the output
	end_output();
}

/// \brief Defines the symbols associated with this language
void output_stage::define_symbols() {
	// TODO: remove me!
}

/// \brief Writes out the lexer tables (the symbol map and the state table)
void output_stage::define_lexer_tables() {
	// TODO: sanity check

	// Get the lexer DFA
	const ndfa* dfa = m_LexerStage->dfa();

	// Get the symbol table
	const class symbol_map& symbols = dfa->symbols();

	// Starting to write lexer definitions
	begin_lexer_definitions();

	// Write to the result
	begin_lexer_symbol_map(symbols.count_sets());

	// Go through all of the symbol sets
	for (symbol_map::iterator setIt = symbols.begin(); setIt != symbols.end(); ++setIt) {
		// Go through the ranges in each set
		for (symbol_set::iterator rangeIt = setIt->first->begin(); rangeIt != setIt->first->end(); ++rangeIt) {
			symbol_map(*rangeIt, setIt->second);
		}
	}

	end_lexer_symbol_map();

	// Start writing out the DFA
	begin_lexer_state_machine(dfa->count_states());

	// Write out each state in turn
	for (int stateId = 0; stateId < dfa->count_states(); ++stateId) {
		// Start writing out this state
		const state& state = dfa->get_state(stateId);
		begin_lexer_state(stateId);

		for (state::iterator transit = state.begin(); transit != state.end(); ++transit) {
			lexer_state_transition(transit->symbol_set(), transit->new_state());
		}

		end_lexer_state();
	}

	end_lexer_state_machine();

	// Write out the accepting states
	begin_lexer_accept_table();

	for (int stateId = 0; stateId < dfa->count_states(); ++stateId) {
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

        ++thisAction;
        for (; thisAction != actions.end(); ++thisAction) {
            if ((*highest) < **thisAction) {
                highest = *thisAction;
            }
        }

        // Write out this action
        accepting_state(stateId, highest->symbol());
	}

	end_lexer_accept_table();

	// Finished the lexer
	end_lexer_definitions();
}

/// \brief Writes out the AST tables
void output_stage::define_ast_tables() {
	// Start the tables
	begin_ast_definitions(*m_LanguageStage->grammar(), *m_LanguageStage->terminals());
    const grammar& gram = *m_LanguageStage->grammar();

    // Output the terminals
    for (int termId = 0; termId < m_LanguageStage->terminals()->count_symbols(); ++termId) {
    	// Get the ID for this terminal
    	terminal 	term(termId);
    	int 		symbolId = gram.identifier_for_item(term);

    	// Write it out
    	begin_ast_terminal(symbolId, term);
    	end_ast_terminal();
	}
    
    // Maps nonterminals to rules (this list is built up separately as the nonterminals within the grammar won't
    // contain any rules that are implicitly generated)
    map<int, rule_list> rulesForNonterminal;
    
    // Iterate through the rules
    for (int ruleId = 0; ruleId < gram.max_rule_identifier(); ++ruleId) {
        // Fetch this rule
        const rule_container& nextRule = gram.rule_with_identifier(ruleId);
        
        // Get the nonterminal ID
        int nonterminalId = gram.identifier_for_item(nextRule->nonterminal());
        
        // Append to the list
        rulesForNonterminal[nonterminalId].push_back(nextRule);
    }


    // Iterate through the nonterminals
    for (map<int, rule_list>::iterator nonterminalDefn = rulesForNonterminal.begin(); nonterminalDefn != rulesForNonterminal.end(); ++nonterminalDefn) {
        // Begin this nonterminal
        begin_ast_nonterminal(nonterminalDefn->first, gram.item_with_identifier(nonterminalDefn->first));
        
        // Iterate through the rules
        for (rule_list::const_iterator ruleDefn = nonterminalDefn->second.begin(); ruleDefn != nonterminalDefn->second.end(); ++ruleDefn) {
            // Start this rule
            begin_ast_rule(gram.identifier_for_rule(*ruleDefn));
            
            // Write out the rule items
            for (rule::iterator ruleItem = (*ruleDefn)->begin(); ruleItem != (*ruleDefn)->end(); ++ruleItem) {
                if ((*ruleItem)->type() == item::terminal) {
                    // Terminal item
                    rule_item_terminal(gram.identifier_for_item(*ruleItem), (*ruleItem)->symbol(), *ruleItem);
                } else {
                    // Nonterminal item
                    rule_item_nonterminal(gram.identifier_for_item(*ruleItem), *ruleItem);
                }
            }
            
            // Finished this rule
            end_ast_rule();
        }
        
        // Finished this nonterminal
        end_ast_nonterminal();
    }

	// Iterate through the symbols
	for (int symbolId = 0; symbolId < m_LanguageStage->grammar()->max_item_identifier(); ++symbolId) {
		// Fetch this item
		const item_container& item = m_LanguageStage->grammar()->item_with_identifier(symbolId);

		// Ignore terminal items
		if (item->type() == item::terminal) continue;

		// TODO: write out the items
	}

	// Finished
	end_ast_definitions();
}

/// \brief Writes out the parser tables
void output_stage::define_parser_tables() {
	// Start the parser definitions
	begin_parser_definitions();

	// Send the tables
	parser_tables(*m_ParserStage->get_parser(), *m_ParserStage->get_tables());

	// Finished the parser
	end_parser_definitions();
}

/// \brief About to begin writing out output
void output_stage::begin_output() {
	// Do nothing in the default implementation
}

/// \brief Finishing writing out output
void output_stage::end_output() {
	// Do nothing in the default implementation
}

/// \brief Generates the terminal symbols list
void output_stage::generate_terminal_symbols() {
	// Clear out the existing symbols (so we regenerate if this is called multiple times)
	m_TerminalSymbols.clear();

	// Fill in the terminal symbols
	for (int symbolId = 0; symbolId < m_LanguageStage->terminals()->count_symbols(); ++symbolId) {
		m_TerminalSymbols.push_back(terminal_symbol(m_LanguageStage->terminals()->name_for_symbol(symbolId), symbolId));
	}
}

/// \brief Generates the nonterminal symbols list
void output_stage::generate_nonterminal_symbols() {
	// Clear out the existing symbols (so we regenerate if this is called multiple times)
	m_NonterminalSymbols.clear();

	// Fill in the nonterminal symbols
	for (int symbolId = 0; symbolId < m_LanguageStage->grammar()->max_item_identifier(); ++symbolId) {
		// Assume that the nonterminal IDs match up to item IDs (they should do)
		item_container ntItem = m_LanguageStage->grammar()->item_with_identifier(symbolId);

		// Must be an actual named nonterminal
		if (ntItem->type() != item::nonterminal) continue;

		// Output this item
		m_NonterminalSymbols.push_back(nonterminal_symbol(m_LanguageStage->grammar()->name_for_nonterminal(symbolId), symbolId, ntItem));
	}
}

/// \brief The first terminal symbol
output_stage::terminal_symbol_iterator output_stage::begin_terminal_symbol() {
	// Generate the terminal symbols if they don't already exist (or, as an edge case, if they exist but there aren't any)
	if (m_TerminalSymbols.empty()) generate_terminal_symbols();

	return m_TerminalSymbols.begin();
}

/// \brief The symbol after the final terminal symbol
output_stage::terminal_symbol_iterator output_stage::end_terminal_symbol() {
	// Generate the terminal symbols if they don't already exist (or, as an edge case, if they exist but there aren't any)
	if (m_TerminalSymbols.empty()) generate_terminal_symbols();

	return m_TerminalSymbols.end();
}

/// \brief The first nonterminal symbol
output_stage::nonterminal_symbol_iterator output_stage::begin_nonterminal_symbol() {
	// Generate the terminal symbols if they don't already exist (or, as an edge case, if they exist but there aren't any)
	if (m_NonterminalSymbols.empty()) generate_nonterminal_symbols();

	return m_NonterminalSymbols.begin();
}

/// \brief The symbol after the final nonterminal symbol
output_stage::nonterminal_symbol_iterator output_stage::end_nonterminal_symbol() {
	// Generate the terminal symbols if they don't already exist (or, as an edge case, if they exist but there aren't any)
	if (m_NonterminalSymbols.empty()) generate_nonterminal_symbols();

	return m_NonterminalSymbols.end();
}

/// \brief Starting to write out the lexer definitions
void output_stage::begin_lexer_definitions() {
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

/// \brief Starting to write out the lexer definitions
void output_stage::end_lexer_definitions() {
	// Do nothing in the default implementation
}

/// \brief Starting to write out the definitions associated with the parser
void output_stage::begin_parser_definitions() {
	// Do nothing in the default implementation
}

/// \brief Supplies the parser tables generated by the compiler
void output_stage::parser_tables(const lr::lalr_builder& builder, const lr::parser_tables& tables) {
	// Do nothing in the default implementation
}

/// \brief Finished the parser definitions
void output_stage::end_parser_definitions() {
	// Do nothing in the default implementation
}

/// \brief Starting to write the AST definitions for a particular terminal symbol
void output_stage::begin_ast_terminal(int identifier, const contextfree::item_container& item) {
	// Do nothing in the default implementation
}

/// \brief Finished writing the definitions for a terminal
void output_stage::end_ast_terminal() {
	// Do nothing in the default implementation
}

/// \brief Starting to write out the definitions associated with the AST
void output_stage::begin_ast_definitions(const contextfree::grammar& grammar, const contextfree::terminal_dictionary& dict) {
	// Do nothing in the default implementation
}

/// \brief Starting to write the AST definitions for the specified nonterminal
void output_stage::begin_ast_nonterminal(int identifier, const contextfree::item_container& item) {
	// Do nothing in the default implementation
}

/// \brief Starting to write out a rule in the current nonterminal
void output_stage::begin_ast_rule(int identifier) {
	// Do nothing in the default implementation
}

/// \brief Writes out an individual item in the current rule (a nonterminal)
void output_stage::rule_item_nonterminal(int nonterminalId, const contextfree::item_container& item) {
	// Do nothing in the default implementation
}

/// \brief Writes out an individual item in the current rule (a terminal)
///
/// Note the distinction between the item ID, which is part of the grammar, and the
/// symbol ID (which is part of the lexer and is the same as the value passed to 
/// terminal_symbol)
void output_stage::rule_item_terminal(int terminalItemId, int terminalSymbolId, const item_container& item) {
	// Do nothing in the default implementation
}

/// \brief Finished writing out 
void output_stage::end_ast_rule() {
	// Do nothing in the default implementation
}

/// \brief Finished writing the definitions for a nonterminal
void output_stage::end_ast_nonterminal() {
	// Do nothing in the default implementation
}

/// \brief Finished writing out the AST information
void output_stage::end_ast_definitions() {
	// Do nothing in the default implementation
}
