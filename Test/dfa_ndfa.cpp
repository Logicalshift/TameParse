//
//  dfa_ndfa.cpp
//  Parse
//
//  Created by Andrew Hunter on 27/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "dfa_ndfa.h"

#include "Dfa/ndfa.h"
#include "Dfa/ndfa_regex.h"

using namespace dfa;

void test_dfa_ndfa::run_tests() {
    // NDFA with two transitions on 'a'
    ndfa twoAs;
    twoAs >> 'a' >> accept_action(0);
    twoAs >> 'a' >> accept_action(1);
    
    // Check that it looks OK (should be 3 states)
    report("create1", twoAs.count_states() == 3);
    
    // Turn into a DFA
    ndfa* twoAsDfa = twoAs.to_dfa();
    
    // Should have two states
    int numStates = twoAsDfa->count_states();
    report("reduced1", numStates == 2);
    
    // Both accept actions should be in the second state
    size_t actionsForSecond = twoAsDfa->actions_for_state(1).size();
    report("accept1", actionsForSecond == 2);
    
    delete twoAsDfa;

    // NDFA with epsilons and two transitions on 'a'
    ndfa twoEpsilonAs;
    twoEpsilonAs >> epsilon() >> 'a' >> accept_action(0);
    numStates = twoEpsilonAs.count_states();
    twoEpsilonAs >> epsilon() >> 'a' >> accept_action(1);
    numStates = twoEpsilonAs.count_states();
    
    // Check that it looks OK (should be 3 states)
    numStates = twoEpsilonAs.count_states();
    report("create2", numStates == 5);
    
    // Turn into a DFA
    ndfa* twoEpsilonAsDfa = twoEpsilonAs.to_dfa();
    
    // Should have two states
    numStates = twoEpsilonAsDfa->count_states();
    report("reduced2", numStates == 2);
    
    // Both accept actions should be in the second state
    actionsForSecond = twoEpsilonAsDfa->actions_for_state(1).size();
    report("accept2", actionsForSecond == 2);
    
    delete twoEpsilonAsDfa;
    
    // NDFA from a regexp
    ndfa_regex oneOrTwoAs;
    int finalState = oneOrTwoAs.add_regex(0, "a?a", 1);
    
    ndfa* oneOrTwoAsAsDfa = oneOrTwoAs.to_dfa();
    
    // Should be 3 states
    numStates = oneOrTwoAsAsDfa->count_states();
    report("regex1", numStates == 3);
    
    // States 1 and 2 should both be accepting
    
    // Simple 'or' regex
    ndfa_regex aOrB;
    finalState = aOrB.add_regex(0, "a|b", 1);

    ndfa* aOrBAsDfa = aOrB.to_dfa();
    
    // Should be 3 states
    numStates = aOrBAsDfa->count_states();
    report("regex2", numStates == 3);
    
    // States 1 and 2 should both be accepting

    // Bracketed 'or' regex
    ndfa_regex aaOrBb;
    finalState = aaOrBb.add_regex(0, "(aa)|(bb)", 1);
    
    ndfa* aaOrBbAsDfa = aaOrBb.to_dfa();
    
    // Should be 5 states
    numStates = aaOrBbAsDfa->count_states();
    report("regex3", numStates == 5);
}
