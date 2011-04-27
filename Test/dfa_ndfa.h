//
//  dfa_ndfa.h
//  Parse
//
//  Created by Andrew Hunter on 27/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "test_fixture.h"

class test_dfa_ndfa : public test_fixture {
public:
    test_dfa_ndfa() : test_fixture("DFA-ndfa") { }
    
    virtual void run_tests();
};
