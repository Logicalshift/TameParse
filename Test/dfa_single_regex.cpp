//
//  dfa_single_regex.cpp
//  Parse
//
//  Created by Andrew Hunter on 28/04/2011.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the \"Software\"), to 
//  deal in the Software without restriction, including without limitation the 
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
//  sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
//  IN THE SOFTWARE.
//

#include <string>
#include <sstream>

#include "dfa_single_regex.h"

#include "TameParse/Dfa/lexer.h"

using namespace std;
using namespace dfa;

void test_dfa_single_regex::test(std::string name, std::string regex, std::string accepting, std::string rejecting) {
    // Create a lexer that accepts the regular expression
    lexer myLexer;
    myLexer.add_symbol(regex, 1);
    
    lexer myCompactLexer;
    myCompactLexer.add_symbol(regex, 1);
    
    // Compile it
    bool isValid = myLexer.verify();
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
    bool success = true;
    
    if (first == NULL)                                      success = false;
    else if (first->matched() != 1)                         success = false;
    else if (first->content<char>() != accepting)           success = false;
    
    if (firstCompact == NULL)                               success = false;
    else if (firstCompact->matched() != 1)                  success = false;
    else if (firstCompact->content<char>() != accepting)    success = false;
    
    if (rejecting.size() == 0 && fail != NULL)              success = false;
    else if (fail == NULL)                                  success = false;
    else if (fail->matched() != -1)                         success = false;
    else if (fail->content().size() == 1)                   success = false;
    
    // Report if the lexer produced is not valid
    if (!isValid) {
        report(name + "-valid", isValid);
    }
    
    if (!success) {
        report(name, true);
    } else {
        // Break things down if we get a failure
        report(name + "-accept", first != NULL && first->matched() == 1);
        report(name + "-content", first != NULL && first->content<char>() == accepting);

        report(name + "-compact-accept", firstCompact != NULL && firstCompact->matched() == 1);
        report(name + "-compact-content", firstCompact != NULL && firstCompact->content<char>() == accepting);

        report(name + "-reject", fail == NULL || fail->matched() == -1);
        report(name + "-rejectone", (fail != NULL && fail->content().size() == 1) || (fail == NULL && rejecting.size() == 0));
    }

    delete first;
    delete firstCompact;
    delete fail;
}

void test_dfa_single_regex::run_tests() {
    test("basic", "a", "a", "b");
    test("sayaaaa", "a*", "aaaaa", "b");
    test("ababab", "(ab)+", "ababab", "aaaaaa");
    
    test("nothing1", "a", "a", "");
    test("nothing2", "[a]", "a", "");
    test("nothing3", "[^a]", "b", "");
    test("nothing4", "[a]+", "aaa", "");
    test("nothing5", "[^a]+", "aaa", ""); // Infinite loop!
    test("nothing6", "(a|b)", "a", "");
    test("nothing7", "(a|\\ufffd)", "a", ""); // 0xfffd = -3 = symbol_set::end_of_input
    test("nothing8", "(a|\\uffff)", "a", "");
    
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
    test("escape2", "\\e", "\x1b", "b");
    test("escape3", "\\n", "\n", "b");
    test("escape4", "\\r", "\r", "b");
    test("escape5", "\\f", "\f", "b");
    test("escape6", "\\t", "\t", "b");
    test("escape7", "\\101", "A", "b");
    test("escape8", "\\x41", "A", "b");
    test("escape9", "\\u0041", "A", "b");
    test("escape10", "\\o000101", "A", "b");
    test("escape11", "\\u0041A", "AA", "Ab");

    test("range1", "[a-z]", "a", "0");
    test("range2", "[a-z]", "z", "0");
    test("range3", "[^a-z]", "0", "a");
    test("range4", "[abc]+", "abc", "d");
    test("range5", "[a-zA-Z]+", "azAZ", "0");
    test("range5-withor", "([a-z]|[A-Z])+", "azAZ", "0");
    test("range6", "[a-c]+", "abc", "d");
    test("range7", "[azAZ]+", "azAZ", "0");
    test("range8", "[azAZ]+", "a", "0");
    test("range9", "[azAZ]+", "z", "0");
    test("range10", "[azAZ]+", "A", "0");
    test("range11", "[azAZ]+", "Z", "0");
    test("range12", "([a-f][c-h])+", "ccah", "ha");

    test("anything1", ".", "a", "");
    test("anything2", "[^a]", "b", "a");
    test("anything3", "[^ab]", "c", "b");
    test("anything4", "[^ab]", "d", "a");
    test("anything5", "([^ab]|b)", "b", "a");
    test("anything6", "([^ab]|b)+", "bcde", "a"); // Infinite loop!
    test("anything7", "[ -\\u00ff]", "a", "");
    test("anything8", "[ -\\ufffc]", "a", "");
    test("anything9", "[ -\\ufffd]", "a", "");
    test("anything10", "[ -\\uffff]", "a", "");

    test("bootstrap-identifier1", "[A-Za-z\\-][A-Za-z\\-0-9]*", "some-identifier", "0");
    test("bootstrap-identifier2", "[A-Za-z\\-][A-Za-z\\-0-9]*", "language", "0");
    test("bootstrap-identifier3", "([A-Za-z\\-][A-Za-z\\-0-9]*)|(language)", "language", "0");
    test("bootstrap-identifier4", "([A-Za-z\\-][A-Za-z\\-0-9]*)|(language)", "some-identifier", "0");
}
