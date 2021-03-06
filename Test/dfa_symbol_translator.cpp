//
//  dfa_symbol_translator.cpp
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

#include <iostream>

#include "dfa_symbol_translator.h"
#include "TameParse/Dfa/symbol_translator.h"

using namespace dfa;

void test_dfa_symbol_translator::run_tests() {
    // Create a symbol map containing all symbols
    symbol_map map1;
    int allSymbols = map1.identifier_for_symbols(range<int>(0, 0x7fffffff));
    
    // Create a translator for it
    symbol_translator<int> trans1(map1);
    
    // Check that it contains all symbols
    report("size1", trans1.size() < 32768);
    report("contains1-1", trans1.set_for_symbol(0) == allSymbols);
    report("contains1-2", trans1.set_for_symbol(256) == allSymbols);
    report("contains1-3", trans1.set_for_symbol(65536) == allSymbols);
    report("contains1-4", trans1.set_for_symbol(0x7ffffffe) == allSymbols);
    report("contains1-5", trans1.set_for_symbol(0x7fffffff) != allSymbols); // Ranges are exclusive, so we can't include the largest int value
    report("contains1-6", trans1.set_for_symbol(-1) == symbol_set::null);

    // Create a symbol map containing some symbol ranges
    symbol_map map2;
    int firstRange = map2.identifier_for_symbols(range<int>(0, 32));
    int secondRange = map2.identifier_for_symbols(range<int>(32, 64));
    
    // Create a translator for it
    symbol_translator<int> trans2(map2);
    
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
    symbol_translator<int> trans3(map3);
    
    report("size3", trans3.size() < 16384);
    report("contains3-1", trans3.set_for_symbol(32) == firstRange);
    report("contains3-2", trans3.set_for_symbol(65554) == firstRange);
    report("contains3-3", trans3.set_for_symbol(70000) == secondRange);
    report("contains3-4", trans3.set_for_symbol(70383) == secondRange);
    report("contains3-5", trans3.set_for_symbol(0) == symbol_set::null);
    report("contains3-6", trans3.set_for_symbol(31) == symbol_set::null);
    report("contains3-7", trans3.set_for_symbol(69999) == symbol_set::null);
    report("contains3-8", trans3.set_for_symbol(70384) == symbol_set::null);

    // Create a symbol map containing all characters
    symbol_map map4;
    allSymbols = map4.identifier_for_symbols(range<int>(0, 0x100));
    
    // Create a translator for it
    symbol_translator<unsigned char> trans4(map4);
    
    // Check that it contains all symbols
    report("size4", trans4.size() < 2048);
    report("contains4-1", trans4.set_for_symbol(0) == allSymbols);
    report("contains4-2", trans4.set_for_symbol(255) == allSymbols);
}
