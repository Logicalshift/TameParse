//
//  language_bootstrap.h
//  Parse
//
//  Created by Andrew Hunter on 30/05/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#include "test_fixture.h"

/// Tests for checking that the bootstrap language works
class test_language_bootstrap : public test_fixture {
public:
    test_language_bootstrap() : test_fixture("language-bootstrap") { }
    
    virtual void run_tests();
};
