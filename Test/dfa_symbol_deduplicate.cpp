//
//  dfa_symbol_deduplicate.cpp
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "dfa_symbol_deduplicate.h"
#include "Dfa/remapped_symbol_map.h"
#include "Dfa/epsilon.h"

using namespace dfa;

// True if one of the specified set of symbol set IDs maps to the specified range in the specified map
static bool contains_range(const symbol_map& map, const remapped_symbol_map::new_symbol_set& sets, range<int> chrs) {
    for (remapped_symbol_map::new_symbol_set::const_iterator it = sets.begin(); it != sets.end(); it++) {
        const symbol_set& thisSet = map[*it];
        for (symbol_set::iterator it = thisSet.begin(); it != thisSet.end(); it++) {
            if (chrs == *it) {
                return true;
            }
        }
    }
    return false;
}

void test_dfa_symbol_deduplicate::run_tests() {
    // Simple to start with
    symbol_map has_duplicates1;
    
    int firstSet    = has_duplicates1.identifier_for_symbols(range<int>(0, 20));
    int secondSet   = has_duplicates1.identifier_for_symbols(range<int>(10, 30));
    
    remapped_symbol_map* no_duplicates = remapped_symbol_map::deduplicate(has_duplicates1);
    
    report("NoEpsilon1", ((const symbol_map*)no_duplicates)->identifier_for_symbols(epsilon()) < 0);
    
    remapped_symbol_map::new_symbol_set newSyms;
    
    // First set should map to two sets, one containing 0-10, and one containing 10-20
    newSyms = no_duplicates->new_symbols(firstSet);
    report("NoDuplicates1", !no_duplicates->has_duplicates());
    report("FirstSet1.Size", newSyms.size() == 2);
    report("FirstSet1.0to10", contains_range(*no_duplicates, newSyms, range<int>(0, 10)));
    report("FirstSet1.10to20", contains_range(*no_duplicates, newSyms, range<int>(10, 20)));

    // Second set should map to two sets, 10 to 20 and 20 to 30
    newSyms = no_duplicates->new_symbols(secondSet);
    report("SecondSet1.Size", newSyms.size() == 2);
    report("SecondSet1.10to20", contains_range(*no_duplicates, newSyms, range<int>(10, 20)));
    report("SecondSet1.20to30", contains_range(*no_duplicates, newSyms, range<int>(20, 30)));

    delete no_duplicates;

    // Complex cases
    symbol_map has_duplicates2;
    
    firstSet    = has_duplicates2.identifier_for_symbols(range<int>(10, 20));
    secondSet   = has_duplicates2.identifier_for_symbols(range<int>(30, 40));
    int thirdSet   = has_duplicates2.identifier_for_symbols(symbol_set(range<int>(0, 20)) | range<int>(30, 50));
    
    no_duplicates = remapped_symbol_map::deduplicate(has_duplicates2);
    
    // First set should map to one set, containing 10-20
    newSyms = no_duplicates->new_symbols(firstSet);
    report("NoDuplicates2", !no_duplicates->has_duplicates());
    report("FirstSet2.Size", newSyms.size() == 1);
    report("FirstSet2.10to20", contains_range(*no_duplicates, newSyms, range<int>(10, 20)));
    
    // Second set should map to one set, containing 30-40
    newSyms = no_duplicates->new_symbols(secondSet);
    report("SecondSet2.Size", newSyms.size() == 1);
    report("SecondSet2.30to40", contains_range(*no_duplicates, newSyms, range<int>(30, 40)));

    // Third set should map to 0-10, 10-20, 30-40 and 40-50
    newSyms = no_duplicates->new_symbols(thirdSet);
    report("ThirdSet2.Size", newSyms.size() == 4);
    report("ThirdSet2.0to10", contains_range(*no_duplicates, newSyms, range<int>(0, 10)));
    report("ThirdSet2.10to20", contains_range(*no_duplicates, newSyms, range<int>(10, 20)));
    report("ThirdSet2.30to40", contains_range(*no_duplicates, newSyms, range<int>(30, 40)));
    report("ThirdSet2.40to50", contains_range(*no_duplicates, newSyms, range<int>(40, 50)));
    
    delete no_duplicates;
    
    // This overlapping set is one that causes errors when generating a 'real' parser/lexer
    symbol_map has_duplicates3;
    symbol_set notSlash;
    
    notSlash |= range<int>(0, '/');
    notSlash |= range<int>('/'+1, 0x7fffffff);
    
    int notSlashId      = has_duplicates3.identifier_for_symbols(notSlash);
    int canBeSlashId    = has_duplicates3.identifier_for_symbols(range<int>('/'));
    int backSlashId     = has_duplicates3.identifier_for_symbols(range<int>('\\'));
    
    // Deduplicate it
    no_duplicates = remapped_symbol_map::deduplicate(has_duplicates3);
    
    report("NoDuplicates3", !no_duplicates->has_duplicates());
    
    // Finished with the set
    delete no_duplicates;
}
