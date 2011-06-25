//
//  lr_weaksymbols.h
//  Parse
//
//  Created by Andrew Hunter on 25/06/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "test_fixture.h"

/// Tests for looking into the way that the weak symbols actions table are produced
class test_lr_weaksymbols : public test_fixture {
public:
    test_lr_weaksymbols() : test_fixture("lr-weaksymbols") { }
    
    virtual void run_tests();
};
