//
//  dfa-multi-regex.h
//  Parse
//
//  Created by Andrew Hunter on 19/06/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
//

#include "test_fixture.h"

/// Tests for creating a DFA that is supposed to distinguish between two regexes
class test_dfa_multi_regex : public test_fixture {
public:
    test_dfa_multi_regex() : test_fixture("DFA-multi-regex") { }
    
    void test(std::string name, std::string regex1, std::string regex2, std::string matches1, std::string matches2);
    
    virtual void run_tests();
};
