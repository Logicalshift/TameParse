//
//  dfa_range.cpp
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
//

#include "dfa_range.h"
#include "TameParse/Dfa/range.h"

using namespace dfa;

void test_dfa_range::run_tests() {
    typedef range<int> r;
    
    // Test some operations on a range
    r testRange = r(20, 50);
    
    // Comparisons
    report("LessThan1",             testRange < r(60, 70));
    report("LessThan2",             testRange < r(20, 60));
    report("LessThan3",             r(10, 20) < testRange);
    report("LessThan4",             r(20, 40) < testRange);
    report("LessThanOrEqualTo1",    testRange <= r(60, 70));
    report("LessThanOrEqualTo2",    testRange <= r(20, 60));
    report("LessThanOrEqualTo3",    testRange <= r(20, 50));
    report("LessThanOrEqualTo4",    r(10, 20) <= testRange);
    report("LessThanOrEqualTo5",    r(20, 40) <= testRange);
    report("GreaterThan1",          testRange > r(10, 20));
    report("GreaterThan2",          testRange > r(20, 40));
    report("GreaterThanOrEqualTo1", testRange >= r(10, 20));
    report("GreaterThanOrEqualTo2", testRange >= r(20, 40));
    report("GreaterThanOrEqualTo3", testRange >= r(20, 50));
    report("Equality",              testRange == r(20, 50));
    report("Inequality",            testRange != r(20, 40));
    
    // Merging
    report("CanMerge1", testRange.can_merge(r(10, 20)));
    report("CanMerge2", testRange.can_merge(r(50, 60)));
    report("CanMerge3", testRange.can_merge(r(20, 50)));
    report("CanMerge4", testRange.can_merge(r(10, 50)));
    report("CanMerge5", testRange.can_merge(r(20, 60)));
    report("CanMerge6", testRange.can_merge(r(10, 60)));

    report("CantMerge1", !testRange.can_merge(r(0, 10)));
    report("CantMerge2", !testRange.can_merge(r(60, 70)));
    report("CantMerge3", !testRange.can_merge(r(10, 19)));
    report("CantMerge4", !testRange.can_merge(r(51, 60)));
    
    // Contains
    report("Contains1", testRange[20]);
    report("Contains2", testRange[49]);
    report("Contains3", !testRange[19]);
    report("Contains4", !testRange[50]);
}
