//
//  test_stage.cpp
//  TameParse
//
//  Created by Andrew Hunter on 22/10/2011.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <sstream>

#include "TameParse/Compiler/test_stage.h"
#include "TameParse/Language/test_block.h"

using namespace std;
using namespace dfa;
using namespace lr;
using namespace language;
using namespace compiler;

/// \brief Creates a new test stage that will run the tests in the specified definition file
test_stage::test_stage(console_container& console, const std::wstring& filename, const language::definition_file_container& definition, const import_stage* import)
: compilation_stage(console, filename)
, m_Definition(definition)
, m_Import(import) {
}

/// \brief Destructor
test_stage::~test_stage() {
	// Delete the lexers
	for (lexer_map::iterator deadLexer = m_Lexers.begin(); deadLexer != m_Lexers.end(); deadLexer++) {
		if (deadLexer->second) {
	 		delete deadLexer->second;
		}
	}
	m_Lexers.clear();

	// Delete the languages
	for (language_map::iterator deadLanguage = m_Languages.begin(); deadLanguage != m_Languages.end(); deadLanguage++) {
	 	if (deadLanguage->second) {
		 	delete deadLanguage->second;
	 	}
	}
	m_Languages.clear();
}

/// \brief Retrieves the language stage for the language with the specified name
language_stage* test_stage::get_language(const std::wstring& languageName, const dfa::position& pos) {
	// Try to retrieve the existing language
	language_map::iterator found = m_Languages.find(languageName);
	if (found != m_Languages.end()) {
		return found->second;
	}

	// Retrieve the language for this test
	const language_block*	language 		= m_Import->language_with_name(languageName);
	wstring                 languageFile    = m_Import->file_with_language(languageName);

	// Error if the language could not be found
	if (!language) {
        wstringstream msg;
        msg << L"Unable to find language '" << languageName << "'";
		cons().report_error(error(error::sev_error, filename(), L"CANT_FIND_LANGUAGE", msg.str(), pos));
		return m_Languages[languageName] = NULL;
	}

	// Create a new language stage
    console_container   cons    = cons_container();
	language_stage*     stage   = new language_stage(cons, languageFile, language, m_Import);

	// Compile it
	stage->compile();

	// TODO: return NULL if there was a compile error

	// Return it
	return m_Languages[languageName] = stage;
}

/// \brief Retrieves the lexer for the language with the specified name
lexer_stage* test_stage::get_lexer(const std::wstring& languageName, const dfa::position& pos) {
	// Try to retrieve the existing lexer
	lexer_map::iterator found = m_Lexers.find(languageName);
	if (found != m_Lexers.end()) {
		return found->second;
	}

	// Get the language stage corresponding to this lexer
	language_stage* language 		= get_language(languageName, pos);
	wstring    		languageFile    = m_Import->file_with_language(languageName);
	if (!language) {
		// Value is null if the language couldn't be generated for any reason
		return m_Lexers[languageName] = NULL;
	}

	// Create the lexer stage for this language
    console_container   cons    = cons_container();
	lexer_stage*        stage   = new lexer_stage(cons, languageFile, language);

	// Compile it
	stage->compile();

	// TODO: return NULL if there was a compile error

	// Return it
	return m_Lexers[languageName] = stage;
}

/// \brief Retrieves the parser for the language with the specified name
lr_parser_stage* test_stage::get_parser(const std::wstring& languageName, const std::wstring& nonterminalName, const dfa::position& pos) {
	// Try to retrieve the existing parser
	pair<wstring, wstring> parserName(languageName, nonterminalName);

	parser_map::iterator found = m_Parsers.find(parserName);
	if (found != m_Parsers.end()) {
		return found->second;
	}

	// Get the name of the file that the language is defined in
	wstring    		languageFile    = m_Import->file_with_language(languageName);

	// Get the lexer for this language
	lexer_stage*	lexer = get_lexer(languageName, pos);

	// Get the language block
	language_stage* language = get_language(languageName, pos);

	if (!lexer || !language) {
		// Value is null if the language couldn't be generated for any reason
		return m_Parsers[parserName] = NULL;
	}

	// Create a parser for this language
	vector<wstring> 	startSymbols;
	console_container	cons(cons_container());
	startSymbols.push_back(nonterminalName);

	lr_parser_stage* parser = new lr_parser_stage(cons, languageFile, language, lexer, startSymbols);

	// Compile it
	parser->compile();

	// TODO: return NULL if there was a compile error

	// Return it
	return m_Parsers[parserName] = parser;
}

