//
//  lr_parser_compiler.cpp
//  Parse
//
//  Created by Andrew Hunter on 27/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <sstream>
#include "lr_parser_compiler.h"
#include "Lr/conflict.h"
#include "Lr/ignored_symbols.h"
#include "Language/formatter.h"

using namespace std;
using namespace dfa;
using namespace contextfree;
using namespace lr;
using namespace language;
using namespace compiler;

/// \brief Constructor
lr_parser_compiler::lr_parser_compiler(console_container& console, const std::wstring& filename, language_compiler* languageCompiler, lexer_compiler* lexerCompiler, const vector<wstring>& startSymbols)
: compilation_stage(console, filename)
, m_Language(languageCompiler)
, m_LexerCompiler(lexerCompiler)
, m_StartSymbols(startSymbols)
, m_StartPosition(position(-1,-1,-1))
, m_Parser(NULL) {
	// Add empty positions for each symbol
	for (size_t x=0; x<m_StartSymbols.size(); x++) {
		m_SymbolStartPosition.push_back(position(-1,-1,-1));
	}
}

/// \brief Constructure which builds the list of start symbols from a parser block
lr_parser_compiler::lr_parser_compiler(console_container& console, const std::wstring& filename, language_compiler* languageCompiler, lexer_compiler* lexerCompiler, parser_block* parserBlock) 
: compilation_stage(console, filename)
, m_Language(languageCompiler)
, m_LexerCompiler(lexerCompiler)
, m_StartPosition(parserBlock->start_pos())
, m_StartSymbols(parserBlock->start_symbols())
, m_Parser(NULL) {
	// Make all the symbols begin in the same place as this block
	// TODO: actually record where the symbols are specified
	for (size_t x=0; x<m_StartSymbols.size(); x++) {
		m_SymbolStartPosition.push_back(m_StartPosition);
	}	
}

/// \brief Destructor
lr_parser_compiler::~lr_parser_compiler() {
	// Finished with the parser
	if (m_Parser) {
		delete m_Parser;
		m_Parser = NULL;
	}
}

