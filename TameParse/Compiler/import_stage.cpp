//
//  import_stage.cpp
//  TameParse
//
//  Created by Andrew Hunter on 25/09/2011.
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

#include <stack>
#include <set>
#include <sstream>

#include "TameParse/Compiler/import_stage.h"
#include "TameParse/Compiler/parser_stage.h"

using namespace std;
using namespace yy_language;
using namespace compiler;

/// \brief Creates a new import stage
import_stage::import_stage(console_container& console, const std::wstring& filename, definition_file_container rootFile) 
: compilation_stage(console, filename) {
    // Add the root file to the stage for file list
    wstring realPath = console->real_path(filename);
    
    m_DefinitionForFile[realPath]   = rootFile;
    m_ShortNameForFile[realPath]    = filename;
}

/// \brief Destructor
import_stage::~import_stage() {
}

/// \brief Performs the actions associated with this compilation stage
void import_stage::compile() {
    console_container consContainer = cons_container();
    
    // Create a stack of definitions to look for import statements in
    typedef pair<wstring, definition_file_container> stack_entry;
    
    stack<stack_entry>  toImport;
    set<wstring>        imported;

    // Iterate through the definitions
    for (map<wstring, definition_file_container>::iterator defn = m_DefinitionForFile.begin(); defn != m_DefinitionForFile.end(); ++defn) {
        // Add to the list to be processed
        toImport.push(*defn);
        imported.insert(cons().real_path(defn->first));
    }

    // While there are files to be processed...
    while (!toImport.empty()) {
        // Get the next file
        stack_entry nextFile = toImport.top();
        toImport.pop();

        // Ignore files with no data
        if (!nextFile.second.item()) continue;
        
        // Look for import statements
        for (definition_file::iterator defn = nextFile.second->begin(); defn != nextFile.second->end(); ++defn) {
            if ((*defn)->import()) {
                // Is an import block: get the filename
                wstring importFile  = (*defn)->import()->import_filename();
                wstring realPath    = cons().real_path(importFile);

                // Ignore if the file has already been imported
                if (imported.find(realPath) != imported.end()) continue;

                // Mark as processed
                imported.insert(realPath);

                // Load in this file
                parser_stage importStage(consContainer, importFile);
                importStage.compile();

                // Add to the result
                m_DefinitionForFile[realPath]   = importStage.definition_file();
                m_ShortNameForFile[realPath]    = importFile;

                // Process any imports that the new file might contain
                toImport.push(stack_entry(realPath, importStage.definition_file()));
            }
            
            if ((*defn)->language()) {
                // This is a language block: fetch the name
                wstring name = (*defn)->language()->identifier();
                
                // Report an error if it already exists
                if (m_LanguageBlock.find(name) != m_LanguageBlock.end()) {
                    // Report the secondary definition
                    wstringstream msg;
                    msg << L"The language '" << name << L"' is defined more than once";
                    cons().report_error(error(error::sev_error, m_ShortNameForFile[nextFile.first], L"AMBIGUOUS_LANGUAGE_DEFINITION", msg.str(), (*defn)->start_pos()));
                    
                    // Report some detail
                    cons().report_error(error(error::sev_detail, m_LanguageFile[name], L"AMBIGUOUS_LANGUAGE_ORIGINAL", L"First encountered here", m_LanguageBlock[name]->start_pos()));
                } else {
                    // Store information about this language
                    m_LanguageFile[name]    = nextFile.first;
                    m_LanguageBlock[name]   = (*defn)->language();
                }
            }
        }
    }
    
    // Output some statistics
    size_t numDefns = m_LanguageBlock.size();
    cons().verbose_stream() << L"    Found " << numDefns << L" language definition" << (numDefns == 1?L"":L"s") << endl;
}
