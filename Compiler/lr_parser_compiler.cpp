//
//  lr_parser_compiler.cpp
//  Parse
//
//  Created by Andrew Hunter on 27/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "lr_parser_compiler.h"

using namespace std;
using namespace dfa;
using namespace lr;
using namespace compiler;

/// \brief Constructor
lr_parser_compiler::lr_parser_compiler(console_container& console, const std::wstring& filename, language_compiler* languageCompiler, lexer_compiler* lexerCompiler, const vector<wstring>& startSymbols)
: compilation_stage(console, filename)
, m_Language(languageCompiler)
, m_LexerCompiler(lexerCompiler)
, m_StartSymbols(startSymbols)
, m_Parser(NULL) {
	
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
	// Recycle the parser generator if it already exists
	if (m_Parser) {
		delete m_Parser;
		m_Parser = NULL;
	}

	// Sanity check (language)
	if (!m_Language) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE", L"Language compiler stage was not supplied to parser stage", position(-1, -1, -1)));
		return;
	}

	if (!m_Language->ndfa()) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE_NDFA", L"Language compiler stage has not generated a lexer", position(-1, -1, -1)));
		return;
	}

	if (!m_Language->terminals()) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE_TERMINALS", L"Language compiler stage has not generated a terminal dictionary", position(-1, -1, -1)));		
		return;
	}

	if (!m_Language->weak_symbols()) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE_WEAK_SYMBOLS", L"Language compiler stage has not generated a weak symbols action table", position(-1, -1, -1)));
		return;
	}

	if (!m_Language->grammar()) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE_GRAMMAR", L"Language compiler stage has not generated a grammar", position(-1, -1, -1)));		
		return;
	}

	// Sanity check (lexer)
	if (!m_LexerCompiler) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LEXER", L"Lexer compiler stage was not supplied to parser stage", position(-1, -1, -1)));
		return;
	}

	if (!m_LexerCompiler->dfa()) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LEXER_DFA", L"Lexer compiler stage has not generate a DFA", position(-1, -1, -1)));
		return;
	}

	if (!m_LexerCompiler->weak_symbols()) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LEXER_DFA", L"Lexer compiler stage has not generate a weak symbols rewriter", position(-1, -1, -1)));
		return;
	}

	// Create a new parser builder
	m_Parser = new lalr_builder(*m_Language->grammar(), *m_Language->terminals());
}
