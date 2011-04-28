//
//  dfa_single_regex.cpp
//  Parse
//
//  Created by Andrew Hunter on 28/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <string>
#include <sstream>

#include "dfa_single_regex.h"

#include "Dfa/lexer.h"

using namespace std;
using namespace dfa;

void test_dfa_single_regex::test(std::string name, std::string regex, std::string accepting, std::string rejecting) {
    // Create a lexer that accepts the regular expression
    lexer myLexer;
    myLexer.add_symbol(regex, 1);
    
    // Compile it
    myLexer.compile();
    
    // Try the accepting phrase
    stringstream acceptIn(accepting);
    lexeme_stream* stream = myLexer.create_stream_from(acceptIn);
    
    // Get the first lexeme
    lexeme* first;
    (*stream) >> first;
    delete stream;
    
    // Try the rejecting phrase
    stringstream rejectIn(rejecting);
    stream = myLexer.create_stream_from(rejectIn);
    
    lexeme* fail;
    (*stream) >> fail;
    
    delete stream;

    // first lexeme should be an accepting lexeme, second should be a rejection
    if (first != NULL && fail != NULL && first->matched() == 1 && first->content<char>() == accepting && fail->matched() == -1 && fail->content().size() == 1) {
        report(name, true);
    } else {
        // Break things down if we get a failure
        report(name + "-accept", first != NULL && first->matched() == 1);
        report(name + "-content", first != NULL && first->content<char>() == accepting);
            
        report(name + "-reject", fail != NULL && fail->matched() == -1);
        report(name + "-rejectone", fail != NULL && fail->content().size() == 1);
    }

    delete first;
    delete fail;
}

void test_dfa_single_regex::run_tests() {
    test("basic", "a", "a", "b");
    test("sayaaaa", "a*", "aaaaa", "b");
    test("ababab", "(ab)+", "ababab", "aaaaaa");
    test("a-or-b1", "a|b", "a", "c");
    test("a-or-b2", "a|b", "b", "c");
    test("a-or-b3", "a+|b+", "aaaaa", "c");
    test("a-or-b4", "a+|b+", "bbbbb", "c");
    test("a-or-b5", "a*|b*", "aaaaa", "c");
    test("a-or-b6", "a*|b*", "bbbbb", "c");
    test("a-or-b7", "(a*)|(b*)", "bbbbb", "c");
    test("a-or-b8", "(a+)|(b+)", "bbbbb", "c");
    test("ab-or-cd1", "(ab)+|(cd)+", "ababab", "c");
    test("ab-or-cd2", "(ab)+|(cd)+", "cdcdcd", "c");
    test("ab-or-cd3", "(ab)+|((cd)+)", "cdcdcd", "c");
}
