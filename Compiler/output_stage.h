//
//  output_stage.h
//  Parse
//
//  Created by Andrew Hunter on 29/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _COMPILER_OUTPUT_STAGE_H
#define _COMPILER_OUTPUT_STAGE_H

#include <string>

#include "Dfa/range.h"

#include "Compiler/compilation_stage.h"
#include "Compiler/language_stage.h"
#include "Compiler/lr_parser_stage.h"
#include "Compiler/lexer_stage.h"

namespace compiler {
	///
	/// \brief Base class for a compilation stage that produces output in a given language
	///
	class output_stage : public compilation_stage {
	private:
		/// \brief The lexer stage that should be compiled
		lexer_stage* m_LexerStage;

		/// \brief The language stage that should be compiled
		language_stage* m_LanguageStage;

		/// \brief The LR parser that should be compiled
		lr_parser_stage* m_ParserStage;

	public:
		/// \brief Creates a new output stage
		output_stage(console_container& console, const std::wstring& filename, lexer_stage* lexer, language_stage* language, lr_parser_stage* parser);

		/// \brief Destructor
		virtual ~output_stage();

		/// \brief Compiles the parser specified by this stage
		///
		/// Subclasses can override this if they want to substantially change the way that the
		/// compiler is generated.
		virtual void compile();

	protected:
		// Higher-level stages of the process

		/// \brief Defines the symbols associated with this language
		virtual void define_symbols();

		/// \brief Writes out the lexer tables (the symbol map and the state table)
		virtual void define_lexer_tables();

	protected:
		// Functions that represent various steps of the output of a language.
		// These are intended to make it easy to write out a file in the specified language.
		// They do nothing in this base class (but they are not abstract)
		//
		// You can override compile() or the define_x methods to get more fine-grained control

		/// \brief About to begin writing out output
		virtual void begin_output();

		/// \brief Finishing writing out output
		virtual void end_output();

		/// \brief The output stage is about to produce a list of terminal symbols
		virtual void begin_terminal_symbols();

		/// \brief Specifies the identifier for the terminal symbol with a given name
		virtual void terminal_symbol(const std::wstring& name, int identifier);

		/// \brief Finished writing out the terminal symbols
		virtual void end_terminal_symbols();

		/// \brief The output stage is about to produce a list of non-terminal symbols
		virtual void begin_nonterminal_symbols();

		/// \brief Specifies the identifier for the non-terminal symbol with a given name
		virtual void nonterminal_symbol(const std::wstring& name, int identifier);

		/// \brief Finished writing out the terminal symbols
		virtual void end_nonterminal_symbols();

		/// \brief Starting to write out the symbol map for the lexer
		virtual void begin_lexer_symbol_map(int maxSetId);

		/// \brief Specifies that a given range of symbols maps to a particular identifier
		virtual void symbol_map(const dfa::range<int>& symbolRange, int identifier);
		
		/// \brief Finishing writing out the symbol map for the lexer
		virtual void end_lexer_symbol_map();

		/// \brief About to begin writing out the lexer tables
		virtual void begin_lexer_state_machine(int numStates);

		/// \brief Starting to write out the transitions for a given state
		virtual void begin_lexer_state(int stateId);

		/// \brief Adds a transition for the current state
		virtual void lexer_state_transition(int symbolSet, int newState);

		/// \brief Finishes writing out a lexer state
		virtual void end_lexer_state();

		/// \brief Finished writing out the lexer table
		virtual void end_lexer_state_machine();

		/// \brief About to write out the list of accepting states for a lexer
		virtual void begin_lexer_accept_table();

		/// \brief The specified state is not an accepting state
		virtual void nonaccepting_state(int stateId);

		/// \brief The specified state is an accepting state
		virtual void accepting_state(int stateId, int acceptSymbolId);

		/// \brief Finished the lexer acceptance table
		virtual void end_lexer_accept_table();
	};
}

#endif
