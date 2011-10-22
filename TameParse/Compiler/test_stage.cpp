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
using namespace language;
using namespace compiler;

/// \brief Creates a new test stage that will run the tests in the specified definition file
test_stage::test_stage(console_container& console, const std::wstring& filename, const language::definition_file_container& definition, const import_stage* import)
: compilation_stage(console, filename)
, m_Definition(definition)
, m_Import(import) {
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
        testMessages << endl << L"    Tests for " << tests->language() << endl;
        
        // Passed/failed ratios for these tests
        int passed  = 0;
        int failed  = 0;

        // Run the tests themselves
        
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
	}
}
