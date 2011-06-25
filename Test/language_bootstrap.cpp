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
using namespace contextfree;
using namespace language;
using namespace lr;

void test_language_bootstrap::run_tests() {
    // Create a bootstrap object
    bootstrap bs;
    
    // Write out the language to test things
    wcerr << formatter::to_string(bs.get_builder(), bs.get_grammar(), bs.get_terminals()) << endl;
    
    // Verify that the weak symbols have a higher priority than the strong symbols
    report("WeakSymbols1", bs.get_terminal_items().identifier->symbol() > bs.get_terminal_items().language->symbol());
    
    // Verify that the lexer looks like a DFA
    ndfa* bsDfa = bs.create_dfa();

    report("DfaIsDfa", bsDfa->is_dfa());
    report("DfaVerifyDfa", bsDfa->verify_is_dfa());
    
    // The 'identifier' symbol should share its actions with some of the weak symbols in the language
    const item_container& icIdentifier  = bs.get_terminal_items().identifier;
    const item_container& icLanguage    = bs.get_terminal_items().language;
    const item_container& icLexer       = bs.get_terminal_items().lexer;
    
    bool anyIds         = false;
    bool anyLanguage    = false;
    bool anyLexer       = false;
    bool idAndLanguage  = false;
    bool idAndLexer     = false;
    bool idAndAnything  = false;
    
    for (int stateId = 0; stateId < bsDfa->count_states(); stateId++) {
        typedef ndfa::accept_action_list aal;
        
        const aal& actions = bsDfa->actions_for_state(stateId);
        if (actions.empty()) continue;
        
        bool hasId = false;
        bool hasLanguage = false;
        bool hasLexer = false;
        
        for (aal::const_iterator nextAction = actions.begin(); nextAction != actions.end(); nextAction++) {
            if ((*nextAction)->symbol() == icIdentifier->symbol()) hasId = true;
            if ((*nextAction)->symbol() == icLanguage->symbol()) hasLanguage = true;
            if ((*nextAction)->symbol() == icLexer->symbol()) hasLexer = true;
        }
        
        if (hasLanguage) {
            anyLanguage = true;
            if (!idAndLanguage) idAndLanguage = hasId;
        }
        
        if (hasLexer) {
            anyLexer = true;
            if (!idAndLexer) idAndLexer = hasId;
        }
        
        if (hasId) {
            anyIds = true;
            if (actions.size() > 1) {
                idAndAnything = true;
            }
        }
    }
    
    report("AnyIdentifiers", anyIds);
    report("AnyLanguage", anyLanguage);
    report("AnyLexer", anyLexer);
    report("IdentifierAndAnything", idAndAnything);
    report("IdentifierAndLanguage", idAndLanguage);
    report("IdentifierAndLexer", idAndLexer);
    
    // Finished looking at the DFA
    delete bsDfa;
    
    // Get the conflicts in the grammar
    conflict_list conflicts;
    conflict::find_conflicts(bs.get_builder(), conflicts);
    
    report("NoConflicts", conflicts.size() == 0);
    
    // Write out the conflicts to the standard I/O if there were any
    if (conflicts.size() > 0) {
        for (conflict_list::const_iterator it = conflicts.begin(); it != conflicts.end(); it++) {
            wcerr << endl << L"===" << endl << formatter::to_string(**it, bs.get_grammar(), bs.get_terminals()) << endl << L"===" << endl;
        }
    }
}
