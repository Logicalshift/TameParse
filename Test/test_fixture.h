//
//  test_fixture.h
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  
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
    
    /// \brief Reports on the result of an individual test (which is known to fail)
    void report_known_failure(std::string test_name, bool result);
    
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
