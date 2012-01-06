//
//  dfa_symbol_deduplicate.cpp
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#include "dfa_symbol_deduplicate.h"
#include "TameParse/Dfa/remapped_symbol_map.h"
#include "TameParse/Dfa/epsilon.h"

using namespace dfa;

// True if one of the specified set of symbol set IDs maps to the specified range in the specified map
static bool contains_range(const symbol_map& map, const remapped_symbol_map::new_symbol_set& sets, range<int> chrs) {
    for (remapped_symbol_map::new_symbol_set::const_iterator symSet = sets.begin(); symSet != sets.end(); ++symSet) {
        const symbol_set& thisSet = map[*symSet];
        for (symbol_set::iterator symRange = thisSet.begin(); symRange != thisSet.end(); ++symRange) {
            if (chrs == *symRange) {
                return true;
            }
        }
    }
    return false;
}

// True if each range in the original symbol map is completely mapped to the new symbol map
static bool check_ranges(const symbol_map& original, const remapped_symbol_map& remapped) {
    typedef remapped_symbol_map::new_symbol_set new_symbols;
    
    // Iterate through the old symbol sets
    for (symbol_map::iterator oldSymbols = original.begin(); oldSymbols != original.end(); ++oldSymbols) {
        // Set of symbols that haven't been remapped
        symbol_set notMapped = *oldSymbols->first;
        
        // Get the IDs that this has been remapped to
        new_symbols remappedSets = remapped.new_symbols(oldSymbols->second);
        
        // Remove the sets from the 'not mapped' set
        for (new_symbols::const_iterator newSet = remappedSets.begin(); newSet != remappedSets.end(); ++newSet) {
            // Should not yet be removed from the unmapped set
            symbol_set intersect = notMapped & remapped[*newSet];
            
            if (intersect != remapped[*newSet]) {
                return false;
            }
            
            // Remove from the set of unmapped symbols
            notMapped.exclude(remapped[*newSet]);
        }
        
        // Should be no symbols left in notMapped
        if (!notMapped.empty()) {
            return false;
        }
    }
    
    // Looks good
    return true;
}

void test_dfa_symbol_deduplicate::run_tests() {
    // Simple to start with
    symbol_map has_duplicates1;
    
    int firstSet    = has_duplicates1.identifier_for_symbols(range<int>(0, 20));
    int secondSet   = has_duplicates1.identifier_for_symbols(range<int>(10, 30));
    
    remapped_symbol_map* no_duplicates = remapped_symbol_map::deduplicate(has_duplicates1);
    
    report("NoEpsilon1", ((const symbol_map*)no_duplicates)->find_identifier_for_symbols(epsilon()) < 0);
    
    remapped_symbol_map::new_symbol_set newSyms;
    
    // First set should map to two sets, one containing 0-10, and one containing 10-20
    newSyms = no_duplicates->new_symbols(firstSet);
    report("NoDuplicates1", !no_duplicates->has_duplicates());
    report("AllRemapped1", check_ranges(has_duplicates1, *no_duplicates));
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
    report("AllRemapped2", check_ranges(has_duplicates2, *no_duplicates));
    report("FirstSet2.Size", newSyms.size() == 1);
    report("FirstSet2.10to20", contains_range(*no_duplicates, newSyms, range<int>(10, 20)));
    report("FirstSet2.0to10", !contains_range(*no_duplicates, newSyms, range<int>(0, 10)));
    report("FirstSet2.20to30", !contains_range(*no_duplicates, newSyms, range<int>(20, 30)));
    
    // Second set should map to one set, containing 30-40
    newSyms = no_duplicates->new_symbols(secondSet);
    report("SecondSet2.Size", newSyms.size() == 1);
    report("SecondSet2.30to40", contains_range(*no_duplicates, newSyms, range<int>(30, 40)));
    report("SecondSet2.20to30", !contains_range(*no_duplicates, newSyms, range<int>(20, 30)));
    report("SecondSet2.40to50", !contains_range(*no_duplicates, newSyms, range<int>(40, 50)));

    // Third set should map to 0-10, 10-20, 30-40 and 40-50
    newSyms = no_duplicates->new_symbols(thirdSet);
    report("ThirdSet2.Size", newSyms.size() == 3);
    report("ThirdSet2.0to10", contains_range(*no_duplicates, newSyms, range<int>(0, 10)));
    report("ThirdSet2.10to20", contains_range(*no_duplicates, newSyms, range<int>(10, 20)));
    report("ThirdSet2.30to40", contains_range(*no_duplicates, newSyms, range<int>(30, 40)));
    report("ThirdSet2.40to50", contains_range(*no_duplicates, newSyms, range<int>(40, 50)));
    report("ThirdSet2.50to60", !contains_range(*no_duplicates, newSyms, range<int>(50, 60)));
    
    delete no_duplicates;
    
    // This overlapping set is one that causes errors when generating a 'real' parser/lexer
    symbol_map has_duplicates3;
    symbol_set notSlash;
    
    notSlash |= range<int>(0, '/');
    notSlash |= range<int>('/'+1, 0x7fffffff);
    
    has_duplicates3.identifier_for_symbols(notSlash);
    has_duplicates3.identifier_for_symbols(range<int>('/'));
    has_duplicates3.identifier_for_symbols(range<int>('\\'));
    
    // Deduplicate it
    no_duplicates = remapped_symbol_map::deduplicate(has_duplicates3);
    
    report("NoDuplicates3", !no_duplicates->has_duplicates());
    report("AllRemapped3", check_ranges(has_duplicates3, *no_duplicates));

    // Same as above, except not using a symbol set with more than one range in it
    symbol_map has_duplicates4;
    
    has_duplicates4.identifier_for_symbols(range<int>(0, '/'));
    has_duplicates4.identifier_for_symbols(range<int>('/'+1, 0x7fffffff));
    has_duplicates4.identifier_for_symbols(range<int>('/'));
    has_duplicates4.identifier_for_symbols(range<int>('\\'));
    
    // Deduplicate it
    no_duplicates = remapped_symbol_map::deduplicate(has_duplicates4);
    
    report("NoDuplicates4", !no_duplicates->has_duplicates());
    report("AllRemapped4", check_ranges(has_duplicates4, *no_duplicates));
    
    // Same as above, except adding some more character ranges
    symbol_map has_duplicates5;
    symbol_set alphabetical;
    symbol_set alphaNumeric;
    
    alphabetical |= range<int>('a', 'z');
    alphabetical |= range<int>('A', 'Z');
    alphabetical |= range<int>('-');
    
    alphaNumeric |= range<int>('a', 'z');
    alphaNumeric |= range<int>('A', 'Z');
    alphaNumeric |= range<int>('-');
    alphaNumeric |= range<int>('0', '9');
    
    has_duplicates5.identifier_for_symbols(notSlash);
    has_duplicates5.identifier_for_symbols(alphabetical);
    has_duplicates5.identifier_for_symbols(alphaNumeric);
    has_duplicates5.identifier_for_symbols(range<int>('/'));
    has_duplicates5.identifier_for_symbols(range<int>('\\'));
    
    // Deduplicate it
    no_duplicates = remapped_symbol_map::deduplicate(has_duplicates5);
    
    report("NoDuplicates5", !no_duplicates->has_duplicates());
    report("AllRemapped5", check_ranges(has_duplicates5, *no_duplicates));

    // Finished with the set
    delete no_duplicates;
}
