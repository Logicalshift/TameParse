//
//  lr_weaksymbols.cpp
//  Parse
//
//  Created by Andrew Hunter on 25/06/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "lr_weaksymbols.h"

#include "Dfa/ndfa_regex.h"
#include "Lr/weak_symbols.h"

using namespace dfa;
using namespace contextfree;
using namespace lr;

void test_lr_weaksymbols::run_tests() {
    // Create a simple terminal dictionary
    terminal_dictionary terminals;
    
    // Add some symbols. Weak symbols are higher priority, so they come first
    int integer     = terminals.add_symbol(L"integer");
    int real        = terminals.add_symbol(L"real");
    
    int identifier  = terminals.add_symbol(L"identifier");
    
    // Create some symbol objects for these items
    terminal tInteger(integer);
    terminal tReal(real);
    terminal tIdentifier(identifier);
    
    item_container icInteger(&tInteger, false);
    item_container icReal(&tReal, false);
    item_container icIdentifier(&tIdentifier, false);
    
    // First check that if we add a symbol that it does actually get added
    weak_symbols    manuallyAdded;
    item_set        realAndInt;
    
    realAndInt.insert(icInteger);
    realAndInt.insert(icReal);
    
    manuallyAdded.add_symbols(icIdentifier, realAndInt);
    
    // Some simple checks
    report("ManualAdded", manuallyAdded.weak_for_strong(icIdentifier) == realAndInt);
    report("ManualNotAdded", manuallyAdded.weak_for_strong(icReal).empty());
    
    // Now generate a DFA containing an identifier and a 'real' and 'integer' node
    ndfa_regex simpleLexerNdfa;
    
    simpleLexerNdfa.add_regex(0, L"integer", integer);
    simpleLexerNdfa.add_regex(0, L"real", real);
    simpleLexerNdfa.add_regex(0, L"[A-Za-z\\-][A-Za-z\\-0-9]*", identifier);
    
    ndfa* simpleLexerDeduped    = simpleLexerNdfa.to_ndfa_with_unique_symbols();
    ndfa* simpleLexer           = simpleLexerDeduped->to_dfa();
    
    report("SimpleLexerIsDfa", simpleLexer->verify_is_dfa());
    
    // Once we've got a DFA, there should be states that have actions on both the 'keyword' items and the 'identifier' item
    // These tests verify that any issues with the weak symbols routine aren't down to the generation of the DFA
    bool matchesInt     = false;
    bool matchesReal    = false;
    
    typedef ndfa::accept_action_list aal;
    
    for (int stateId=0; stateId < simpleLexer->count_states(); stateId++) {
        // Get the actions for this state
        const aal& actions = simpleLexer->actions_for_state(stateId);
        
        // Nothing to do if there are no actions for this state (hrm, shouldn't actually be any of these in reality)
        if (actions.empty()) continue;
        
        // Iterate through the actions and find out which symbols it matches
        bool hasIdentifier  = false;
        bool hasInt         = false;
        bool hasReal        = false;
        
        for (aal::const_iterator nextAction = actions.begin(); nextAction != actions.end(); nextAction++) {
            if ((*nextAction)->symbol() == integer) {
                hasInt          = true;
                matchesInt      = true;
            } else if ((*nextAction)->symbol() == real) {
                hasReal         = true;
                matchesReal     = true;
            } else if ((*nextAction)->symbol() == identifier) {
                hasIdentifier   = true;
            }
        }
        
        if (hasReal) {
            report("RealAndIdentifier", hasReal && hasIdentifier);
            report("RealAndNotInteger", hasReal && !hasInt);
        } else if (hasInt) {
            report("IntegerAndIdentifier", hasInt && hasIdentifier);
            report("IntegerAndNotReal", hasInt && !hasReal);            
        }
    }
    
    report("MatchesInt", matchesInt);
    report("MatchesReal", matchesReal);
    
    // Try creating the same conflict we created manually above, this time using the DFA
    weak_symbols dfaWeak;
    
    // Add the symbols as before
    dfaWeak.add_symbols(*simpleLexer, realAndInt, terminals);
    
    // Check for the appropriate conflicts
    const item_set& identifierWeak = dfaWeak.weak_for_strong(icIdentifier);
    
    report("IdentifierHasTwoWeak", dfaWeak.weak_for_strong(icIdentifier).size() == 2);
    report("RealHasNoWeak", dfaWeak.weak_for_strong(icReal).empty());
    report("IntegerHasNoWeak", dfaWeak.weak_for_strong(icInteger).empty());
    report("RealIsWeakForId", identifierWeak.find(icReal) != identifierWeak.end());
    report("IntegerIsWeakForId", identifierWeak.find(icInteger) != identifierWeak.end());
    report("IdentifierIsNotWeak", identifierWeak.find(icIdentifier) == identifierWeak.end());
    
    // Tidy up the lexers
    delete simpleLexerDeduped;
    delete simpleLexer;
}
