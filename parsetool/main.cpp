//
//  main.cpp
//  parsetool
//
//  Created by Andrew Hunter on 24/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/TameParse.h"
#include "boost_console.h"

#include <iostream>

using namespace std;
using namespace dfa;
using namespace tameparse;
using namespace compiler;

int main (int argc, const char * argv[])
{
    // Create the console
    boost_console       console(argc, argv);
    console_container   cons(&console, false);
    
    try {
        // Give up if the console is set not to start
        if (!console.can_start()) {
            return 1;
        }
        
        // Startup message
        console.message_stream() << L"TameParse " << version::major_version << "." << version::minor_version << "." << version::revision << endl;
        console.verbose_stream() << endl;
        
        // Parse the input file
        parser_stage parserStage(cons, console.input_file());
        parserStage.compile();
        
        // Stop if we have an error
        if (console.exit_code()) {
            return console.exit_code();
        }
        
        // The definition file should exist
        if (!parserStage.definition_file().item()) {
            console.report_error(error(error::sev_bug, console.input_file(), L"BUG_NO_FILE_DATA", L"File did not produce any data", position(-1, -1, -1)));
            return error::sev_bug;
        }
        
        // Parse any imported files
        import_stage importStage(cons, console.input_file(), parserStage.definition_file());
        importStage.compile();

        // Stop if we have an error
        if (console.exit_code()) {
            return console.exit_code();
        }

        // Done
        return console.exit_code();
    } catch (...) {
        console.report_error(error(error::sev_bug, L"", L"BUG_UNCAUGHT_EXCEPTION", L"Uncaught exception", position(-1, -1, -1)));
        throw;
    }
}

