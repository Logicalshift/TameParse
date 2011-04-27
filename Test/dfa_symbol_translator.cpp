//
//  dfa_symbol_translator.cpp
//  Parse
//
//  Created by Andrew Hunter on 27/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>

#include "dfa_symbol_translator.h"
#include "symbol_translator.h"

using namespace dfa;

void test_dfa_symbol_translator::run_tests() {
    // Create a symbol map containing all symbols
    symbol_map map1;
    int allSymbols = map1.identifier_for_symbols(range<int>(0, 0x7fffffff));
    
    // Create a translator for it
    symbol_translator trans1(map1);
    
    // Check that it contains all symbols
    std::cout << trans1.size();
    report("size1", trans1.size() < 32768);
    report("contains1-1", trans1.set_for_symbol(0) == allSymbols);
    report("contains1-2", trans1.set_for_symbol(256) == allSymbols);
    report("contains1-3", trans1.set_for_symbol(65536) == allSymbols);
    report("contains1-4", trans1.set_for_symbol(0x7ffffffe) == allSymbols);
    report("contains1-5", trans1.set_for_symbol(0x7fffffff) != allSymbols);
    report("contains1-6", trans1.set_for_symbol(-1) == symbol_set::null);

    // Create a symbol map containing some symbol ranges
    symbol_map map2;
    int firstRange = map2.identifier_for_symbols(range<int>(0, 32));
    int secondRange = map2.identifier_for_symbols(range<int>(32, 64));
    
    // Create a translator for it
    symbol_translator trans2(map2);
    
    report("size2", trans2.size() < 16384);
    report("contains2-1", trans2.set_for_symbol(0) == firstRange);
    report("contains2-2", trans2.set_for_symbol(31) == firstRange);
    report("contains2-3", trans2.set_for_symbol(32) == secondRange);
    report("contains2-4", trans2.set_for_symbol(63) == secondRange);
    report("contains2-5", trans2.set_for_symbol(64) == symbol_set::null);
    
    // Create a symbol map containing some larger symbol ranges
    symbol_map map3;
    firstRange = map3.identifier_for_symbols(range<int>(32, 65555));
    secondRange = map3.identifier_for_symbols(range<int>(70000, 70384));
    
    // Create a translator for it
    symbol_translator trans3(map3);
    
    report("size3", trans3.size() < 16384);
    report("contains3-1", trans3.set_for_symbol(32) == firstRange);
    report("contains3-2", trans3.set_for_symbol(65554) == firstRange);
    report("contains3-3", trans3.set_for_symbol(70000) == secondRange);
    report("contains3-4", trans3.set_for_symbol(70383) == secondRange);
    report("contains3-5", trans3.set_for_symbol(0) == symbol_set::null);
    report("contains3-6", trans3.set_for_symbol(31) == symbol_set::null);
    report("contains3-7", trans3.set_for_symbol(69999) == symbol_set::null);
    report("contains3-8", trans3.set_for_symbol(70384) == symbol_set::null);
}
