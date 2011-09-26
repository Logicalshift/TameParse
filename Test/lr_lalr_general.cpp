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

#include "TameParse/Dfa/character_lexer.h"
#include "TameParse/ContextFree/grammar.h"
#include "TameParse/Lr/lalr_builder.h"
#include "TameParse/Lr/parser.h"
#include "TameParse/Lr/conflict.h"
#include "TameParse/Language/formatter.h"

using namespace std;
using namespace dfa;
using namespace contextfree;
using namespace language;
using namespace lr;

static void dump(const item& it, const grammar& gram, const terminal_dictionary& dict) {
    if (it.type() == item::nonterminal) {
        wcerr << gram.name_for_nonterminal(it.symbol());
        wcerr << "(" << gram.identifier_for_item(it) << L")";
    } else if (it.type() == item::terminal) {
        wcerr << dict.name_for_symbol(it.symbol());
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

static void dump(const item_set& la, const grammar& gram, const terminal_dictionary& dict) {
    for (item_set::const_iterator it = la.begin(); it != la.end(); it++) {
        wcerr << L" ";
        dump(**it, gram, dict);
    }
}

static void dump(const rule& rule, const grammar& gram, const terminal_dictionary& dict) {
    wcerr << gram.identifier_for_rule(rule) << L": ";
    dump(*rule.nonterminal(), gram, dict);
    
    wcerr << L" ->";
    int pos;
    for (pos = 0; pos < rule.items().size(); pos++) {
        wcerr << L" ";
        dump(*rule.items()[pos], gram, dict);
    }
}

static void dump(const lr0_item& item, const item_set& la, const grammar& gram, const terminal_dictionary& dict) {
    wcerr << gram.identifier_for_rule(item.rule()) << L": ";
    dump(*item.rule()->nonterminal(), gram, dict);
    
    wcerr << L" ->";
    int pos;
    for (pos = 0; pos < item.rule()->items().size(); pos++) {
        if (pos == item.offset()) wcerr << L" *";
        wcerr << L" ";
        dump(*item.rule()->items()[pos], gram, dict);
    }
    if (pos == item.offset()) wcerr << L" *";
    
    wcerr << L",";
    dump(la, gram, dict);
    
    wcerr << L" [";
    if (item.offset() < item.rule()->items().size()) {
        dump(*item.rule()->items()[item.offset()], gram, dict);
        wcerr << L" >FIRST>";
        dump(gram.first(*item.rule()->items()[item.offset()]), gram, dict);
    }
    wcerr << L"]";
}

static void dump(const lr_action_set& actions, const grammar& gram, const terminal_dictionary& dict) {
    for (lr_action_set::const_iterator it = actions.begin(); it != actions.end(); it++) {
        wcerr << L"  ";
        switch ((*it)->type()) {
            case lr_action::act_accept:
                wcerr << L"ACCEPT ";
                break;
            case lr_action::act_divert:
                wcerr << L"DIVERT ";
                break;
            case lr_action::act_guard:
                wcerr << L"GUARD ";
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
            case lr_action::act_shiftstrong:
                wcerr << L"SHIFT-STRONG ";
                break;
        }
        dump(*(*it)->item(), gram, dict);
        
        if ((*it)->type() == lr_action::act_reduce || (*it)->type() == lr_action::act_weakreduce) {
            wcerr << L" on ";
            dump(*(*it)->rule(), gram, dict);
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
        
        for (lalr_state::iterator nextItem = state.begin(); nextItem != state.end(); nextItem++) {
            int itemId = state.find_identifier(*nextItem);
            const lr0_item& item = *state[itemId];
            wcerr << L"  ";
            dump(item, state.lookahead_for(itemId), machine.gram(), builder.terminals());
            wcerr << endl;
            
            if (item.offset() < item.rule()->items().size()) {
                lr1_item lr1(item, empty_set);
                item.rule()->items()[item.offset()]->cache_closure(lr1, closure, machine.gram());
            }
        }

        if (!closure.empty()) {
            wcerr << endl;
            for (lr1_item_set::iterator closed = closure.begin(); closed != closure.end(); closed++) {
                wcerr << L"  ";
                dump(**closed, (*closed)->lookahead(), machine.gram(), builder.terminals());
                wcerr << endl;
            }
        }
        
        wcerr << endl;
        
        dump(builder.actions_for_state(stateId), builder.gram(), builder.terminals());

        wcerr << endl;
    }
}

static bool state_comparison_always_reversible(lalr_machine& m) {
    // If state X is less than state Y then state Y must not be less than state X
    for (int stateX = 0; stateX < m.count_states(); stateX++) {
        const lalr_state& stateXreal = *m.state_with_id(stateX);
        
        for (int stateY = 0; stateY < m.count_states(); stateY++) {
            const lalr_state& stateYreal = *m.state_with_id(stateY);
            
            if (stateXreal < stateYreal) {
                if (stateYreal < stateXreal) {
                    // DOH!
                    wcerr << stateX << L" is less than " << stateY << L" but is also greater than or equal to it!" << endl;
                    return false;
                }
            }
        }
    }
    
    return true;
}

static bool no_duplicate_states(lalr_machine& m) {
    // State X and state Y can only be the same if they have the same ID
    bool ok = true;
    
    for (int stateX = 0; stateX < m.count_states(); stateX++) {
        const lalr_state& stateXreal = *m.state_with_id(stateX);
        
        for (int stateY = 0; stateY < m.count_states(); stateY++) {
            const lalr_state& stateYreal = *m.state_with_id(stateY);
            
            if (stateXreal == stateYreal) {
                if (stateX != stateY) {
                    // DOH!
                    wcerr << stateX << L" is a duplicate of " << stateY << endl;
                    ok = false;
                }
            } else {
                if (stateX == stateY) {
                    // DOH!
                    wcerr << stateX << L" isn't equal to itself!" << endl;
                    ok = false;
                }
            }
        }
    }
    
    return ok;
}

typedef basic_string<int> symbol_string;
typedef basic_stringstream<int> symbol_stringstream;

bool can_parse(symbol_string& symbols, simple_parser& p, character_lexer& lex) {
    symbol_stringstream stream(symbols);
    simple_parser::state* state = p.create_parser(new simple_parser_actions(lex.create_stream_from(stream)));
    
    bool result = state->parse();
    
    delete state;
    return result;
}

void test_lalr_general::run_tests() {
    // Grammar specified in example 4.46 of the dragon book
    grammar             dragon446;
    terminal_dictionary terms;
    
    nonterminal sPrime(dragon446.id_for_nonterminal(L"S'"));
    nonterminal s(dragon446.id_for_nonterminal(L"S"));
    nonterminal l(dragon446.id_for_nonterminal(L"L"));
    nonterminal r(dragon446.id_for_nonterminal(L"R"));
    
    int equalsId    = terms.add_symbol(L"'='");
    int timesId     = terms.add_symbol(L"'*'");
    int idId        = terms.add_symbol(L"'i'");
    
    terminal equals(equalsId);
    terminal times(timesId);
    terminal id(idId);
    
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
    lalr_builder builder(dragon446, terms);
    
    // S' defines the language
    builder.add_initial_state(s);
    builder.complete_parser();
    
    dump_machine(builder);
    
    // Some tests with LR(1) items (putting them here to save a lot of work with the setup)
    item_set set1;
    set1.insert(new terminal(1));
    item_set set2;
    set2.insert(new terminal(0));
    
    lr1_item item1(&dragon446, dragon446.rule_with_identifier(0), 0, set1);
    lr1_item item2(&dragon446, dragon446.rule_with_identifier(0), 0, set2);
    
    // Standard LR(1) item set performs merging
    lr1_item_set is2;
    is2.insert(item1);
    is2.insert(item2);
    
    report("Multi-Lr1-items-merging", is2.size() == 1);
    
    // Should be possible to distinguish items based only on lookahead when in their own set
    set<lr1_item> is;
    is.insert(item1);
    is.insert(item2);

    report("Multi-Lr1-items1", is.size() == 2);
    report("Multi-Lr1-items2", is.size() != 1);
    report("Lr1-ItemCompare1", item1 != item2);
    report("Lr1-ItemCompare2", item1 < item2 || item2 < item1);
    
    // Assert some things about the machine (specified in the dragon book)
    report("NumStates", builder.machine().count_states() == 10); // Figure 4.42: the result should have 10 states
    report("NotEqualSimple", (*builder.machine().state_with_id(1)) != (*builder.machine().state_with_id(6)));
    report("NoDuplicateStates", no_duplicate_states(builder.machine()));
    report("StateOrderingWorks", state_comparison_always_reversible(builder.machine()));
    
    // Create a parser for this grammar
    simple_parser p(builder, NULL);
    character_lexer lex;
    
    symbol_string test1;
    symbol_string test2;
    
    test1 += idId;
    test2 += timesId;
    test2 += idId;
    test2 += equalsId;
    test2 += idId;
    
    symbol_stringstream stream1(test1);
    symbol_stringstream stream2(test2);
    
    simple_parser::state* parse1 = p.create_parser(new simple_parser_actions(lex.create_stream_from(stream1)));
    simple_parser::state* parse2 = p.create_parser(new simple_parser_actions(lex.create_stream_from(stream2)));
    
    // Test the parser
    report("Accept1", parse1->parse());
    report("Accept2", parse2->parse());

    conflict_list conflicts;
    conflict::find_conflicts(builder, conflicts);
    
    report("NoConflicts1", conflicts.size() == 0);

    delete parse1;
    delete parse2;
    
    // Create another parser, this one with a particular type of empty production (accepts arbitrary strings of ids)
    grammar emptyProd;

    nonterminal sPrime2(emptyProd.id_for_nonterminal(L"S'"));
    nonterminal s2(emptyProd.id_for_nonterminal(L"S"));

    (emptyProd += sPrime2) << s2;
    (emptyProd += s2);
    (emptyProd += s2) << s2 << idId;
    
    lalr_builder emptyBuilder(emptyProd, terms);

    emptyBuilder.add_initial_state(sPrime);
    emptyBuilder.complete_parser();

    dump_machine(emptyBuilder);
    simple_parser emptyParser(emptyBuilder, NULL);
    
    // Some simple checks to see if this parser works
    symbol_string empty;
    symbol_string oneId;
    symbol_string twoIds;
    symbol_string manyIds;

    oneId   += idId;
    twoIds  += idId;
    twoIds  += idId;
    for (int x=0; x<30; x++) manyIds += idId;
    
    // Should accept the lot
    conflicts.clear();
    conflict::find_conflicts(emptyBuilder, conflicts);
    
    report("NoConflicts2", conflicts.size() == 0);
    report("Empty", can_parse(empty, emptyParser, lex));
    report("OneId", can_parse(oneId, emptyParser, lex));
    report("TwoIds", can_parse(twoIds, emptyParser, lex));
    report("ManyIds", can_parse(manyIds, emptyParser, lex));

    // Test of our ability to accept some context-sensitive languages
    grammar contextSensitive;
    
    int aId = terms.add_symbol(L"'a'");
    int bId = terms.add_symbol(L"'b'");
    int cId = terms.add_symbol(L"'c'");

    terminal a(aId);
    terminal b(bId);
    terminal c(cId);
    
    nonterminal matchingBs(contextSensitive.id_for_nonterminal(L"<Matching-Bs>"));
    nonterminal matchingCs(contextSensitive.id_for_nonterminal(L"<Matching-Cs>"));
    nonterminal csLan(contextSensitive.id_for_nonterminal(L"<Context-Sensitive>"));
    
    ebnf_repeating someBs;
    (*someBs.get_rule()) << b;
    
    (contextSensitive += L"<Matching-Bs>") << a << matchingBs << b;
    (contextSensitive += L"<Matching-Bs>") << a << b;
    (contextSensitive += L"<Matching-Cs>") << a << matchingCs << c;
    (contextSensitive += L"<Matching-Cs>") << a << someBs << c;
    
    guard matchBguard;
    (*matchBguard.get_rule()) << matchingBs << c;
    
    (contextSensitive += L"<Context-Sensitive>") << matchBguard << matchingCs;
    
    // Build our context-sensitve parser
    lalr_builder csBuilder(contextSensitive, terms);
    csBuilder.add_initial_state(csLan);
    csBuilder.complete_parser();
    
    // Create a parser for this grammar
    simple_parser simpleCsParser(csBuilder, NULL);
    
    // And some test strings
    symbol_string threeOfEach;
    symbol_string csDoesntMatch1;
    symbol_string csDoesntMatch2;
    symbol_string csDoesntMatch3;
    
    for (int x=0; x<3; x++) threeOfEach += aId;
    for (int x=0; x<3; x++) threeOfEach += bId;
    for (int x=0; x<3; x++) threeOfEach += cId;
    
    for (int x=0; x<3; x++) csDoesntMatch1 += aId;
    for (int x=0; x<2; x++) csDoesntMatch1 += bId;
    for (int x=0; x<3; x++) csDoesntMatch1 += cId;

    for (int x=0; x<2; x++) csDoesntMatch2 += aId;
    for (int x=0; x<2; x++) csDoesntMatch2 += bId;
    for (int x=0; x<3; x++) csDoesntMatch2 += cId;
    
    for (int x=0; x<3; x++) csDoesntMatch3 += aId;
    for (int x=0; x<4; x++) csDoesntMatch3 += bId;
    for (int x=0; x<3; x++) csDoesntMatch3 += cId;

    // Now test it out
    report("ContextSensitive1", can_parse(threeOfEach, simpleCsParser, lex));
    report("ContextSensitive2", !can_parse(csDoesntMatch1, simpleCsParser, lex));
    report("ContextSensitive3", !can_parse(csDoesntMatch2, simpleCsParser, lex));
    report("ContextSensitive4", !can_parse(csDoesntMatch3, simpleCsParser, lex));
}
