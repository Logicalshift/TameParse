//
//  cplusplus.h
//  Parse
//
//  Created by Andrew Hunter on 29/08/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
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
		virtual void begin_terminal_symbols(const contextfree::grammar& gram);

		/// \brief Specifies the identifier for the terminal symbol with a given name
		virtual void terminal_symbol(const std::wstring& name, int identifier);

		/// \brief Finished writing out the terminal symbols
		virtual void end_terminal_symbols();

		/// \brief The output stage is about to produce a list of non-terminal symbols
		virtual void begin_nonterminal_symbols(const contextfree::grammar& gram);

		/// \brief Specifies the identifier for the non-terminal symbol with a given name
		virtual void nonterminal_symbol(const std::wstring& name, int identifier, const contextfree::item_container& item);

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
