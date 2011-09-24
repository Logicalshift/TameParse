//
//  dfa_symbol_set.cpp
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "dfa_symbol_set.h"
#include "TameParse/Dfa/symbol_set.h"

using namespace dfa;

void test_dfa_symbol_set::run_tests() {
    typedef range<int> r;
    symbol_set testSet(r(10, 20));
    
    report("Contains1", testSet[10]);
    report("Contains2", testSet[19]);
    report("Contains3", !testSet[9]);
    report("Contains4", !testSet[20]);
    
    symbol_set disjointSet = testSet | r(40, 50);

    report("Disjoint1", disjointSet[10]);
    report("Disjoint2", disjointSet[19]);
    report("Disjoint3", disjointSet[40]);
    report("Disjoint4", disjointSet[49]);
    report("Disjoint5", !disjointSet[9]);
    report("Disjoint6", !disjointSet[20]);
    report("Disjoint7", !disjointSet[39]);
    report("Disjoint8", !disjointSet[51]);

    symbol_set rejoinedSet = disjointSet | r(15, 45);

    report("Rejoined1", rejoinedSet[10]);
    report("Rejoined2", rejoinedSet[19]);
    report("Rejoined3", rejoinedSet[40]);
    report("Rejoined4", rejoinedSet[49]);
    report("Rejoined5", !rejoinedSet[9]);
    report("Rejoined6", rejoinedSet[20]);
    report("Rejoined7", rejoinedSet[39]);
    report("Rejoined8", !rejoinedSet[51]);
    
    symbol_set brokenUp = symbol_set(r(10, 50)) & ~symbol_set(r(20, 40));
    
    report("BrokenUp1", brokenUp[10]);
    report("BrokenUp2", brokenUp[19]);
    report("BrokenUp3", brokenUp[40]);
    report("BrokenUp4", brokenUp[49]);
    report("BrokenUp5", !brokenUp[9]);
    report("BrokenUp6", !brokenUp[20]);
    report("BrokenUp7", !brokenUp[39]);
    report("BrokenUp8", !brokenUp[51]);
    
    symbol_set rejoinedBrokenUp = rejoinedSet.excluding(r(20, 40));
    
    report("RejoinedBrokenUp1", rejoinedBrokenUp[10]);
    report("RejoinedBrokenUp2", rejoinedBrokenUp[19]);
    report("RejoinedBrokenUp3", rejoinedBrokenUp[40]);
    report("RejoinedBrokenUp4", rejoinedBrokenUp[49]);
    report("RejoinedBrokenUp5", !rejoinedBrokenUp[9]);
    report("RejoinedBrokenUp6", !rejoinedBrokenUp[20]);
    report("RejoinedBrokenUp7", !rejoinedBrokenUp[39]);
    report("RejoinedBrokenUp8", !rejoinedBrokenUp[51]);
    
    symbol_set empty = brokenUp.excluding(r(5, 60));
    
    report("Empty1", !empty[10]);
    report("Empty2", !empty[19]);
    report("Empty3", !empty[40]);
    report("Empty4", !empty[49]);
    report("Empty5", !empty[9]);
    report("Empty6", !empty[20]);
    report("Empty7", !empty[39]);
    report("Empty8", !empty[51]);
    
    report("Equality1", brokenUp == disjointSet);
    report("Equality2", brokenUp <= disjointSet);
    report("Equality3", empty != disjointSet);
    report("Equality4", disjointSet != testSet);
    
    symbol_set threeGroups = symbol_set(r(10, 20)) | r(30, 40) | r(50, 60);
    
    report("Join1", (symbol_set(r(20, 40)) | r(10, 50)) == symbol_set(r(10, 50)));
    report("Join2", (symbol_set(r(10, 50)) | r(20, 40)) == symbol_set(r(10, 50)));
    report("Join3", (threeGroups | r(20, 50)) == symbol_set(r(10, 60)));

    report("Split1", (symbol_set(r(10, 60)).excluding(r(20, 30)).excluding(r(40, 50))) == threeGroups);
    report("Split2", (symbol_set(r(10, 50)) & symbol_set(r(20, 40))) == symbol_set(r(20, 40)));
    report("Split3", (threeGroups & symbol_set(r(20, 50))) == symbol_set(r(30, 40)));

    report("Invert1", threeGroups == ~~threeGroups);
    report("Invert2", threeGroups != ~threeGroups);
    report("Invert3", ~(threeGroups | ~threeGroups) == empty);
    report("Invert4", (threeGroups & ~threeGroups) == empty);
    report("Invert5", (threeGroups | ~threeGroups) == ~empty);
}
