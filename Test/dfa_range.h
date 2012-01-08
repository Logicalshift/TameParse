//
//  dfa_range.h
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#include "test_fixture.h"

class test_dfa_range : public test_fixture {
public:
    test_dfa_range() : test_fixture("DFA-range") { }
    
protected:
    /// \brief Overridden by subclasses to run all of the tests associated with this fixture
    virtual void run_tests();
};
