//
//  cplusplus.h
//  Parse
//
//  Created by Andrew Hunter on 29/08/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#ifndef _COMPILER_OUTPUT_CPLUSPLUS_H
#define _COMPILER_OUTPUT_CPLUSPLUS_H

#include <string>
#include <iostream>
#include <vector>

#include "TameParse/Compiler/output_stage.h"
#include "TameParse/Dfa/symbol_table.h"

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

        /// \brief Maps item IDs to their class identifiers
        std::map<int, std::string> m_ClassNameForItem;

        /// \brief Reserved words and keywords
        std::set<std::string> m_ReservedWords;

        /// \brief The used class (and other identifier) names for the class (which should not be re-used)
        std::set<std::string> m_UsedClassNames;

	public:
		/// \brief Creates a new output stage
		output_cplusplus(console_container& console, const std::wstring& filename, lexer_stage* lexer, language_stage* language, lr_parser_stage* parser, const std::wstring& filenamePrefix, const std::wstring& className, const std::wstring& namespaceName);

		/// \brief Destructor
		virtual ~output_cplusplus();

	protected:
		/// \brief Returns a valid C++ identifier for the specified symbol name
		virtual std::string get_identifier(const std::wstring& name);

        /// \brief Retrieves or assigns a name for a nonterminal with the specified ID
        virtual std::string name_for_nonterminal(int ntId, const contextfree::item_container& item, const contextfree::grammar& gram, const contextfree::terminal_dictionary& terminals);

		/// \brief Writes out a header to the specified file
		virtual void write_header(const std::wstring& filename, std::ostream* target);

	private:
		/// \brief Writes out the terminal symbols definitions to the header file
		void header_terminal_symbols();

		/// \brief Writes out definitions for the nonterminal symbols to the header file
		void header_nonterminal_symbols();

		/// \brief Writes the symbol map definitions to the header
		void header_symbol_map();

		/// \brief Writes the symbol map definitions to the source file
		void source_symbol_map();

		/// \brief Writes out the header items for the lexer state machine
		void header_lexer_state_machine();

		/// \brief Writes out the source code for the lexer state machine
		void source_lexer_state_machine();

		/// \brief Writes out the header items for the parser tables
		void header_parser_tables();

		/// \brief Writes out the source code for the parser tables
		void source_parser_tables();

		/// \brief Writes out the forward declarations for the classes that represent nonterminals
		void header_ast_forward_declarations();

		/// \brief Writes out the declarations for the classes that represent the AST
		void header_ast_class_declarations();

		/// \brief Writes out the implementations of the AST classes to the source file
		void source_ast_class_definitions();

		/// \brief Writes out the parser actions to the header file
		void header_parser_actions();

		/// \brief Writes out the shift actions to the source file
		void source_shift_actions();

		/// \brief Writes out the reduce actions to the source file
		void source_reduce_actions();

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

		/// \brief Defines the symbols associated with this language
		virtual void define_symbols();

		/// \brief Defines the symbols associated with this language
		virtual void define_lexer_tables();

		/// \brief Defines the parser tables for this language
		virtual void define_parser_tables();
        
		/// \brief Starting to write out the definitions associated with the AST
		virtual void begin_ast_definitions(const contextfree::grammar& grammar, const contextfree::terminal_dictionary& terminals);

		/// \brief Starting to write the AST definitions for a particular terminal symbol
		virtual void begin_ast_terminal(int itemIdentifier, const contextfree::item_container& item);

		/// \brief Finished writing the definitions for a terminal
		virtual void end_ast_terminal();
        
		/// \brief Starting to write the AST definitions for the specified nonterminal
		virtual void begin_ast_nonterminal(int identifier, const contextfree::item_container& item);
        
		/// \brief Starting to write out a rule in the current nonterminal
		virtual void begin_ast_rule(int identifier);
        
		/// \brief Writes out an individual item in the current rule (a nonterminal)
		virtual void rule_item_nonterminal(int nonterminalId, const contextfree::item_container& item);
        
        /// \brief Writes out an individual item in the current rule (a terminal)
        ///
        /// Note the distinction between the item ID, which is part of the grammar, and the
        /// symbol ID (which is part of the lexer and is the same as the value passed to 
        /// terminal_symbol)
        virtual void rule_item_terminal(int terminalItemId, int terminalSymbolId, const contextfree::item_container& item);
        
		/// \brief Finished writing out 
		virtual void end_ast_rule();
        
		/// \brief Finished writing the definitions for a nonterminal
		virtual void end_ast_nonterminal();
        
		/// \brief Finished writing out the AST information
		virtual void end_ast_definitions();
	};
}

#endif
