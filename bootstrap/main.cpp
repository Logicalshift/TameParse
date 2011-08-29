//
//  main.cpp
//  bootstrap
//
//  Created by Andrew Hunter on 21/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <unistd.h>
#include <sstream>

#include "Language/bootstrap.h"
#include "Compiler/console.h"
#include "Compiler/std_console.h"
#include "Compiler/language_stage.h"
#include "Compiler/lexer_stage.h"
#include "Compiler/lr_parser_stage.h"
#include "Compiler/OutputStages/cplusplus.h"

using namespace std;
using namespace dfa;
using namespace lr;
using namespace language;
using namespace compiler;

int main (int argc, const char * argv[]) {
    char buf[512];
    getcwd(buf, 512);
    cout << buf << endl;
    wcout << L"TameParse bootstrapper" << endl;
    wcout << L"======================" << endl << endl;
    
    // Create the console object
    // We do the parsing stages manually so there's not much actual use for a filename
    std_console         cons(L"definition.txt");
    console_container   consContainer(&cons, false);
    
    // Create the bootstrap language
    bootstrap bs;
    
    // Parse the actual parser language definition using the bootstrap language
    wcout << L"  = Parsing language definition" << endl;
    
    stringstream        parserLanguageDefinition(bootstrap::get_default_language_definition());
    lexeme_stream*      languageStream  = bs.get_lexer().create_stream_from(parserLanguageDefinition);
    ast_parser::state*  languageParser  = bs.get_parser().create_parser(new ast_parser_actions(languageStream));
    
    bool acceptedLanguage = languageParser->parse();
    if (!acceptedLanguage) {
        // Doh, fail
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
    language_stage languageStage(consContainer, L"definition.txt", lBlock);
    languageStage.compile();
    
    // Stage 2: compile the lexer
    lexer_stage lexerStage(consContainer, L"definition.txt", &languageStage);
    lexerStage.compile();
    
    // Stage 3: compile the LR parser
    vector<wstring> startSymbols;
    startSymbols.push_back(L"<Parser-Language>");

    lr_parser_stage lrStage(consContainer, L"definition.txt", &languageStage, &lexerStage, startSymbols);
    lrStage.compile();
    
    // Stage 4: build as a C++ parser
    output_cplusplus cPlusPlus(consContainer, L"definition.txt", &lexerStage, &languageStage, &lrStage, L"tameparse_language", L"tameparse_language", L"language"); 
    
    cPlusPlus.compile();
    
    // Done
    return cons.exit_code();
}

