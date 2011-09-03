//
//  cplusplus.h
//  Parse
//
//  Created by Andrew Hunter on 29/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _COMPILER_OUTPUT_CPLUSPLUS_H
#define _COMPILER_OUTPUT_CPLUSPLUS_H

#include <string>
#include <iostream>
#include <vector>

#include "Compiler/output_stage.h"
#include "Dfa/symbol_table.h"

namespace compiler {
	///
	/// \brief Output stage that generates a C++ file
	///
	class output_cplusplus : public output_stage {
	private:
		/// \brief The prefix for the output files
		std::wstring m_FilenamePrefix;

		/// \brief The name of the class that should be generated
		std::wstring m_ClassName;

		/// \brief The namespace the class should be put in (or the empty string for no namespace)
		std::wstring m_Namespace;

		/// \brief The source file
		std::ostream* m_SourceFile;

		/// \brief The header file
		std::ostream* m_HeaderFile;
        
        /// \brief Number of times a terminal symbol with a particular name has been used
        std::map<std::string, int> m_TerminalSymbolCount;
        
        /// \brief Number of times a nonterminal symbol with a particular name has been used
        std::map<std::string, int> m_NonterminalSymbolCount;

        /// \brief The symbol table built up from the symbols supplied to this object
        dfa::symbol_table<wchar_t>* m_SymbolLevels;

        /// \brief Current lexer entry position
        int m_LexerEntryPos;

        /// \brief The state that's being written out by the lexer
        int m_LexerCurrentState;

        /// \brief Maps lexer states to their offset in the state machine table
        std::vector<int> m_StateToEntryOffset;

	public:
		/// \brief Creates a new output stage
		output_cplusplus(console_container& console, const std::wstring& filename, lexer_stage* lexer, language_stage* language, lr_parser_stage* parser, const std::wstring& filenamePrefix, const std::wstring& className, const std::wstring& namespaceName);

		/// \brief Destructor
		virtual ~output_cplusplus();

	protected:
		/// \brief Returns a valid C++ identifier for the specified symbol name
		std::string get_identifier(const std::wstring& name);

		/// \brief Writes out a header to the specified file
		virtual void write_header(const std::wstring& filename, std::ostream* target);

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

		/// \brief Finished all of the lexer definitions
		virtual void end_lexer_definitions();
	};
}

#endif