/// \brief Performs the actions associated with this compilation stage
void test_stage::compile() {
	// Sanity check
	if (!m_Definition.item()) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_MISSING_DEFINITION", L"Definition for testing stage not specified", position(-1, -1, -1)));
		return;
	}

	if (!m_Import) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_MISSING_IMPORT", L"Imports for testing stage not specified", position(-1, -1, -1)));
		return;
	}
    
    // Output the stage name
    cons().verbose_stream() << L"  = Running tests" << endl;

	// Iterate through the definitions
    bool firstTestSet = true;
    
	for (definition_file::iterator defn = m_Definition->begin(); defn != m_Definition->end(); defn++) {
		// Retrieve the tests for this block
		const test_block* tests = (*defn)->test();

		// Ignore blocks that aren't test blocks
		if (!tests) continue;

		// Retrieve the language for this test
		const language_block*	language 		= m_Import->language_with_name(tests->language());
		wstring                 languageFile    = m_Import->file_with_language(tests->language());

		// Error if the language could not be found
		if (!language) {
            wstringstream msg;
            msg << L"Unable to find language '" << tests->language() << "'";
			cons().report_error(error(error::sev_error, filename(), L"CANT_FIND_LANGUAGE", msg.str(), tests->start_pos()));
			continue;
		}
        
        // Message for these tests
        wstringstream testMessages;
        if (!firstTestSet) {
            testMessages << endl;
        }
        testMessages << L"    Tests for " << tests->language() << endl;
        
        // Passed/failed ratios for these tests
        int passed  = 0;
        int failed  = 0;

        // Get the lexer for this language
        lexer_stage* lexer = get_lexer(tests->language(), tests->start_pos());
        if (!lexer || !lexer->get_lexer()) {
        	// Do nothing more if the lexer doesn't exist (failed to build)
        	// The lexer compiler should have reported an error so this should be OK
        	continue;
        }

        // Test IDs for given test names
        map<wstring, int> testIds;

        // Run the tests themselves
        for (test_block::iterator testDefn = tests->begin(); testDefn != tests->end(); testDefn++) {
        	// Compile a language for this nonterminal if one doesn't exist already
        	// TODO: deal with nonterminals in other languages
        	lr_parser_stage* parserStage = get_parser(tests->language(), (*testDefn)->nonterminal(), tests->start_pos());

        	// Get the text to be tested
        	// TODO: deal with 'from' items
        	wstringstream testText((*testDefn)->test_string());

      		// Create the parser
      		simple_parser parser(parserStage->get_tables(), false);

        	// Create the lexeme stream
        	lexeme_stream* stream = lexer->get_lexer()->create_stream_from(testText);

        	// Create the parser state
        	simple_parser::state* parseState = parser.create_parser(new simple_parser_actions(stream));

        	// Run the test
        	bool result = parseState->parse();

        	// Done with the parser
        	delete parseState;

        	// Work out a name for this test
        	wstringstream testName;

        	// The name starts <language name>.
        	testName << tests->language() << L".";

        	// Followed up by the identifier or the nonterminal if none is available
        	if (!(*testDefn)->identifier().empty()) {
        		// Use the test identifier instead if one is supplied
        		testName << (*testDefn)->identifier();
        	} else {
        		// Use the nonterminal
        		testName << (*testDefn)->nonterminal();
        	}

        	// Finally, get the identifier so identically named tests can be handled
        	// TODO: maybe don't append this for unique tests?
        	int identifier = ++testIds[testName.str()];
        	testName << L"." << identifier;

        	// Write out a message to the result string
        	wstring finalName(testName.str());
        	testMessages 	<< L"      " << finalName 
        					<< wstring(54 - finalName.size(), L'.')
        					<< (result?L"ok":L"FAILED")
        					<< endl;

        	// Add to the count of successful/failed tests
        	if (result) {
        		passed++;
        	} else {
        		failed++;
        	}
        }
        
        // Write the messages. We use the verbose stream if the tests mostly passed
        if (failed) {
            cons().message_stream() << testMessages.str();
        } else {
            cons().verbose_stream() << testMessages.str();
        }
        
        // Final message
        cons().message_stream() << L"    " << tests->language() << L": " << passed << L"/" << (passed+failed) << L" passed" << endl;
        
        // Warning if there are no tests for this language
        if (passed == 0 && failed == 0) {
            cons().report_error(error(error::sev_warning, filename(), L"NO_TESTS_TO_RUN", L"Found no tests to run", tests->start_pos()));
        }
        
        // No longer on the first test set
        firstTestSet = false;
	}
}
