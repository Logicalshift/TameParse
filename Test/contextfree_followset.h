//
//  contextfree_followset.h
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#include "test_fixture.h"

/// Tests for checking that the grammar can calculate follow sets
class test_contextfree_followset : public test_fixture {
public:
    test_contextfree_followset() : test_fixture("contextfree-followset") { }
    
    virtual void run_tests();
};
