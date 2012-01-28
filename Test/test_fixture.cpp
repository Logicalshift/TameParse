//
//  test_fixture.cpp
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
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
