//
//  lr_lalr_general.h
//  Parse
//
//  Created by Andrew Hunter on 02/05/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#include "test_fixture.h"

/// Tests for checking various generic things about the LALR parser
class test_lalr_general : public test_fixture {
public:
    test_lalr_general() : test_fixture("lalr-general") { }
    
    virtual void run_tests();
};
