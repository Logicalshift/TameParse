//
//  test_fixture.cpp
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>

#include "test_fixture.h"

using namespace std;

/// \brief Creates a new test fixture with the specified name
test_fixture::test_fixture(std::string name) 
: m_Name(name) {
}

/// \brief Reports on the result of an individual test
void test_fixture::report(std::string test_name, bool result) {
    string prefix   = m_Name + "." + test_name;
    string dots     = string(60 - prefix.length(), '.');
    
    if (result) {
        cout << prefix << dots << "ok" << endl;
    } else {
        cerr << prefix << dots << "FAILED" << endl;
    }
}

/// \brief Runs this test
void test_fixture::run() {
    cout << endl << "*** TESTS FROM " << m_Name << endl;
    run_tests();
}
