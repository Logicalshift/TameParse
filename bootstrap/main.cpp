//
//  main.cpp
//  bootstrap
//
//  Created by Andrew Hunter on 21/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <sstream>

#include "TameParse/Util/utf8reader.h"
#include "TameParse/Language/bootstrap.h"
#include "TameParse/Language/formatter.h"
#include "TameParse/Compiler/console.h"
#include "TameParse/Compiler/std_console.h"
#include "TameParse/Compiler/language_stage.h"
#include "TameParse/Compiler/lexer_stage.h"
#include "TameParse/Compiler/lr_parser_stage.h"
#include "TameParse/Compiler/OutputStages/cplusplus.h"

using namespace std;
using namespace util;
using namespace dfa;
using namespace lr;
using namespace language;
using namespace compiler;

#ifdef DEBUG

#include "dfa_range.h"
#include "dfa_symbol_set.h"
#include "dfa_symbol_deduplicate.h"
#include "dfa_symbol_translator.h"
#include "dfa_ndfa.h"
#include "dfa_single_regex.h"
#include "contextfree_firstset.h"
#include "contextfree_followset.h"
#include "lr_weaksymbols.h"
#include "lr_lalr_general.h"
#include "dfa_multi_regex.h"
#include "language_bootstrap.h"

static void run(test_fixture& fixture) {
    fixture.run();
    
    //s_Run += fixture.count_run();
    //s_Failed += fixture.count_failed();
}

static void run_tests() {
    test_dfa_range              dfa_range;      run(dfa_range);
    test_dfa_symbol_set         symbol_set;     run(symbol_set);
    test_dfa_symbol_deduplicate dedupe;         run(dedupe);
    test_dfa_ndfa               ndfa;           run(ndfa);
    test_dfa_symbol_translator  trans;          run(trans);
    test_dfa_single_regex       singleregex;    run(singleregex);
    test_dfa_multi_regex        multiregex;     run(multiregex);
    
    test_contextfree_firstset   firstset;       run(firstset);
    test_contextfree_followset  followset;      run(followset);
    
    test_lr_weaksymbols         weakSymbols;    run(weakSymbols);
    test_lalr_general           lalr1;          run(lalr1);

    test_language_bootstrap     boostrap;       run(boostrap);
}

#endif

int main (int argc, const char * argv[]) {
    // Run some tests in debug builds
#ifdef DEBUG
    run_tests();
#endif

    wcout << L"TameParse bootstrapper" << endl;
    wcout << L"======================" << endl << endl;
    
    // Create the console object
    // We do the parsing stages manually so there's not much actual use for a filename
    std_console         cons(L"definition.tp");
    console_container   consContainer(&cons, false);
    
    // Create the bootstrap language
    bootstrap bs;
    
    // Parse the actual parser language definition using the bootstrap language
    wcout << L"  = Parsing language definition" << endl;
    
    stringstream        parserLanguageDefinition(bootstrap::get_default_language_definition());
    utf8reader          parserReader(&parserLanguageDefinition);
    lexeme_stream*      languageStream  = bs.get_lexer().create_stream_from<wchar_t>(parserReader);
    ast_parser::state*  languageParser  = bs.get_parser().create_parser(new ast_parser_actions(languageStream));
    
    bool acceptedLanguage = languageParser->parse();
    if (!acceptedLanguage) {
        // Doh, fail
        wcerr << formatter::to_string(bs.get_builder(), bs.get_grammar(), bs.get_terminals()) << endl;
        wcerr << L"Parser error!" << endl;
        return 1;
    }
    
    // Get the definition for the language
    definition_file_container defn = bs.get_definition(languageParser->get_item().item());
    if (defn.item() == NULL) {
        wcerr << L"Could not retrieve language definition" << endl;
        return 1;
    }
    
    // Run the parser stages to generate the output file
    const language_block* lBlock = NULL;
    for (definition_file::iterator mightBeLanguage = defn->begin(); mightBeLanguage != defn->end(); mightBeLanguage++) {
        if ((*mightBeLanguage)->language()) {
            lBlock = (*mightBeLanguage)->language();
        }
    }
    
    if (lBlock == NULL) {
        wcerr << L"Could not retrieve language definition block" << endl;
        return 1;
    }
    
    // Stage 1: interpret the language and generate the NDFA and grammar
    language_stage languageStage(consContainer, L"definition.tp", lBlock, NULL);
    languageStage.compile();
    languageStage.report_unused_symbols();
    
    // Stage 2: compile the lexer
    lexer_stage lexerStage(consContainer, L"definition.tp", &languageStage);
    lexerStage.compile();
    
    // Stage 3: compile the LR parser
    vector<wstring> startSymbols;
    startSymbols.push_back(L"<Parser-Language>");

    lr_parser_stage lrStage(consContainer, L"definition.tp", &languageStage, &lexerStage, startSymbols);
    lrStage.compile();
    
    // Stage 4: build as a C++ parser
    output_cplusplus cPlusPlus(consContainer, L"definition.tp", &lexerStage, &languageStage, &lrStage, L"tameparse_language", L"tameparse_language", L"language"); 
    
    cPlusPlus.compile();
    
    // Done
    return cons.exit_code();
}

