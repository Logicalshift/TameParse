//
//  dfa_ndfa.cpp
//  Parse
//
//  Created by Andrew Hunter on 27/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "dfa_ndfa.h"

#include "Dfa/ndfa.h"

using namespace dfa;

void test_dfa_ndfa::run_tests() {
    // NDFA with two transitions on 'a'
    ndfa twoAs;
    twoAs >> 'a' >> accept_action(0);
    twoAs >> 'a' >> accept_action(1);
    
    // Check that it looks OK (should be 3 states)
    report("DFA-create1", twoAs.count_states() == 3);
    
    // Turn into a DFA
    ndfa* twoAsDfa = twoAs.to_dfa();
    
    // Should have two states
    int numStates = twoAsDfa->count_states();
    report("DFA-ndfa-reduced1", numStates == 2);
    
    // Both accept actions should be in the second state
    size_t actionsForSecond = twoAsDfa->actions_for_state(1).size();
    report("DFA-ndfa-accept1", actionsForSecond == 2);
    
    delete twoAsDfa;

    // NDFA with epsilons and two transitions on 'a'
    ndfa twoEpsilonAs;
    twoEpsilonAs >> epsilon() >> 'a' >> accept_action(0);
    numStates = twoEpsilonAs.count_states();
    twoEpsilonAs >> epsilon() >> 'a' >> accept_action(1);
    numStates = twoEpsilonAs.count_states();
    
    // Check that it looks OK (should be 3 states)
    numStates = twoEpsilonAs.count_states();
    report("DFA-create2", numStates == 5);
    
    // Turn into a DFA
    ndfa* twoEpsilonAsDfa = twoEpsilonAs.to_dfa();
    
    // Should have two states
    numStates = twoEpsilonAsDfa->count_states();
    report("DFA-ndfa-reduced2", numStates == 2);
    
    // Both accept actions should be in the second state
    actionsForSecond = twoEpsilonAsDfa->actions_for_state(1).size();
    report("DFA-ndfa-accept2", actionsForSecond == 2);
    
    delete twoEpsilonAsDfa;
}
