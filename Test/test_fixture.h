//
//  test_fixture.h
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _TEST_FIXTURE_H
#define _TEST_FIXTURE_H

#include <string>

/// \brief Class that represents a series of tests
class test_fixture {
private:
    /// \brief The name of this test fixture
    std::string m_Name;
    
    /// \brief Number of tests that have been run
    int m_TestsRun;
    
    /// \brief Number of tests that have failed
    int m_TestsFailed;
    
public:
    /// \brief Creates a new test fixture with the specified name
    test_fixture(std::string name);
    
    /// \brief Reports on the result of an individual test
    void report(std::string test_name, bool result);
    
    /// \brief Runs this test
    void run();
    
    /// \brief The number of tests that have been run in this fixture
    inline int count_run() const { return m_TestsRun; }
    
    /// \brief The number of tests that failed in this fixture
    inline int count_failed() const { return m_TestsFailed; }
    
protected:
    /// \brief Overridden by subclasses to run all of the tests associated with this fixture
    virtual void run_tests() = 0;
};

#endif
