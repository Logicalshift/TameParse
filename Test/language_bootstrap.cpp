//
//  language_bootstrap.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>

#include "language_bootstrap.h"
#include "Language/bootstrap.h"
#include "Language/formatter.h"
#include "Lr/conflict.h"

using namespace std;
using namespace language;
using namespace lr;

void test_language_bootstrap::run_tests() {
    // Create a bootstrap object
    bootstrap bs;
    
    // Write out the language to test thigns
    wcerr << formatter::to_string(bs.get_grammar(), bs.get_terminals());
    
    // Get the conflicts in the grammar
    conflict_list conflicts;
    conflict::find_conflicts(bs.get_builder(), conflicts);
    
    report("NoConflicts", conflicts.size() == 0);
}
