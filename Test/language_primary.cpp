//
//  language_primary.cpp
//  TameParse
//
//  Created by Andrew Hunter on 10/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <string>
#include <sstream>

#include "TameParse/Util/utf8reader.h"
#include "TameParse/Language/bootstrap.h"
#include "TameParse/Language/language_parser.h"

#include "language_primary.h"
#include "tameparse_language.h"

using namespace std;
using namespace util;
using namespace dfa;
using namespace lr;
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
        for (size_t x=0; x<match->content().size(); ++x) {
            cerr << (char)match->content()[x];
        }
        cerr << endl;
        result = false;
    }
    if (match->matched() != expectedSymbol) {
        cerr << match->matched() << endl;
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
    report("MatchRegex2", test_lex("/\\//", tameparse_language::lexer, tameparse_language::t::regex));
    report("MatchString1", test_lex("\"string\"", tameparse_language::lexer, tameparse_language::t::string));
    report("MatchString2", test_lex("\"str\\\"ing\"", tameparse_language::lexer, tameparse_language::t::string));
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
    report("MatchComment1", test_lex("// Comment", tameparse_language::lexer, tameparse_language::t::comment));
    report("MatchComment2", test_lex("///", tameparse_language::lexer, tameparse_language::t::comment));
    report("MatchComment3", test_lex("//", tameparse_language::lexer, tameparse_language::t::comment));

    // Create a lexer for the language definition
    bootstrap bs;
    stringstream bootstrapDefinition(bootstrap::get_default_language_definition());
    utf8reader bootstreapReader(&bootstrapDefinition);

    lexeme_stream* defaultStream = tameparse_language::lexer.create_stream_from<wchar_t>(bootstreapReader);
    
    // Create a parser for it
    typedef tameparse_language::ast_parser_type ast_parser;
    typedef tameparse_language::parser_actions parser_actions;
    
#if 0
    typedef parser<tameparse_language::syntax_node_container, parser_actions, debug_parser_trace<2> > debug_parser;
    
    debug_parser parser(tameparse_language::lr_tables);
    debug_parser::state* defParser = parser.create_parser(new parser_actions(defaultStream));
#elif 0
    simple_parser::state* defParser = tameparse_language::simple_parser.create_parser(new simple_parser_actions(defaultStream));
#else
    ast_parser::state* defParser = tameparse_language::ast_parser.create_parser(new parser_actions(defaultStream));
#endif
    
    // Try parsing the language
    bool acceptedDefault = defParser->parse();
    
    report("CanParseLanguageDefinition1", acceptedDefault);
    
    language_parser lp;
    report("CanParseLanguageDefinition2", lp.parse(bootstrap::get_default_language_definition()));
    report("CanGetDefinition", lp.file_definition().item() != NULL);
}
