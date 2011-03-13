//
//  test_fixture.h
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <string>

/// \brief Class that represents a series of tests
class test_fixture {
private:
    /// \brief The name of this test fixture
    std::string m_Name;
    
public:
    /// \brief Creates a new test fixture with the specified name
    test_fixture(std::string name);
    
    /// \brief Reports on the result of an individual test
    void report(std::string test_name, bool result);
    
    /// \brief Runs this test
    void run();
    
protected:
    /// \brief Overridden by subclasses to run all of the tests associated with this fixture
    virtual void run_tests() = 0;
};
