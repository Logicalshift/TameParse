//
//  contextfree_firstset.h
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#include "test_fixture.h"

/// Tests for checking that the grammar can calculate first sets
class test_contextfree_firstset : public test_fixture {
public:
    test_contextfree_firstset() : test_fixture("contextfree-firstset") { }
    
    virtual void run_tests();
};
