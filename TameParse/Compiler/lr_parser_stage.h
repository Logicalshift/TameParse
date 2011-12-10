//
//  lr_parser_stage.h
//  Parse
//
//  Created by Andrew Hunter on 27/08/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#ifndef _COMPILER_LR_PARSER_STAGE_H
#define _COMPILER_LR_PARSER_STAGE_H

#include <string>

#include "TameParse/Compiler/compilation_stage.h"
#include "TameParse/Compiler/language_stage.h"
#include "TameParse/Compiler/lexer_stage.h"

#include "TameParse/Dfa/position.h"
#include "TameParse/Language/parser_block.h"
#include "TameParse/Lr/lalr_builder.h"
#include "TameParse/Lr/conflict.h"
#include "TameParse/Lr/ast_parser.h"

namespace compiler {
	///
	/// \brief Compilation stage that takes the results of compiling a grammar and a lexer and produces a LR parser
	///
	class lr_parser_stage : public compilation_stage {
	private:
		/// \brief The language compiler stage
		language_stage* m_Language;

		/// \brief The lexer compiler stage
		lexer_stage* m_LexerCompiler;

		/// \brief The names of the nonterminals that are the start symbols for this language
		std::vector<std::wstring> m_StartSymbols;

		/// \brief Position for each start symbol
		std::vector<dfa::position> m_SymbolStartPosition;

		/// \brief The position of the parser block
		dfa::position m_StartPosition;

        /// \brief The initial states of the parser (one per start symbol once compilation is complete, assuming that there
        /// were no missing symbols)
        std::vector<int> m_InitialStates;
        
		/// \brief The LALR builder that contains the resulting parser
		lr::lalr_builder* m_Parser;
        
        /// \brief The parser tables for the final parser
        lr::parser_tables* m_Tables;

	public:
		/// \brief Constructor, without using a parser block
		lr_parser_stage(console_container& console, const std::wstring& filename, language_stage* languageCompiler, lexer_stage* lexerCompiler, const std::vector<std::wstring>& startSymbols);

		/// \brief Constructure which builds the list of start symbols from a parser block
		lr_parser_stage(console_container& console, const std::wstring& filename, language_stage* languageCompiler, lexer_stage* lexerCompiler, language::parser_block* parserBlock);

		/// \brief Destructor
		~lr_parser_stage();

		/// \brief Compiles the parser specified by the parameters to this stage
		void compile();

	private:
		/// \brief Reports errors for a particular reduce conflict (the 'in' and 'to' messages)
		void report_reduce_conflict(lr::conflict::reduce_iterator& reduceItem, contextfree::item_container nonterminal, std::set<contextfree::item_container>& displayedNonterminals, int level);
        
    public:
        /// \brief Returns the parser built by this stage
        inline lr::lalr_builder* get_parser() { return m_Parser; }
        
        /// \brief Returns the parse tables built by this stage
        inline lr::parser_tables* get_tables() { return m_Tables; }
	};
}

#endif
