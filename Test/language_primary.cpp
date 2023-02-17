//
//  language_primary.cpp
//  TameParse
//
//  Created by Andrew Hunter on 10/09/2011.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the \"Software\"), to 
//  deal in the Software without restriction, including without limitation the 
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
//  sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
//  IN THE SOFTWARE.
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
using namespace yy_language;

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

// Checks that a given phrase is lexed as an ignored symbol
static bool test_ignored(string phrase, const lexer& lex, const set<int>& ignoredSymbols) {
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
    if (ignoredSymbols.find(match->matched()) == ignoredSymbols.end()) {
        cerr << match->matched() << endl;
        result = false;
    }
    
    // Finished with the stream
    delete match;
    delete lxs;
    
    return result;
}

void test_language_primary::run_tests() {
    // Build the list of ignored symbols
    set<int> ignoredSymbols;
    
    // insert takes a reference which tries to create a link to constant value, which fails because they aren't defined anywhere,
    // so we do manually what the compiler should be smart enough to do automatically.
    int ws      = yy_tameparse_language::t::yy_whitespace;
    int nl      = yy_tameparse_language::t::yy_newline;
    int com     = yy_tameparse_language::t::yy_comment;
    int ccom    = yy_tameparse_language::t::yy_c_comment;
    ignoredSymbols.insert(ws);
    ignoredSymbols.insert(nl);
    ignoredSymbols.insert(com);
    ignoredSymbols.insert(ccom);
    
    // Test that the lexer can match some particular symbols
    report("MatchIdentifier1", test_lex("fdsu", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_identifier));
    report("MatchIdentifier2", test_lex("some-identifier", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_identifier));
    report("MatchIdentifier3", test_lex("id128", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_identifier));
    report("MatchIdentifier4", test_lex("identifier", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_identifier));
    report("MatchNonterminal1", test_lex("<nonterminal>", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_nonterminal));
    report("MatchRegex1", test_lex("/regex/", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_regex));
    report("MatchRegex2", test_lex("/\\//", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_regex));
    report("MatchString1", test_lex("\"string\"", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_string));
    report("MatchString2", test_lex("\"str\\\"ing\"", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_string));
    report("MatchCharacter1", test_lex("'c'", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_character));
    report("MatchCharacter2", test_lex("'\\n'", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_character));
    
    report("MatchLanguage", test_lex("language", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_language));
    report("MatchGrammar", test_lex("grammar", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_grammar));
    report("MatchLexer", test_lex("lexer", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_lexer));
    report("MatchLexerSymbols", test_lex("lexer-symbols", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_lexer_symbols));
    report("MatchWeak", test_lex("weak", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_weak));
    report("MatchIgnore", test_lex("ignore", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_ignore));
    report("MatchKeywords", test_lex("keywords", yy_tameparse_language::lexer, yy_tameparse_language::t::yy_keywords));
    report("MatchWhitespace", test_ignored("  ", yy_tameparse_language::lexer, ignoredSymbols));
    report("MatchNewline", test_ignored("\n", yy_tameparse_language::lexer, ignoredSymbols));
    report("MatchComment1", test_ignored("// Comment", yy_tameparse_language::lexer, ignoredSymbols));
    report("MatchComment2", test_ignored("///", yy_tameparse_language::lexer, ignoredSymbols));
    report("MatchComment3", test_ignored("//", yy_tameparse_language::lexer, ignoredSymbols));
    report("MatchComment4", test_ignored("/* C comment */", yy_tameparse_language::lexer, ignoredSymbols));

    // Create a lexer for the language definition
    bootstrap bs;
    stringstream bootstrapDefinition(bootstrap::get_default_language_definition());
    utf8reader bootstreapReader(&bootstrapDefinition);

    lexeme_stream* defaultStream = yy_tameparse_language::lexer.create_stream_from<wchar_t>(bootstreapReader);
    
    // Create a parser for it
    typedef yy_tameparse_language::ast_parser_type ast_parser;
    typedef yy_tameparse_language::parser_actions parser_actions;
    
#if 0
    typedef parser<yy_tameparse_language::_syntax_node_container, parser_actions, debug_parser_trace<2> > debug_parser;
    
    debug_parser parser(yy_tameparse_language::_lr_tables);
    debug_parser::state* defParser = parser.create_parser(new parser_actions(defaultStream));
#elif 0
    simple_parser::state* defParser = yy_tameparse_language::_simple_parser.create_parser(new simple_parser_actions(defaultStream));
#else
    ast_parser::state* defParser = yy_tameparse_language::ast_parser.create_parser(new parser_actions(defaultStream));
#endif
    
    // Try parsing the language
    bool acceptedDefault = defParser->parse();
    
    report("CanParseLanguageDefinition1", acceptedDefault);
    
    language_parser lp;
    report("CanParseLanguageDefinition2", lp.parse(bootstrap::get_default_language_definition()));
    report("CanGetDefinition", lp.file_definition().item() != NULL);
}
