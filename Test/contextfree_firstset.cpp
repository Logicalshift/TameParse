//
//  contextfree_firstset.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#include "contextfree_firstset.h"
#include "TameParse/ContextFree/grammar.h"

using namespace contextfree;

static inline bool contains(const item_set& set, const item& target) {
    return set.contains(item_container((item*)&target, false));
}

void test_contextfree_firstset::run_tests() {
    grammar testGram;
    
    // Rule with a single terminal
    nonterminal ntTerm(testGram.id_for_nonterminal(L"term"));
    (testGram += L"term") << 1;
    
    // Nonterminal rule that references our original terminal
    nonterminal ntNonterm(testGram.id_for_nonterminal(L"nonterm"));
    (testGram += L"nonterm") << L"term";

    // Rule with multiple terminals
    nonterminal ntTerm2(testGram.id_for_nonterminal(L"term2"));
    (testGram += L"term2") << 1;
    (testGram += L"term2") << 2;

    // Recursive rule
    nonterminal ntRecurse(testGram.id_for_nonterminal(L"recurse"));
    (testGram += L"recurse") << L"term2";
    (testGram += L"recurse") << L"recurse" << L"term2";
    
    // Rule with an empty clause
    nonterminal ntEmpty(testGram.id_for_nonterminal(L"empty"));
    (testGram += L"empty");
    (testGram += L"empty") << L"term2";
    
    // Non-empty rule that references an empty one
    nonterminal ntNonempty(testGram.id_for_nonterminal(L"nonempty"));
    (testGram += L"nonempty") << L"empty" << 3;
    
    // Check out the first sets of these rules
    terminal term1(1);
    terminal term2(2);
    terminal term3(3);
    empty_item empty;

    report("term1.contains-self", contains(testGram.first(term1), term1));

    report("ntTerm.contains-self", contains(testGram.first(ntTerm), ntTerm));
    report("ntTerm.contains-term1", contains(testGram.first(ntTerm), term1));
    report("ntTerm.size", testGram.first(ntTerm).size() == 2);
    
    report("ntNonterm.contains-self", contains(testGram.first(ntNonterm), ntNonterm));
    report("ntNonterm.contains-term", contains(testGram.first(ntNonterm), ntTerm));
    report("ntNonterm.contains-term1", contains(testGram.first(ntNonterm), term1));
    report("ntNonterm.size", testGram.first(ntNonterm).size() == 3);

    report("ntTerm2.contains-self", contains(testGram.first(ntTerm2), ntTerm2));
    report("ntTerm2.contains-term1", contains(testGram.first(ntTerm2), term1));
    report("ntTerm2.contains-term2", contains(testGram.first(ntTerm2), term2));
    report("ntTerm2.size", testGram.first(ntTerm2).size() == 3);

    report("ntRecurse.contains-self", contains(testGram.first(ntRecurse), ntRecurse));
    report("ntRecurse.contains-term1", contains(testGram.first(ntRecurse), term1));
    report("ntRecurse.contains-term2", contains(testGram.first(ntRecurse), term2));
    report("ntRecurse.size", testGram.first(ntRecurse).size() == 4);

    report("ntEmpty.contains-self", contains(testGram.first(ntEmpty), ntEmpty));
    report("ntEmpty.contains-term1", contains(testGram.first(ntEmpty), term1));
    report("ntEmpty.contains-term2", contains(testGram.first(ntEmpty), term2));
    report("ntEmpty.contains-empty", contains(testGram.first(ntEmpty), empty));
    report("ntEmpty.size", testGram.first(ntEmpty).size() == 5);

    report("ntNonempty.contains-self", contains(testGram.first(ntNonempty), ntNonempty));
    report("ntNonempty.contains-term1", contains(testGram.first(ntNonempty), term1));
    report("ntNonempty.contains-term2", contains(testGram.first(ntNonempty), term2));
    report("ntNonempty.contains-term3", contains(testGram.first(ntNonempty), term3));
    report("ntNonempty.contains-ntEmpty", contains(testGram.first(ntNonempty), ntEmpty));
    report("ntNonempty.doesnot-contain-empty", !contains(testGram.first(ntNonempty), empty));
    report("ntNonempty.size", testGram.first(ntEmpty).size() == 5);
}
