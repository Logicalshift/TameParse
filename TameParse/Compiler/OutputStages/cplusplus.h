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
		virtual std::string get_identifier(const std::wstring& name, bool allowReserved = false);

        /// \brief Retrieves or assigns a class name for an item with the specified ID
        virtual std::string class_name_for_item(const contextfree::item_container& item);

		/// \brief Writes out a header to the specified file
		virtual void write_header(const std::wstring& filename, std::ostream* target);

		/// \brief Returns true if the specified name should be considered 'valid'
		///
		/// This is used when generating unique names for rules (and may be used in
		/// other places where a unique name is required)
		virtual bool name_is_valid(const std::wstring& name);

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

		/// \brief Writes out the AST tables
		virtual void define_ast_tables();
   	};
}

#endif
