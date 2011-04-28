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
    
    // Should be an accepting lexeme
    report(name + "-accept", first != NULL && first->matched() == 1);
    report(name + "-content", first != NULL && first->content<char>() == accepting);
    
    delete stream;
    delete first;
    
    // Try the rejecting phrase
    stringstream rejectIn(rejecting);
    stream = myLexer.create_stream_from(rejectIn);
    
    lexeme* fail;
    (*stream) >> fail;
    
    report(name + "-reject", fail != NULL && fail->matched() == -1);
    report(name + "-rejectone", fail != NULL && fail->content().size() == 1);
    
    delete stream;
    delete fail;
}

void test_dfa_single_regex::run_tests() {
    test("basic", "a", "a", "b");
    test("sayaaaa", "a*", "aaaaa", "b");
    test("ababab", "(ab)+", "ababab", "aaaaaa");
}
