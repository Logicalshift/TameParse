//
//  lr_lalr_general.cpp
//  Parse
//
//  Created by Andrew Hunter on 02/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>

#include "lr_lalr_general.h"

#include "ContextFree/grammar.h"
#include "Lr/lalr_builder.h"

using namespace std;
using namespace contextfree;
using namespace lr;

static void dump(const item& it, const grammar& gram) {
    if (it.type() == item::nonterminal) {
        wcerr << gram.name_for_nonterminal(it.symbol());
    } else if (it.type() == item::terminal) {
        wcerr << L"'" << (wchar_t) it.symbol() << L"'";
    } else {
        wcerr << L"?";
    }
}

/// \brief Dumps out a state machine to wcerr
static void dump_machine(const lalr_machine& machine) {
    for (int stateId = 0; stateId < machine.count_states(); stateId++) {
        wcerr << L"STATE " << stateId << endl;
        
        const lalr_state& state = *machine.state_with_id(stateId);
        
        for (int itemId = 0; itemId < state.count_items(); itemId++) {
            const lr0_item& item = *state[itemId];
            wcerr << L"  ";
            dump(*item.rule().nonterminal(), machine.gram());
            
            wcerr << L" -> ";
            int pos;
            for (pos = 0; pos < item.rule().items().size(); pos++) {
                if (pos == item.offset()) wcerr << L"* ";
                dump(*item.rule().items()[pos], machine.gram());
            }
            if (pos == item.offset()) wcerr << L"* ";
            
            wcerr << endl;
        }
        
        wcerr << endl;
    }
}

void test_lalr_general::run_tests() {
    // Grammar specified in example 4.46 of the dragon book
    grammar dragon446;
    
    nonterminal sPrime(dragon446.id_for_nonterminal(L"S'"));
    nonterminal s(dragon446.id_for_nonterminal(L"S"));
    nonterminal l(dragon446.id_for_nonterminal(L"L"));
    nonterminal r(dragon446.id_for_nonterminal(L"R"));
    
    terminal equals('=');
    terminal times('*');
    terminal id('i');
    
    // S' -> S
    (dragon446 += sPrime) << s;
    
    // S -> L = R | R
    (dragon446 += s) << l << equals << r;
    (dragon446 += s) << r;
    
    // L -> * R | id
    (dragon446 += l) << times << r;
    (dragon446 += l) << id;
    
    // R -> L
    (dragon446 += r) << l;
    
    // Build this grammar
    lalr_builder builder(dragon446);
    
    // S' defines the language
    builder.add_initial_state(sPrime);
    builder.complete_parser();
    
    dump_machine(builder.machine());
}
