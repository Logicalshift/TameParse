//
//  contextfree_followset.cpp
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
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

#include "contextfree_followset.h"

#include "TameParse/ContextFree/grammar.h"

using namespace contextfree;

static inline bool contains(const item_set& set, const item& target) {
    return set.contains(target);
}

void test_contextfree_followset::run_tests() {
    grammar testGram;
    
    // Rule with a single terminal
    nonterminal simpleFollow(testGram.id_for_nonterminal(L"simplefollow"));
    (testGram += L"simplefollow") << 1;
    
    // Create a follow set for simplefollow consisting of a single item (so FOLLOW(simplefollow) == 2)
    (testGram += L"nonterm") << L"simplefollow" << 2;
    
    // Emptyfollow1 is used in a context where it's followed by an empty rule
    nonterminal emptyFollow1(testGram.id_for_nonterminal(L"emptyfollow1"));
    (testGram += L"empty");
    (testGram += L"emptyfollow1") << 1;

    (testGram += L"nonterm2") << L"emptyfollow1" << L"empty" << 3;
    
    // Emptyfollow2 is used in a context where it's followed by an empty rule, which is itself followed by an empty rule
    nonterminal emptyFollow2(testGram.id_for_nonterminal(L"emptyfollow2"));
    (testGram += L"emptyfollow2") << 1;
    
    (testGram += L"nonterm3") << L"emptyfollow2" << L"empty";
    (testGram += L"nonterm4") << L"nonterm3" << 4;
    
    // Verify each of the follow sets
    terminal term1(1);
    terminal term2(2);
    terminal term3(3);
    terminal term4(4);
    
    empty_item an_empty_item;
    nonterminal ntEmpty(testGram.id_for_nonterminal(L"empty"));
    
    item_set followSet(&testGram);
    
    followSet = testGram.follow(simpleFollow);
    report("simplefollow.size", followSet.size() == 1);
    report("simplefollow.contains-term2", contains(followSet, term2));

    followSet = testGram.follow(emptyFollow1);
    report("emptyFollow1.size", followSet.size() == 3);
    report("emptyFollow1.contains-term3", contains(followSet, term3));
    report("emptyFollow1.contains-empty", contains(followSet, an_empty_item));
    report("emptyFollow1.contains-ntEmpty", contains(followSet, ntEmpty));
    
    followSet = testGram.follow(emptyFollow2);
    report("emptyFollow2.size", followSet.size() == 3);
    report("emptyFollow2.contains-term4", contains(followSet, term4));
    report("emptyFollow2.contains-empty", contains(followSet, an_empty_item));
    report("emptyFollow2.contains-ntEmpty", contains(followSet, ntEmpty));
}
