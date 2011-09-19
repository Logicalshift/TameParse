//
//  language_parser.cpp
//  TameParse
//
//  Created by Andrew Hunter on 19/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "util/stringreader.h"
#include "Language/language_parser.h"

#include "tameparse_language.h"

using namespace util;
using namespace dfa;
using namespace language;

/// \brief Creates a new language object
language_parser::language_parser()
: m_FileDefinition(NULL, true) {
}

typedef tameparse_language::epsilon epsilon;
typedef tameparse_language::Parser_Language Parser_Language;

/// \brief Turns a parser language object into a definition file object
static definition_file* definition_for(const Parser_Language* language) {
    // TODO
    return NULL;
}

/// \brief Turns a base definition into a definition file object
static definition_file* definition_for(const tameparse_language::epsilon* root) {
    // Pass the parser language straight through
    return definition_for(root->Parser_Language);
}

/// \brief Parses the language file specified in the given string and stores it in this object.
///
/// This will return true if the file parsed correctly. If this is the case, then the file_definition() function
/// will return the result. If there is any existing definition, this will be replaced by this call.
bool language_parser::parse(const std::wstring& language) {
    bool result = false;
    
    // Clear the definition
    m_FileDefinition = definition_file_container(NULL, true);
    
    // Create the parser for this language
    typedef tameparse_language::ast_parser_type::state  state;
    typedef tameparse_language::parser_actions          parser_actions;
    
    // Create a lexer for this string
    wstringreader reader(language);
    
    lexeme_stream* stream = tameparse_language::lexer.create_stream_from<wchar_t>(reader);
    
    // Create the parser
    // Currently using the 'raw' parser here (due to the state of the C++ generator at this point in time: I imagine it will have
    // a few more interesting/easy ways of creating parsers later on)
    state* parser_state = tameparse_language::ast_parser.create_parser(new parser_actions(stream));
    
    // Parse the language
    result = parser_state->parse();
    
    // Convert to a definition
    if (result) {
        // Fetch the root item (which will be an epsilon item at the moment due to the way the parser is built up)
        // The name of this item will probably change to something more sensible at some point (and I'll forget to remove this comment)
        const tameparse_language::epsilon* root = (const tameparse_language::epsilon*) parser_state->get_item().item();
        
        // Turn into a definition
        m_FileDefinition = definition_file_container(definition_for(root), true);
    }
    
    // Finished with the parser
    delete parser_state;
    delete stream;
    
    // Done
    return result;
}
