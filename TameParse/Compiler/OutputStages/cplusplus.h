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

        /// \brief The last grammar supplied to this object
        const contextfree::grammar* m_Grammar;

        /// \brief The last terminal dictionary supplied to this object
        const contextfree::terminal_dictionary* m_Terminals;

        /// \brief Reserved words and keywords
        std::set<std::string> m_ReservedWords;

        /// \brief The used class (and other identifier) names for the class (which should not be re-used)
        std::set<std::string> m_UsedClassNames;

        /// \brief Within a single rule, the used names for the various items
        std::set<std::string> m_UsedRuleItems;

        /// \brief Within a nonterminal, the used names for the various items
        std::set<std::string> m_UsedNtItems;

        /// \brief Name of the current nonterminal class that's being defined
        std::string m_CurrentNonterminal;

        /// \brief Identifier of the nonterminal that's currently being defined
        int m_CurrentNonterminalId;

        /// \brief The kind of the current nonterminal
        contextfree::item::kind m_CurrentNonterminalKind;

        /// \brief Identifier for the rule that's currently being defined
        int m_CurrentRuleId;

        /// \brief The item names for each of the items in the current rule
        std::vector<std::string> m_CurrentRuleNames;

        /// \brief The type names for each of the items in the current rule
        std::vector<std::string> m_CurrentRuleTypes;

        /// \brief Stringstream used to build up the forward declarations for the nonterminal classes
        ///
        /// C++ is hopeless and can't clear items, so we have to use tedious pointers
        std::stringstream* m_NtForwardDeclarations;

        /// \brief Stringstream used to build up the declarations of the classes that contain the nonterminal definitions themselves
        std::stringstream* m_NtClassDefinitions;

        /// \brief String stream used to build up the switch statement used to declare the parser shift actions
        std::stringstream* m_ShiftDefinitions;

        /// \brief String stream used to build up the switch statement used to declare the parser reduce actions
        std::stringstream* m_ReduceDefinitions;

        /// \brief String stream used to build up the switch statement used to declare the pos() function
        std::stringstream* m_PosDefinitions;

        /// \brief String stream used to build up the switch statement used to declare the final_pos() function
        std::stringstream* m_FinalPosDefinitions;

	public:
		/// \brief Creates a new output stage
		output_cplusplus(console_container& console, const std::wstring& filename, lexer_stage* lexer, language_stage* language, lr_parser_stage* parser, const std::wstring& filenamePrefix, const std::wstring& className, const std::wstring& namespaceName);

		/// \brief Destructor
		virtual ~output_cplusplus();

	protected:
		/// \brief Returns a valid C++ identifier for the specified symbol name
		virtual std::string get_identifier(const std::wstring& name);

		/// \brief Returns a valid C++ name for a grammar rule
		virtual std::string name_for_rule(const contextfree::rule_container& rule, const contextfree::grammar& gram, const contextfree::terminal_dictionary& terminals);

		/// \brief Returns a valid C++ name for an EBNF item
		virtual std::string name_for_ebnf_item(const contextfree::ebnf& ebnfItem, const contextfree::grammar& gram, const contextfree::terminal_dictionary& terminals);
        
        /// \brief Returns a valid C++ name for the specified item
        ///
        /// This can be treated as a base name for getting names for nonterminals with particular identifiers
        virtual std::string name_for_item(const contextfree::item_container& item, const contextfree::grammar& gram, const contextfree::terminal_dictionary& terminals);

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

		/// \brief Starting to write out the definitions associated with the parser
		virtual void begin_parser_definitions();

		/// \brief Supplies the parser tables generated by the compiler
		virtual void parser_tables(const lr::lalr_builder& builder, const lr::parser_tables& tables);

		/// \brief Finished the parser definitions
		virtual void end_parser_definitions();
        
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
