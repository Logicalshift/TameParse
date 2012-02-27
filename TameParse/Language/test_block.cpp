//
//  test_block.cpp
//  TameParse
//
//  Created by Andrew Hunter on 18/10/2011.
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

#include "TameParse/Language/test_block.h"

using namespace std;
using namespace language;

/// \brief Creates a new test block
test_block::test_block(const std::wstring& languageIdentifier, position start, position end) 
: m_Language(languageIdentifier)
, block(start, end) {
}

/// \brief Creates a new test block by copying an old one
test_block::test_block(const test_block& copyFrom) {
    (*this) = copyFrom;
}

/// \brief Assigns the content of this block
test_block& test_block::operator=(const test_block& assignFrom) {
    // Can't assign to ourselves
    if (&assignFrom == this) return *this;

    // Clear out this object
    for (test_definition_list::iterator defn = m_TestDefinitions.begin(); defn != m_TestDefinitions.end(); ++defn) {
        delete *defn;
    }
    m_TestDefinitions.clear();

    // Assign this object
    m_Language = assignFrom.m_Language;

    for (test_block::iterator defn = assignFrom.begin(); defn != assignFrom.end(); ++defn) {
        m_TestDefinitions.push_back(new test_definition(**defn));
    }
    
    return *this;
}

/// \brief Destructor
test_block::~test_block() {
    // Free the definitions
    for (test_definition_list::iterator defn = m_TestDefinitions.begin(); defn != m_TestDefinitions.end(); ++defn) {
        delete *defn;
    }
    m_TestDefinitions.clear();
}

/// \brief Adds a new test definition (the definition will become owned by this object)
void test_block::add_test_definition(test_definition* newDefinition) {
    m_TestDefinitions.push_back(newDefinition);
}
