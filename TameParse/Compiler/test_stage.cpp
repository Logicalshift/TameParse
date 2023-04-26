//
//  test_stage.cpp
//  TameParse
//
//  Created by Andrew Hunter on 22/10/2011.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the \"Software\"), to 
//  deal in the Software without restriction, including without limitation the 
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
//  sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
//  IN THE SOFTWARE.
//

#include <sstream>

#include "TameParse/Util/utf8reader.h"
#include "TameParse/Compiler/test_stage.h"
#include "TameParse/Language/test_block.h"

using namespace std;
using namespace util;
using namespace dfa;
using namespace lr;
using namespace yy_language;
using namespace compiler;

/// \brief Creates a new test stage that will run the tests in the specified definition file
test_stage::test_stage(console_container& console, const std::wstring& filename, const yy_language::definition_file_container& definition, const import_stage* import)
: compilation_stage(console, filename)
, m_Definition(definition)
, m_Import(import) {
}

/// \brief Destructor
test_stage::~test_stage() {
    // Delete the lexers
    for (lexer_map::iterator deadLexer = m_Lexers.begin(); deadLexer != m_Lexers.end(); ++deadLexer) {
        if (deadLexer->second) {
            delete deadLexer->second;
        }
    }
    m_Lexers.clear();

    // Delete the languages
    for (language_map::iterator deadLanguage = m_Languages.begin(); deadLanguage != m_Languages.end(); ++deadLanguage) {
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
    const language_block*   language        = m_Import->language_with_name(languageName);
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
    language_stage* language        = get_language(languageName, pos);
    wstring         languageFile    = m_Import->file_with_language(languageName);
    if (!language) {
        // Value is null if the language couldn't be generated for any reason
        return m_Lexers[languageName] = NULL;
    }
    
    // Report any unused symbols in this language
    language->report_unused_symbols();

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
    wstring         languageFile    = m_Import->file_with_language(languageName);

    // Get the lexer for this language
    lexer_stage*    lexer = get_lexer(languageName, pos);

    // Get the language block
    language_stage* language = get_language(languageName, pos);

    if (!lexer || !language) {
        // Value is null if the language couldn't be generated for any reason
        return m_Parsers[parserName] = NULL;
    }

    // Create a parser for this language
    vector<wstring>     startSymbols;
    console_container   cons(cons_container());
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
    bool firstTestSet   = true;
    bool runAnyTests    = false;

    for (definition_file::iterator defn = m_Definition->begin(); defn != m_Definition->end(); ++defn) {
        // Retrieve the tests for this block
        const test_block* tests = (*defn)->test();

        // Ignore blocks that aren't test blocks
        if (!tests) continue;

        // Flag up that we've hit at least one test block
        runAnyTests = true;

        // Retrieve the language for this test
        const language_block*   language        = m_Import->language_with_name(tests->language());
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
        for (test_block::iterator testDefn = tests->begin(); testDefn != tests->end(); ++testDefn) {
            // Compile a language for this nonterminal if one doesn't exist already
            // TODO: deal with nonterminals in other languages
            lr_parser_stage* parserStage = get_parser(tests->language(), (*testDefn)->nonterminal(), tests->start_pos());

            // Get the text to be tested
            // TODO: deal with 'from' items
            wstringstream testText;

            if ((*testDefn)->type() == test_definition::match_from_file) {
                // Read from the supplied file (which we assume is in UTF-8 format)
                istream* fromFile = cons().open_file((*testDefn)->test_string());

                // Error if the file doesn't exist
                if (!fromFile) {
                    cons().report_error(error(error::sev_error, (*testDefn)->test_string(), L"TEST_FILE_NOT_FOUND", L"File not found", position(-1, -1, -1)));
                    continue;
                }

                // Read in as UTF-8
                utf8reader reader(fromFile, true);

                for (;;) {
                    // Get the next character
                    wchar_t nextChar;
                    reader.get(nextChar);

                    // Stop at the end (or if the stream goes ungood for any reason)
                    if (!reader.good()) break;

                    // Store this character
                    testText << nextChar;
                }
            } else {
                // Just use the literal test string
                testText << (*testDefn)->test_string();
            }

            // Create the parser
            simple_parser parser(parserStage->get_tables(), false);

            // Create the lexeme stream
            lexeme_stream* stream = lexer->get_lexer()->create_stream_from(testText);

            // Create the parser state
            simple_parser::state* parseState = parser.create_parser(new simple_parser_actions(stream));

            // Run the test
            bool result = parseState->parse();

            // The result is inverted if the test type is a no match test
            if ((*testDefn)->type() == test_definition::no_match) {
                // We expect to not be able to parse this
                result = !result;
            }

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
            testMessages    << L"      " << finalName 
                            << wstring(54 - finalName.size(), L'.')
                            << (result?L"ok":L"FAILED")
                            << endl;

            // For 'from' tests, report the line number of any failure
            if (!result && (*testDefn)->type() == test_definition::match_from_file) {
                position failPos(-1, -1, -1);
                if (parseState->look().item()) {
                    failPos = parseState->look()->pos();
                }

                cons().report_error(error(error::sev_warning, (*testDefn)->test_string(), L"TEST_SYNTAX_ERROR", L"Syntax error in test file", failPos));
            } else if (!result) {
                // For other tests, report which line failed
                cons().report_error(error(error::sev_error, filename(), L"THIS_TEST_FAILED", L"Test failed", (*testDefn)->test_string_position()));

                // Report where in the test the failure occurred if we can
                if ((*testDefn)->type() != test_definition::no_match) {
                    position failPos = (*testDefn)->test_string_position();

                    if (parseState->look().item()) {
                        // Failed at a specific item
                        position itemPos = parseState->look()->pos();

                        // Adjust the position
                        failPos = position(failPos.offset() + itemPos.offset(), failPos.line() + itemPos.line(), (itemPos.line()==0?failPos.column():0)+itemPos.column());
                    } else {
                        // Failed at the end of the definition
                        failPos = (*testDefn)->end_pos();
                    }

                    // Report the failure
                    cons().report_error(error(error::sev_detail, filename(), L"FAILURE_POSITION", L"Parse error", failPos));
                }
            }

            // Add to the count of successful/failed tests
            if (result) {
                ++passed;
            } else {
                ++failed;
            }

            // Done with the parser
            delete parseState;
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
            cons().report_error(error(error::sev_warning, filename(), L"NO_TESTS_TO_RUN", L"Found an empty test block", tests->start_pos()));
        }

        // Test failures give an error message
        if (failed != 0) {
            wstringstream msg;
            msg << failed << L" tests failed while testing " << tests->language();
            cons().report_error(error(error::sev_error, filename(), L"TESTS_FAILED", msg.str(), tests->start_pos()));
        }
        
        // No longer on the first test set
        firstTestSet = false;
    }

    // It's an error to use run-tests on any file with no test blocks
    if (!runAnyTests) {
        cons().report_error(error(error::sev_error, filename(), L"NO_TEST_BLOCKS", L"Could not find any tests to run in this file", position(-1, -1, -1)));
    }
}
