//
//  dfa_symbol_set.cpp
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
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

#include "dfa_symbol_set.h"
#include "TameParse/Dfa/symbol_set.h"
#include "TameParse/Util/unicode.h"

using namespace dfa;
using namespace util;

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

    symbol_set a_to_z(r('a', 'z'));
    symbol_set A_to_Z(r('A', 'Z'));
    symbol_set m_to_o(r('m', 'o'));
    symbol_set M_to_O(r('M', 'O'));
    symbol_set a_to_z_to_0 = a_to_z | r('0', '9');
    symbol_set A_to_Z_to_0 = A_to_Z | r('0', '9');
    unicode uc;

    report("ToUpper1", uc.to_upper(a_to_z) == A_to_Z);
    report("ToUpper2", uc.to_upper(A_to_Z) == A_to_Z);
    report("ToUpper3", uc.to_upper(a_to_z_to_0) == A_to_Z_to_0);
    report("ToUpper4", uc.to_upper(m_to_o) == M_to_O);

    report("ToLower1", uc.to_lower(A_to_Z) == a_to_z);
    report("ToLower2", uc.to_lower(a_to_z) == a_to_z);
    report("ToLower3", uc.to_lower(A_to_Z_to_0) == a_to_z_to_0);
    report("ToLower4", uc.to_lower(M_to_O) == m_to_o);
}