/// \brief Compiles the parser specified by the parameters to this stage
void lr_parser_compiler::compile() {
    // Verbose message to say which stage we're at
	cons().verbose_stream() << L"  = Building parser" << endl;

	// Recycle the parser generator if it already exists
	if (m_Parser) {
		delete m_Parser;
		m_Parser = NULL;
	}

	// Sanity check (language)
	if (!m_Language) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE", L"Language compiler stage was not supplied to parser stage", m_StartPosition));
		return;
	}

	if (!m_Language->ndfa()) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE_NDFA", L"Language compiler stage has not generated a lexer", m_StartPosition));
		return;
	}

	if (!m_Language->terminals()) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE_TERMINALS", L"Language compiler stage has not generated a terminal dictionary", m_StartPosition));		
		return;
	}

	if (!m_Language->weak_symbols()) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE_WEAK_SYMBOLS", L"Language compiler stage has not the set of weak symbols", m_StartPosition));
		return;
	}

	if (!m_Language->ignored_symbols()) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE_IGNORE_SYMBOLS", L"Language compiler stage has not generated the set of ignore symbols", m_StartPosition));
	}

	if (!m_Language->grammar()) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE_GRAMMAR", L"Language compiler stage has not generated a grammar", m_StartPosition));		
		return;
	}

	// Sanity check (lexer)
	if (!m_LexerCompiler) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LEXER", L"Lexer compiler stage was not supplied to parser stage", m_StartPosition));
		return;
	}

	if (!m_LexerCompiler->dfa()) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LEXER_DFA", L"Lexer compiler stage has not generate a DFA", m_StartPosition));
		return;
	}

	if (!m_LexerCompiler->weak_symbols()) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LEXER_DFA", L"Lexer compiler stage has not generate a weak symbols rewriter", m_StartPosition));
		return;
	}

	// Create a new parser builder
	m_Parser = new lalr_builder(*m_Language->grammar(), *m_Language->terminals());

	// Get the nonterminal items corresponding to the start symbols
	vector<item_container> startItems;

	for (size_t startSymbolId = 0; startSymbolId < m_StartSymbols.size(); startSymbolId++) {
		// Get the symbol
		const wstring& startSymbol = m_StartSymbols[startSymbolId];

		// Find the nonterminal item corresponding to this symbol
		if (!m_Language->grammar()->nonterminal_is_defined(startSymbol)) {
			// Report an error if this nonterminal is not defined
			wstringstream msg;
			msg << L"Start symbol is not defined: " << startSymbol;

			cons().report_error(error(error::sev_error, filename(), L"UNDEFINED_NONTERMINAL", msg.str(), m_SymbolStartPosition[startSymbolId]));
			continue;
		}

		// Add to the list of start items
		startItems.push_back(m_Language->grammar()->get_nonterminal(startSymbol));
	}

	// Give up if there are no symbols defined
	if (startItems.empty()) {
		cons().report_error(error(error::sev_error, filename(), L"NO_START_SYMBOLS", L"No start symbols are defined", m_StartPosition));
		return;
	}
    
    // Generate the ignore actions
    ignored_symbols* ignored = new ignored_symbols();
    action_rewriter_container ignoreContainer(ignored, true);
    
    for (set<int>::iterator ignoredTerminalId = m_Language->ignored_symbols()->begin(); ignoredTerminalId != m_Language->ignored_symbols()->end(); ignoredTerminalId++) {
        ignored->add_item(terminal(*ignoredTerminalId));
    }

	// Add the initial states to the LALR builder
	m_InitialStates.clear();
	for (vector<item_container>::iterator initialItem = startItems.begin(); initialItem != startItems.end(); initialItem++) {
		m_InitialStates.push_back(m_Parser->add_initial_state(*initialItem));
	}
    
    // Add the weak symbols and ignore items actions
    // TODO: it might be good to have a way to supply extra rewriters from other stages instead of just having them
    // hardcoded here. This is good enough for now, though.
    m_Parser->add_rewriter(action_rewriter_container(m_LexerCompiler->weak_symbols(), false));
    m_Parser->add_rewriter(ignoreContainer);

	// Build the parser
	m_Parser->complete_parser();

	// Get any conflicts that might exist
	conflict_list conflictList;
	conflict::find_conflicts(*m_Parser, conflictList);

	// Report the conflicts
	// TODO: make the way that conflicts are reported (warnings or errors) configurable
	error::severity shiftReduceSev 	= error::sev_warning;
	error::severity reduceReduceSev	= error::sev_error;

	for (conflict_list::iterator conflict = conflictList.begin(); conflict != conflictList.end(); conflict++) {
		// We don't understand the conflict type if there are no reduce items
		if ((*conflict)->first_reduce_item() == (*conflict)->last_reduce_item()) {
			cons().report_error(error(error::sev_bug, filename(), L"BUG_CONFLICT_NO_REDUCE", L"Found a conflict with no reduce actions", m_StartPosition));
			continue;
		}

		// Test the type of this conflict
		if ((*conflict)->first_shift_item() != (*conflict)->last_shift_item()) {
			// Shift/reduce conflict: we report the 'shift' part of the conflict as the first line
			for (lr0_item_set::const_iterator shiftItem = (*conflict)->first_shift_item(); shiftItem != (*conflict)->last_shift_item(); shiftItem++) {
				// Start building the message
				wstringstream 	shiftMessage;
				error::severity	sev = shiftReduceSev;

				// Message is different if this is the initial message for this conflict vs a detail message
				if (shiftItem == (*conflict)->first_shift_item()) {
					// Displaying the shift/reduce warning if we're on the first shift item
					shiftMessage << L"Shift/reduce conflict on";
					shiftMessage << L" '" << formatter::to_string(*(*conflict)->token(), *m_Language->grammar(), *m_Language->terminals()) << L"':";
				} else {
					// Displaying additional items
					shiftMessage << L"  in:";
					sev = error::sev_detail;
				}

				// Add the item being shifted
				shiftMessage << L" " << formatter::to_string(**shiftItem, *m_Language->grammar(), *m_Language->terminals());

				// Display the warning/error
				int 		ruleId 	= (*shiftItem)->rule()->identifier(*m_Language->grammar());
				position 	rulePos	= m_Language->rule_definition_pos(ruleId);
				cons().report_error(error(sev, m_Language->filename(), L"CONFLICT_SHIFT_REDUCE", shiftMessage.str(), rulePos));
			}
		}

		// Display the reductions for this conflict
		for (conflict::reduce_iterator reduceItem = (*conflict)->first_reduce_item(); reduceItem != (*conflict)->last_reduce_item(); reduceItem++) {
			// Start building the message
			wstring         reduceCode      = L"DETAIL_REDUCE";
			error::severity reductionSev    = error::sev_detail;
			wstringstream	reduceMessage;

            // This is a reduce/reduce conflict if this is the first conflict in the list (ie, no other shift or reduce items)
			if (reduceItem == (*conflict)->first_reduce_item() && (*conflict)->first_shift_item() == (*conflict)->last_shift_item()) {
				// This is a reduce/reduce conflict
				reductionSev = reduceReduceSev;
				reduceCode = L"CONFLICT_REDUCE_REDUCE";
				reduceMessage << L"Reduce/reduce conflict on";
				reduceMessage << L" '" << formatter::to_string(*(*conflict)->token(), *m_Language->grammar(), *m_Language->terminals()) << L"':";
			} else {
				// Displaying additional items
				reduceMessage << L"or reduce:";
			}

			// Add the item being reduced
			reduceMessage << L" " << formatter::to_string(*reduceItem->first->rule(), *m_Language->grammar(), *m_Language->terminals());
			
			// Display the message for this item
			int 		ruleId 	= reduceItem->first->rule()->identifier(*m_Language->grammar());
			position 	rulePos	= m_Language->rule_definition_pos(ruleId);
			cons().report_error(error(reductionSev, m_Language->filename(), reduceCode, reduceMessage.str(), rulePos));

			// For reduce/reduce conflicts, display the context in which the reduction can occur
            set<item_container> displayedNonterminals;
            report_reduce_conflict(reduceItem, reduceItem->first->rule()->nonterminal(), displayedNonterminals, 0);
		}
	}
}

