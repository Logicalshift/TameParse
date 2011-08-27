//
//  lr_parser_compiler.h
//  Parse
//
//  Created by Andrew Hunter on 27/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _COMPILER_LR_PARSER_COMPILER_H
#define _COMPILER_LR_PARSER_COMPILER_H

#include <string>

#include "Compiler/compilation_stage.h"
#include "Compiler/language_compiler.h"
#include "Compiler/lexer_compiler.h"

#include "Dfa/position.h"
#include "Language/parser_block.h"
#include "Lr/lalr_builder.h"

namespace compiler {
	///
	/// \brief Compilation stage that takes the results of compiling a grammar and a lexer and produces a LR parser
	///
	class lr_parser_compiler : public compilation_stage {
	private:
		/// \brief The language compiler stage
		language_compiler* m_Language;

		/// \brief The lexer compiler stage
		lexer_compiler* m_LexerCompiler;

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

	public:
		/// \brief Constructor, without using a parser block
		lr_parser_compiler(console_container& console, const std::wstring& filename, language_compiler* languageCompiler, lexer_compiler* lexerCompiler, const std::vector<std::wstring>& startSymbols);

		/// \brief Constructure which builds the list of start symbols from a parser block
		lr_parser_compiler(console_container& console, const std::wstring& filename, language_compiler* languageCompiler, lexer_compiler* lexerCompiler, language::parser_block* parserBlock);

		/// \brief Destructor
		~lr_parser_compiler();

		/// \brief Compiles the parser specified by the parameters to this stage
		void compile();
	};
}

#endif
