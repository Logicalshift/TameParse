//
//  parser_stage.cpp
//  TameParse
//
//  Created by Andrew Hunter on 24/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <sstream>

#include "TameParse/Util/utf8reader.h"
#include "TameParse/Language/language_parser.h"
#include "TameParse/Compiler/parser_stage.h"

using namespace std;
using namespace util;
using namespace dfa;
using namespace language;
using namespace compiler;

/// \brief Creates a new compilation stage which will use the specified console object
parser_stage::parser_stage(console_container& console, const std::wstring& filename) 
: compilation_stage(console, filename) {
}

/// \brief Performs the actions associated with this compilation stage
void parser_stage::compile() {
    // Message to say what we're doing
    cons().verbose_stream() << " = Reading " << filename() << endl;

    // Try to open the file
    istream* fileStream = cons().open_file(filename());
    
    // Report an error if the file could not be opened
    if (!fileStream || !fileStream->good()) {
        cons().report_error(error(error::sev_fatal, filename(), L"CANT_OPEN_FILE", L"Cannot read from file", position(-1, -1, -1)));
        return;
    }
    
    // Attempt to parse the file
    m_Parser.set_filename(filename());
    bool parsedOk = m_Parser.parse(*fileStream);
    
    // Report an error if we failed to parse OK
    if (!parsedOk) {
        // Fetch the errors
        typedef language_parser::error_list error_list;
        const error_list& errors = m_Parser.errors();
        
        if (!errors.empty()) {
            // Report the errors
            for (error_list::const_iterator nextError = errors.begin(); nextError != errors.end(); nextError++) {
                cons().report_error(*nextError);
            }
        } else {
            // Whoops, couldn't report the syntax error
            cons().report_error(error(error::sev_bug, filename(), L"BUG_CANT_REPORT_SYNTAX", L"Unknown syntax error", position(-1, -1, -1)));
        }
        return;
    }
    
    // Done
    delete fileStream;
}
