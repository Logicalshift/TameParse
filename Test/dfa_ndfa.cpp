//
//  dfa_ndfa.cpp
//  Parse
//
//  Created by Andrew Hunter on 27/04/2011.
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

#include "dfa_ndfa.h"

#include "TameParse/Dfa/ndfa.h"
#include "TameParse/Dfa/ndfa_regex.h"

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
    
    report("isndfa1", !twoAs.verify_is_dfa());
    report("isdfa1", twoAsDfa->is_dfa());
    report("verifydfa1", twoAsDfa->verify_is_dfa());
    
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
    twoEpsilonAs >> epsilon() >> 'a' >> accept_action(1);
    
    // Check that it looks OK (should be 3 states)
    numStates = twoEpsilonAs.count_states();
    report("create2", numStates == 5);
    
    // Turn into a DFA
    ndfa* twoEpsilonAsDfa = twoEpsilonAs.to_dfa();

    report("isndfa2", !twoEpsilonAs.verify_is_dfa());
    report("isdfa2", twoEpsilonAsDfa->is_dfa());
    report("verifydfa2", twoEpsilonAsDfa->verify_is_dfa());

    // Should have two states
    numStates = twoEpsilonAsDfa->count_states();
    report("reduced2", numStates == 2);
    
    // Both accept actions should be in the second state
    actionsForSecond = twoEpsilonAsDfa->actions_for_state(1).size();
    report("accept2", actionsForSecond == 2);
    
    delete twoEpsilonAsDfa;
    
    // NDFA from a regexp
    ndfa_regex oneOrTwoAs;
    oneOrTwoAs.add_regex(0, "a?a", 1);
    
    ndfa* oneOrTwoAsAsDfa = oneOrTwoAs.to_dfa();

    report("isndfa3", !oneOrTwoAs.verify_is_dfa());
    report("isdfa3", oneOrTwoAsAsDfa->is_dfa());
    report("verifydfa3", oneOrTwoAsAsDfa->verify_is_dfa());

    // Should be 3 states
    numStates = oneOrTwoAsAsDfa->count_states();
    report("regex1", numStates == 3);
    
    // States 1 and 2 should both be accepting
    
    // Simple 'or' regex
    ndfa_regex aOrB;
    aOrB.add_regex(0, "a|b", 1);

    ndfa* aOrBAsDfa = aOrB.to_dfa();
    
    report("isdfa4", aOrBAsDfa->is_dfa());
    report("verifydfa4", aOrBAsDfa->verify_is_dfa());

    // Should be 3 states
    numStates = aOrBAsDfa->count_states();
    report("regex2", numStates == 3);
    
    // States 1 and 2 should both be accepting

    // Bracketed 'or' regex
    ndfa_regex aaOrBb;
    aaOrBb.add_regex(0, "(aa)|(bb)", 1);
    
    ndfa* aaOrBbAsDfa = aaOrBb.to_dfa();
    
    report("isdfa5", aaOrBbAsDfa->is_dfa());
    report("verifydfa5", aaOrBbAsDfa->verify_is_dfa());
    
    // Should be 5 states
    numStates = aaOrBbAsDfa->count_states();
    report("regex3", numStates == 5);
}
