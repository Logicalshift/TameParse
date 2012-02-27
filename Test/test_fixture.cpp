//
//  test_fixture.cpp
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
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

#include "test_fixture.h"

using namespace std;

/// \brief Creates a new test fixture with the specified name
test_fixture::test_fixture(std::string name) 
: m_Name(name)
, m_TestsRun(0)
, m_TestsFailed(0) {
}

/// \brief Reports on the result of an individual test
void test_fixture::report(std::string test_name, bool result) {
    ++m_TestsRun;
    
    string prefix   = m_Name + "." + test_name;
    string dots     = string(60 - prefix.length(), '.');
    
    if (result) {
        cout << prefix << dots << "ok" << endl;
    } else {
        ++m_TestsFailed;
        cerr << prefix << dots << "FAILED" << endl;
    }
}

/// \brief Reports on the result of an individual test
void test_fixture::report_known_failure(std::string test_name, bool result) {
    ++m_TestsRun;
    
    string prefix   = m_Name + "." + test_name;
    string dots     = string(60 - prefix.length(), '.');
    
    if (result) {
        cout << prefix << dots << "SUCCEEDED (unexpectedly)" << endl;
    } else {
        cerr << prefix << dots << "known failure" << endl;
    }
}

/// \brief Runs this test
void test_fixture::run() {
    cout << endl << "*** TESTS FROM " << m_Name << endl;
    run_tests();
    
    if (m_TestsFailed > 0) {
        cerr << endl << "*** " << m_Name << ": " << m_TestsFailed << "/" << m_TestsRun << " FAILED" << endl;
    }
}
