//
//  dfa-multi-regex.cpp
//  Parse
//
//  Created by Andrew Hunter on 19/06/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#include <string>
#include <sstream>

#include "dfa_multi_regex.h"

#include "TameParse/Dfa/lexer.h"

using namespace std;
using namespace dfa;

void test_dfa_multi_regex::test(std::string name, std::string regex1, std::string regex2, std::string matches1, std::string matches2) {
    // Create a lexer that accepts the regular expression
    lexer myLexer;
    myLexer.add_symbol(regex1, 1);
    myLexer.add_symbol(regex2, 2);
    
    // Compile it
    bool isValid = myLexer.verify();
    myLexer.compile(false);
    
    // Try the first phrase
    stringstream match1In(matches1);
    lexeme_stream* stream = myLexer.create_stream_from(match1In);
    
    lexeme* shouldMatch1;
    (*stream) >> shouldMatch1;
    delete stream;
    
    // Try the second phrase
    stringstream match2In(matches2);
    stream = myLexer.create_stream_from(match2In);
    
    lexeme* shouldMatch2;
    (*stream) >> shouldMatch2;
    
    report(name + "-valid", isValid);
    
    report(name + "-matches1", shouldMatch1->matched() == 1);
    report(name + "-matches1-not2", shouldMatch1->matched() != 2);
    report(name + "-matches2", shouldMatch2->matched() == 2);
    report(name + "-matches2-not1", shouldMatch2->matched() != 1);
    
    delete shouldMatch1;
    delete shouldMatch2;
}

void test_dfa_multi_regex::run_tests() {
    test("ab", "a", "b", "a", "b");
    test("aa1", "a", "aa", "a", "aa");
    test("aa2", "a", "aa+", "a", "aa");
    test("aa3", "a", "aa*", "a", "aa");

    test("a-or-b1", "(a|b)", "aa", "a", "aa");
    test("a-or-b2", "(a|b)", "aa", "b", "aa");
    test("a-or-b3", "a", "(a|b)a", "a", "aa");
    test("a-or-b4", "a", "(a|b)a", "a", "ba");
    test("a-or-b5", "(a|b)", "(a|b)a", "a", "ba");
    test("a-or-b6", "(a|b)", "(a|b)a", "b", "ba");

    test("not-a-or-a", "[^a]", "a", "b", "a");
    test("anything-or-a", "a", ".", "a", "b");

    test("language-or-identifier1", "language", "[A-Za-z\\-][A-Za-z\\-0-9]*", "language", "some-identifier");
    test("language-or-identifier2", "language", "[A-Za-z\\-][A-Za-z\\-0-9]*", "language", "lang-identifier");
    test("language-or-identifier3", "language", "[A-Za-z\\-][A-Za-z\\-0-9]*", "language", "language2");
    
    test("character-or-identifier", "'(.|(\\\\.))'", "[A-Za-z\\-][A-Za-z\\-0-9]*", "'a'", "some-identifier");
    test("regex-or-character1", "/([^/]|(\\\\/))*/", "'(.|(\\\\.))'", "/re/", "'a'");
    test("regex-or-character2", "/([a-z]|(\\\\/))*/", "'([a-z]|(\\\\[a-z]))'", "/re/", "'a'");
    test("regex-or-identifier1", "/([^/]|(\\\\/))*/", "[A-Za-z\\-][A-Za-z\\-0-9]*", "/re/", "some-identifier");
    test("regex-or-identifier2", "/([a-z]|(\\\\/))*/", "[A-Za-z\\-][A-Za-z\\-0-9]*", "/re/", "some-identifier");
}