/// \brief Reports errors for a particular reduce conflict (the 'in' and 'to' messages)
void lr_parser_compiler::report_reduce_conflict(lr::conflict::reduce_iterator& reduceItem, item_container nonterminal, set<item_container>& displayedNonterminals, int level) {
    // Only display the set for a given target nonterminal once
    if (displayedNonterminals.find(nonterminal) != displayedNonterminals.end()) {
        return;
    }

    // Mark this nonterminal as being displayed (so we won't iterate over it)
    displayedNonterminals.insert(nonterminal);
    
    // For reduce/reduce conflicts, display the context in which the reduction can occur
    for (conflict::possible_reduce_states::const_iterator possibleState = reduceItem->second.begin(); possibleState != reduceItem->second.end(); possibleState++) {
        // Generate a detail message for this item
        const conflict::lr_item_id& itemId = *possibleState;
        
        // Get the relevant item
        const lr0_item_container& item = (*m_Parser->machine().state_with_id(itemId.first))[itemId.second];

        // Ignore this item if it's not on the correct nonterminal
        if (item->at_end()) continue;
        if (*item->rule()->items()[item->offset()] != *nonterminal) continue;

        // Work out the rule position for this item
        int 		reducedRuleId 	= item->rule()->identifier(*m_Language->grammar());
        position 	reducedRulePos	= m_Language->rule_definition_pos(reducedRuleId);
        
        // Generate a message
        wstringstream detailMessage;
        detailMessage << wstring((level+1)*2, L' ');
        detailMessage << (level==0?L"in: ":L"to: ");
        detailMessage << formatter::to_string(*item, *m_Language->grammar(), *m_Language->terminals());
        
        // Write it out
        cons().report_error(error(error::sev_detail, m_Language->filename(), L"DETAIL_REDUCE_IN", detailMessage.str(), reducedRulePos));
        
        // Display the set for the nonterminals for this rule
        report_reduce_conflict(reduceItem, item->rule()->nonterminal(), displayedNonterminals, level+1);
    }
}
