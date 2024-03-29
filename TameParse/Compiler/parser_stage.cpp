//
//  parser_stage.cpp
//  TameParse
//
//  Created by Andrew Hunter on 24/09/2011.
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

#include <iostream>
#include <sstream>

#include "TameParse/Util/utf8reader.h"
#include "TameParse/Language/language_parser.h"
#include "TameParse/Compiler/parser_stage.h"

using namespace std;
using namespace util;
using namespace dfa;
using namespace yy_language;
using namespace compiler;

/// \brief Creates a new compilation stage which will use the specified console object
parser_stage::parser_stage(console_container& console, const std::wstring& filename) 
: compilation_stage(console, filename) {
}

/// \brief Performs the actions associated with this compilation stage
void parser_stage::compile() {
    // Message to say what we're doing
    cons().verbose_stream() << "  = Reading " << filename() << endl;

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
            for (error_list::const_iterator nextError = errors.begin(); nextError != errors.end(); ++nextError) {
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
