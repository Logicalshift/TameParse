//
//  main.cpp
//  bootstrap
//
//  Created by Andrew Hunter on 21/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <sstream>

#include "Language/bootstrap.h"
#include "Compiler/console.h"
#include "Compiler/std_console.h"
#include "Compiler/language_compiler.h"
#include "Compiler/lexer_compiler.h"

using namespace std;
using namespace dfa;
using namespace lr;
using namespace language;
using namespace compiler;

int main (int argc, const char * argv[])
{
    wcout << L"Parser bootstrapper" << endl;
    wcout << L"===================" << endl << endl;
    
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
    language_compiler languageStage(consContainer, L"definition.txt", lBlock);
    languageStage.compile();
    
    // Stage 2: compile the lexer
    lexer_compiler lexerStage(consContainer, L"definition.txt", &languageStage);
    lexerStage.compile();
    
    // Done
    return cons.exit_code();
}

