//
//  lr_lalr_general.cpp
//  Parse
//
//  Created by Andrew Hunter on 02/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <sstream>
#include <iostream>

#include "lr_lalr_general.h"

#include "Dfa/character_lexer.h"
#include "ContextFree/grammar.h"
#include "Lr/lalr_builder.h"
#include "Lr/parser.h"

using namespace std;
using namespace dfa;
using namespace contextfree;
using namespace lr;

static void dump(const item& it, const grammar& gram) {
    if (it.type() == item::nonterminal) {
        wcerr << gram.name_for_nonterminal(it.symbol());
        wcerr << "(" << gram.identifier_for_item(it) << L")";
    } else if (it.type() == item::terminal) {
        wcerr << L"'" << (wchar_t) it.symbol() << L"'";
    } else if (it.type() == item::eoi) {
        wcerr << L"$";
        wcerr << "(" << gram.identifier_for_item(it) << L")";
    } else if (it.type() == item::empty) {
        wcerr << L"#";
        wcerr << "(" << gram.identifier_for_item(it) << L")";
    } else {
        wcerr << L"?";
        wcerr << "(" << gram.identifier_for_item(it) << L")";
    }
}

static void dump(const item_set& la, const grammar& gram) {
    for (item_set::const_iterator it = la.begin(); it != la.end(); it++) {
        wcerr << L" ";
        dump(**it, gram);
    }
}

static void dump(const rule& rule, const grammar& gram) {
    wcerr << gram.identifier_for_rule(rule) << L": ";
    dump(*rule.nonterminal(), gram);
    
    wcerr << L" ->";
    int pos;
    for (pos = 0; pos < rule.items().size(); pos++) {
        wcerr << L" ";
        dump(*rule.items()[pos], gram);
    }
}

static void dump(const lr0_item& item, const item_set& la, const grammar& gram) {
    wcerr << gram.identifier_for_rule(item.rule()) << L": ";
    dump(*item.rule()->nonterminal(), gram);
    
    wcerr << L" ->";
    int pos;
    for (pos = 0; pos < item.rule()->items().size(); pos++) {
        if (pos == item.offset()) wcerr << L" *";
        wcerr << L" ";
        dump(*item.rule()->items()[pos], gram);
    }
    if (pos == item.offset()) wcerr << L" *";
    
    wcerr << L",";
    dump(la, gram);
    
    wcerr << L" [";
    if (item.offset() < item.rule()->items().size()) {
        dump(*item.rule()->items()[item.offset()], gram);
        wcerr << L" >FIRST>";
        dump(gram.first(*item.rule()->items()[item.offset()]), gram);
    }
    wcerr << L"]";
}

static void dump(const lr_action_set& actions, const grammar& gram) {
    for (lr_action_set::const_iterator it = actions.begin(); it != actions.end(); it++) {
        wcerr << L"  ";
        switch ((*it)->type()) {
            case lr_action::act_accept:
                wcerr << L"ACCEPT ";
                break;
            case lr_action::act_reduce:
                wcerr << L"REDUCE ";
                break;
            case lr_action::act_shift:
                wcerr << L"SHIFT ";
                break;
            case lr_action::act_weakreduce:
                wcerr << L"WEAK REDUCE ";
                break;
            case lr_action::act_goto:
                wcerr << L"GOTO ";
                break;
            case lr_action::act_ignore:
                wcerr << L"IGNORE ";
                break;
        }
        dump(*(*it)->item(), gram);
        
        if ((*it)->type() == lr_action::act_reduce || (*it)->type() == lr_action::act_weakreduce) {
            wcerr << L" on ";
            dump(*(*it)->rule(), gram);
        } else {
            wcerr << L" -> " << (*it)->next_state();
        }
        wcerr << endl;
    }
}

/// \brief Dumps out a state machine to wcerr
static void dump_machine(const lalr_builder& builder) {
    item_set empty_set;
    empty_item empty;
    empty_set.insert(empty);
    const lalr_machine& machine = builder.machine();
    
    for (int stateId = 0; stateId < machine.count_states(); stateId++) {
        wcerr << L"STATE " << stateId << endl;
        
        const lalr_state& state = *machine.state_with_id(stateId);
        lr1_item_set closure;
        
        for (int itemId = 0; itemId < state.count_items(); itemId++) {
            const lr0_item& item = *state[itemId];
            wcerr << L"  ";
            dump(item, state.lookahead_for(itemId), machine.gram());
            wcerr << endl;
            
            if (item.offset() < item.rule()->items().size()) {
                lr1_item lr1(item, empty_set);
                item.rule()->items()[item.offset()]->closure(lr1, closure, machine.gram());
            }
        }

        if (!closure.empty()) {
            wcerr << endl;
            for (lr1_item_set::iterator closed = closure.begin(); closed != closure.end(); closed++) {
                wcerr << L"  ";
                dump(**closed, (*closed)->lookahead(), machine.gram());
                wcerr << endl;
            }
        }
        
        wcerr << endl;
        
        dump(builder.actions_for_state(stateId), builder.gram());

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
    builder.add_initial_state(s);
    builder.complete_parser();
    
    dump_machine(builder);
    
    // Create a parser for this grammar
    simple_parser p(builder);
    character_lexer lex;
    
    string test1("i");
    string test2("*i=i");
    
    stringstream stream1(test1);
    stringstream stream2(test2);
    
    simple_parser::state* parse1 = p.create_parser(new simple_parser_actions(lex.create_stream_from(stream1)));
    simple_parser::state* parse2 = p.create_parser(new simple_parser_actions(lex.create_stream_from(stream2)));
    
    // Test the parser
    report("accept1", parse1->parse());
    report("accept2", parse2->parse());
    
    delete parse1;
    delete parse2;
    
    // Run the parser 50000 times
    for (int x=0; x<50000; x++) {
        stringstream stream2(test2);
        simple_parser::state* parse2 = p.create_parser(new simple_parser_actions(lex.create_stream_from(stream2)));
        
        parse2->parse();
        delete parse2;
    }
    
    // Build the parser 10000 times
    for (int x=0; x<10000; x++) {
        // Build this grammar
        lalr_builder builder(dragon446);
        
        // S' defines the language
        builder.add_initial_state(s);
        builder.complete_parser();
    }
}
