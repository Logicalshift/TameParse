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
static bool test_lex(string phrase, const lexer& lex, int expectedSymbol, const terminal_dictionary& terms) {
    // Create a lexeme stream
    stringstream source(phrase);
    lexeme_stream* lxs = lex.create_stream_from(source);
    
    // Get the next lexeme
    lexeme* match;
    (*lxs) >> match;
    
    bool result = true;
    if (match->content().size() != phrase.size()) {
        for (int x=0; x<match->content().size(); x++) {
            cerr << (char)match->content()[x];
        }
        cerr << endl;
        result = false;
    }
    if (match->matched() != expectedSymbol) {
        wcerr << terms.name_for_symbol(match->matched()) << endl;
        result = false;
    }
    
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
    report("MatchIdentifier1", test_lex("fdsu", bs.get_lexer(), bs.get_terminal_items().identifier->symbol(), bs.get_terminals()));
    report("MatchIdentifier2", test_lex("some-identifier", bs.get_lexer(), bs.get_terminal_items().identifier->symbol(), bs.get_terminals()));
    report("MatchIdentifier3", test_lex("id128", bs.get_lexer(), bs.get_terminal_items().identifier->symbol(), bs.get_terminals()));
    report("MatchIdentifier4", test_lex("identifier", bs.get_lexer(), bs.get_terminal_items().identifier->symbol(), bs.get_terminals()));
    report("MatchNonterminal1", test_lex("<nonterminal>", bs.get_lexer(), bs.get_terminal_items().nonterminal->symbol(), bs.get_terminals()));
    report("MatchRegex1", test_lex("/regex/", bs.get_lexer(), bs.get_terminal_items().regex->symbol(), bs.get_terminals()));
    report("MatchString1", test_lex("\"string\"", bs.get_lexer(), bs.get_terminal_items().string->symbol(), bs.get_terminals()));
    report("MatchCharacter1", test_lex("'c'", bs.get_lexer(), bs.get_terminal_items().character->symbol(), bs.get_terminals()));
    report("MatchCharacter2", test_lex("'\\n'", bs.get_lexer(), bs.get_terminal_items().character->symbol(), bs.get_terminals()));
    
    report("MatchLanguage", test_lex("language", bs.get_lexer(), bs.get_terminal_items().language->symbol(), bs.get_terminals()));
    report("MatchGrammar", test_lex("grammar", bs.get_lexer(), bs.get_terminal_items().grammar->symbol(), bs.get_terminals()));
    report("MatchLexer", test_lex("lexer", bs.get_lexer(), bs.get_terminal_items().lexer->symbol(), bs.get_terminals()));
    report("MatchLexerSymbols", test_lex("lexer-symbols", bs.get_lexer(), bs.get_terminal_items().lexersymbols->symbol(), bs.get_terminals()));
    report("MatchWeakLexer", test_lex("weaklexer", bs.get_lexer(), bs.get_terminal_items().weaklexer->symbol(), bs.get_terminals()));
    report("MatchIgnore", test_lex("ignore", bs.get_lexer(), bs.get_terminal_items().ignore->symbol(), bs.get_terminals()));
    report("MatchKeywords", test_lex("keywords", bs.get_lexer(), bs.get_terminal_items().keywords->symbol(), bs.get_terminals()));
    report("MatchWhitespace", test_lex("  ", bs.get_lexer(), bs.get_terminal_items().whitespace->symbol(), bs.get_terminals()));
    report("MatchNewline", test_lex("\n", bs.get_lexer(), bs.get_terminal_items().newline->symbol(), bs.get_terminals()));
    report("MatchComment", test_lex("// Comment", bs.get_lexer(), bs.get_terminal_items().comment->symbol(), bs.get_terminals()));
    
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
    
    // Create a stream for the language definition
    stringstream bootstrapDefinition(bootstrap::get_default_language_definition());
    
    report("LanguageDefinitionHasData", bootstrap::get_default_language_definition().size() > 0);
    
    // Create a lexer for it
    lexeme_stream* defaultStream = bs.get_lexer().create_stream_from(bootstrapDefinition);
    
    // Create a parser for it
    ast_parser::state* defParser = bs.get_parser().create_parser(new ast_parser_actions(defaultStream));
    
    // Try parsing the language
    bool acceptedDefault = defParser->parse();
    
    report("CanParseLanguageDefinition", acceptedDefault);
    
    delete defParser;
    delete defaultStream;
    
    // Create a stream o' nonsense, and parse it
    stringstream nonsense("rhubarb rhubarb rhubarb");
    lexeme_stream* nonsenseStream = bs.get_lexer().create_stream_from(nonsense);
    ast_parser::state* nonsenseParser = bs.get_parser().create_parser(new ast_parser_actions(nonsenseStream));
    
    report("CantParseNonsense", !nonsenseParser->parse());
    
    delete nonsenseParser;
    delete nonsenseStream;
}
