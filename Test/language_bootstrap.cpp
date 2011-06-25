//
//  language_bootstrap.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <string>
#include <sstream>
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

// Checks that a given phrase is lexed as gthe specified symbol
static bool test_lex(string phrase, const lexer& lex, int expectedSymbol) {
    // Create a lexeme stream
    stringstream source(phrase);
    lexeme_stream* lxs = lex.create_stream_from(source);
    
    // Get the next lexeme
    lexeme* match;
    (*lxs) >> match;
    
    bool result = true;
    if (match->content().size() != phrase.size())   result = false;
    if (match->matched() != expectedSymbol)         result = false;
    
    // Finished with the stream
    delete match;
    delete lxs;
    
    return result;
}

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
    report("DfaNoOverlap", bsDfa->verify_no_symbol_overlap());
    
    // The 'identifier' symbol should share its actions with some of the weak symbols in the language
    const item_container& icIdentifier  = bs.get_terminal_items().identifier;
    const item_container& icLanguage    = bs.get_terminal_items().language;
    const item_container& icLexer       = bs.get_terminal_items().lexer;
    const item_container& icEquals      = bs.get_terminal_items().equals;
    
    bool anyIds         = false;
    bool anyLanguage    = false;
    bool anyLexer       = false;
    bool idAndLanguage  = false;
    bool idAndLexer     = false;
    bool idAndAnything  = false;
    bool idAndEquals    = false;
    
    for (int stateId = 0; stateId < bsDfa->count_states(); stateId++) {
        typedef ndfa::accept_action_list aal;
        
        const aal& actions = bsDfa->actions_for_state(stateId);
        if (actions.empty()) continue;
        
        bool hasId          = false;
        bool hasLanguage    = false;
        bool hasLexer       = false;
        bool hasEquals      = false;
        
        for (aal::const_iterator nextAction = actions.begin(); nextAction != actions.end(); nextAction++) {
            if ((*nextAction)->symbol() == icIdentifier->symbol())  hasId = true;
            if ((*nextAction)->symbol() == icLanguage->symbol())    hasLanguage = true;
            if ((*nextAction)->symbol() == icLexer->symbol())       hasLexer = true;
            if ((*nextAction)->symbol() == icEquals->symbol())      hasEquals = true;
        }
        
        if (hasLanguage) {
            anyLanguage = true;
            if (!idAndLanguage) idAndLanguage = hasId;
        }
        
        if (hasLexer) {
            anyLexer = true;
            if (!idAndLexer) idAndLexer = hasId;
        }
        
        if (hasEquals) {
            if (!idAndEquals) idAndEquals = hasId;
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
    report("IdentifierAndNotEquals", !idAndEquals);
    
    // Finished looking at the DFA
    delete bsDfa;
    
    // Test that the lexer can match some particular symbols
    report("MatchIdentifier1", test_lex("fdsu", bs.get_lexer(), bs.get_terminal_items().identifier->symbol()));
    report("MatchIdentifier2", test_lex("some-identifier", bs.get_lexer(), bs.get_terminal_items().identifier->symbol()));
    report("MatchIdentifier3", test_lex("id128", bs.get_lexer(), bs.get_terminal_items().identifier->symbol()));
    report("MatchNonterminal1", test_lex("<nonterminal>", bs.get_lexer(), bs.get_terminal_items().nonterminal->symbol()));
    report("MatchRegex1", test_lex("/regex/", bs.get_lexer(), bs.get_terminal_items().regex->symbol()));
    report("MatchString1", test_lex("\"string\"", bs.get_lexer(), bs.get_terminal_items().string->symbol()));
    report("MatchCharacter1", test_lex("'c'", bs.get_lexer(), bs.get_terminal_items().character->symbol()));
    
    report("MatchLanguage", test_lex("language", bs.get_lexer(), bs.get_terminal_items().language->symbol()));
    report("MatchGrammar", test_lex("grammar", bs.get_lexer(), bs.get_terminal_items().grammar->symbol()));
    report("MatchLexer", test_lex("lexer", bs.get_lexer(), bs.get_terminal_items().lexer->symbol()));
    report("MatchLexerSymbols", test_lex("lexer-symbols", bs.get_lexer(), bs.get_terminal_items().lexersymbols->symbol()));
    report("MatchWeakLexer", test_lex("weaklexer", bs.get_lexer(), bs.get_terminal_items().weaklexer->symbol()));
    report("MatchIgnore", test_lex("ignore", bs.get_lexer(), bs.get_terminal_items().ignore->symbol()));
    report("MatchKeywords", test_lex("keywords", bs.get_lexer(), bs.get_terminal_items().keywords->symbol()));
    
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
