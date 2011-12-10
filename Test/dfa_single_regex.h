//
//  dfa_single_regex.h
//  Parse
//
//  Created by Andrew Hunter on 28/04/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
//

#include "test_fixture.h"

/// Tests for creating a DFA that accepts a single regex
class test_dfa_single_regex : public test_fixture {
public:
    test_dfa_single_regex() : test_fixture("DFA-single-regex") { }
    
    void test(std::string name, std::string regex, std::string accepting, std::string rejecting);
    
    virtual void run_tests();
};
