//
//  hard_coded_symbol_table.cpp
//  Parse
//
//  Created by Andrew Hunter on 03/09/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#include "TameParse/Dfa/hard_coded_symbol_table.h"

using namespace dfa;

#ifdef DEBUG

/// \brief Test class to highlight any compilation errors in the class in debug builds
static int some_ints[] = { 1,2,3 };
static hard_coded_symbol_table<wchar_t, 2> s_WideCharSymbolTableTest(some_ints);

static void lookup_test() {
	s_WideCharSymbolTableTest.lookup(L'x');
}

#endif
