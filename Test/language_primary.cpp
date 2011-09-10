//
//  language_primary.cpp
//  TameParse
//
//  Created by Andrew Hunter on 10/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <string>
#include <sstream>

#include "language_primary.h"
#include "tameparse_language.h"

using namespace std;
using namespace dfa;
using namespace language;

// Checks that a given phrase is lexed as the specified symbol
static bool test_lex(string phrase, const lexer& lex, int expectedSymbol) {
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
        result = false;
    }
    
    // Finished with the stream
    delete match;
    delete lxs;
    
    return result;
}

void test_language_primary::run_tests() {
	// Test that the lexer can match some particular symbols
    report("MatchIdentifier1", test_lex("fdsu", tameparse_language::lexer, tameparse_language::t::identifier));
    report("MatchIdentifier2", test_lex("some-identifier", tameparse_language::lexer, tameparse_language::t::identifier));
    report("MatchIdentifier3", test_lex("id128", tameparse_language::lexer, tameparse_language::t::identifier));
    report("MatchIdentifier4", test_lex("identifier", tameparse_language::lexer, tameparse_language::t::identifier));
    report("MatchNonterminal1", test_lex("<nonterminal>", tameparse_language::lexer, tameparse_language::t::nonterminal));
    report("MatchRegex1", test_lex("/regex/", tameparse_language::lexer, tameparse_language::t::regex));
    report("MatchString1", test_lex("\"string\"", tameparse_language::lexer, tameparse_language::t::string));
    report("MatchCharacter1", test_lex("'c'", tameparse_language::lexer, tameparse_language::t::character));
    report("MatchCharacter2", test_lex("'\\n'", tameparse_language::lexer, tameparse_language::t::character));
    
    report("MatchLanguage", test_lex("language", tameparse_language::lexer, tameparse_language::t::language));
    report("MatchGrammar", test_lex("grammar", tameparse_language::lexer, tameparse_language::t::grammar));
    report("MatchLexer", test_lex("lexer", tameparse_language::lexer, tameparse_language::t::lexer));
    report("MatchLexerSymbols", test_lex("lexer-symbols", tameparse_language::lexer, tameparse_language::t::lexer_symbols));
    report("MatchWeak", test_lex("weak", tameparse_language::lexer, tameparse_language::t::weak));
    report("MatchIgnore", test_lex("ignore", tameparse_language::lexer, tameparse_language::t::ignore));
    report("MatchKeywords", test_lex("keywords", tameparse_language::lexer, tameparse_language::t::keywords));
    report("MatchWhitespace", test_lex("  ", tameparse_language::lexer, tameparse_language::t::whitespace));
    report("MatchNewline", test_lex("\n", tameparse_language::lexer, tameparse_language::t::newline));
    report("MatchComment", test_lex("// Comment", tameparse_language::lexer, tameparse_language::t::comment));
}
