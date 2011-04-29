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
    
    lexer myCompactLexer;
    myCompactLexer.add_symbol(regex, 1);
    
    // Compile it
    myLexer.compile(false);
    myCompactLexer.compile(true);
    
    // Try the accepting phrase
    stringstream acceptIn(accepting);
    lexeme_stream* stream = myLexer.create_stream_from(acceptIn);
    
    // Get the first lexeme
    lexeme* first;
    (*stream) >> first;
    delete stream;
    
    // Try again with the compact lexer
    stringstream acceptCompactIn(accepting);
    stream = myCompactLexer.create_stream_from(acceptCompactIn);
    
    lexeme* firstCompact;
    (*stream) >> firstCompact;
    delete stream;
    
    // Try the rejecting phrase
    stringstream rejectIn(rejecting);
    stream = myLexer.create_stream_from(rejectIn);
    
    lexeme* fail;
    (*stream) >> fail;
    
    delete stream;

    // first lexeme should be an accepting lexeme, second should be a rejection
    if (first != NULL && fail != NULL && first->matched() == 1 && first->content<char>() == accepting
        && firstCompact != NULL && firstCompact->matched() == 1 && firstCompact->content<char>() == accepting
        && fail != NULL && fail->matched() == -1 && fail->content().size() == 1) {
        report(name, true);
    } else {
        // Break things down if we get a failure
        report(name + "-accept", first != NULL && first->matched() == 1);
        report(name + "-content", first != NULL && first->content<char>() == accepting);

        report(name + "-compact-accept", firstCompact != NULL && firstCompact->matched() == 1);
        report(name + "-compact-content", firstCompact != NULL && firstCompact->content<char>() == accepting);

        report(name + "-reject", fail != NULL && fail->matched() == -1);
        report(name + "-rejectone", fail != NULL && fail->content().size() == 1);
    }

    delete first;
    delete firstCompact;
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
    
    test("ab-or-cd1", "ab|cd", "ab", "a");
    test("ab-or-cd2", "ab|cd", "cd", "a");
    test("ab-or-cd3", "(ab)+|(cd)+", "ababab", "a");
    test("ab-or-cd4", "(ab)+|(cd)+", "cdcdcd", "c");
    test("ab-or-cd5", "(ab)+|((cd)+)", "cdcdcd", "c");

    test("ab-or-cd-or-ef1", "ab|cd|ef", "ab", "af");
    test("ab-or-cd-or-ef2", "ab|cd|ef", "cd", "c");
    test("ab-or-cd-or-ef3", "ab|cd|ef", "ef", "d");

    test("a-or-b-c1", "(a|b?)c", "ac", "a");
    test("a-or-b-c2", "(a|b?)c", "bc", "b");
    test("a-or-b-c3", "(a|b?)c", "c", "b");

    test("escape1", "\\a", "\a", "b");
    test("escape2", "\\e", "\e", "b");
    test("escape3", "\\n", "\n", "b");
    test("escape4", "\\r", "\r", "b");
    test("escape5", "\\f", "\f", "b");
    test("escape6", "\\t", "\t", "b");
    test("escape7", "\\101", "A", "b");
    test("escape8", "\\x41", "A", "b");
    test("escape9", "\\u0041", "A", "b");
    test("escape10", "\\o000101", "A", "b");

    test("range1", "[a-z]", "a", "0");
    test("range2", "[a-z]", "z", "0");
    test("range3", "[^a-z]", "0", "a");
    test("range4", "[abc]+", "abc", "d");
    test("range5", "[a-zA-Z]+", "azAZ", "0");
    test("range6", "[a-c]+", "abc", "d");
    test("range7", "[azAZ]+", "azAZ", "0");
    test("range8", "[azAZ]+", "a", "0");
    test("range9", "[azAZ]+", "z", "0");
    test("range10", "[azAZ]+", "A", "0");
    test("range11", "[azAZ]+", "Z", "0");
}
