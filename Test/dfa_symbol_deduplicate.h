//
//  dfa_symbol_deduplicate.h
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#include "test_fixture.h"

class test_dfa_symbol_deduplicate : public test_fixture {
public:
    test_dfa_symbol_deduplicate() : test_fixture("DFA-symbol-deduplicate") { }
    
    virtual void run_tests();
};
