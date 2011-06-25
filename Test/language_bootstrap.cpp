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
using namespace dfa;
using namespace language;
using namespace lr;

void test_language_bootstrap::run_tests() {
    // Create a bootstrap object
    bootstrap bs;
    
    // Write out the language to test thigns
    wcerr << formatter::to_string(bs.get_builder(), bs.get_grammar(), bs.get_terminals()) << endl;
    
    // Get the conflicts in the grammar
    conflict_list conflicts;
    conflict::find_conflicts(bs.get_builder(), conflicts);
    
    ndfa* bsDfa = bs.create_dfa();
    report("DfaIsDfa", bsDfa->is_dfa());
    report("DfaVerifyDfa", bsDfa->verify_is_dfa());
    
    delete bsDfa;
    
    report("NoConflicts", conflicts.size() == 0);
    
    // Write out the conflicts to the standard I/O if there were any
    if (conflicts.size() > 0) {
        for (conflict_list::const_iterator it = conflicts.begin(); it != conflicts.end(); it++) {
            wcerr << endl << L"===" << endl << formatter::to_string(**it, bs.get_grammar(), bs.get_terminals()) << endl << L"===" << endl;
        }
    }
}
