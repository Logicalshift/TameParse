//
//  language_parser.cpp
//  TameParse
//
//  Created by Andrew Hunter on 19/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "util/stringreader.h"
#include "Language/language_parser.h"
#include "Language/process.h"

#include "tameparse_language.h"

using namespace util;
using namespace dfa;
using namespace language;

/// \brief Creates a new language object
language_parser::language_parser()
: m_FileDefinition(NULL, true) {
}

typedef tameparse_language::Parser_Language         Parser_Language;
typedef tameparse_language::list_of_TopLevel_Block  list_of_TopLevel_Block;
typedef tameparse_language::TopLevel_Block          ast_TopLevel_Block;
typedef tameparse_language::Language_Block          ast_Language_Block;
typedef tameparse_language::Language_Definition     ast_Language_Definition;

/// \brief Interprets a language unit
static language_unit* definition_for(const ast_Language_Definition* defn) {
    // TODO
    return NULL;
}

/// \brief Interprets a language block
static language_block* definition_for(const ast_Language_Block* language) {
    // Create the language block
    language_block* result = new language_block(language->identifier->content<wchar_t>(), language->pos(), language->final_pos());
    
    // Deal with the inherits block, if it exists
    if (language->optional_Language_Inherits->Language_Inherits) {
        // Add the first identifier
        result->add_inherits(language->optional_Language_Inherits->Language_Inherits->identifier->content<wchar_t>());
        
        // Iterate through the identifiers of the languages this inherits from
        for (tameparse_language::list_of__comma__identifier::iterator inherit = 
             language->optional_Language_Inherits->Language_Inherits->list_of__comma__identifier->begin();
             inherit != language->optional_Language_Inherits->Language_Inherits->list_of__comma__identifier->end();
             inherit++) {
            // Add each one to the result
            result->add_inherits((*inherit)->identifier->content<wchar_t>());
        }
    }
    
    // Add the language definitions
    for (tameparse_language::list_of_Language_Definition::iterator langDefinition = language->list_of_Language_Definition->begin();
         langDefinition != language->list_of_Language_Definition->end();
         langDefinition++) {
        // Get the next definition
        language_unit* nextUnit = definition_for((*langDefinition)->Language_Definition);
        
        // Self-destruct if we get a failure here
        if (!nextUnit) {
            // Doh, bug
            delete result;
            return NULL;
        }
        
        // Add to the definition
        result->add_unit(nextUnit);
    }
    
    // Return the result
    return result;
}

/// \brief Interprets a top-level block
static toplevel_block* definition_for(const ast_TopLevel_Block* toplevel) {
    // Action depends on the type of block
    
    // Language block
    if (toplevel->Language_Block) {
        // Get the definition for this language block
        language_block* language = definition_for(toplevel->Language_Block);
        if (!language) {
            // Doh
            return NULL;
        }
        
        // Turn into a toplevel block
        return new toplevel_block(language);
    }
    
    // Import block
    else if (toplevel->Import_Block) {
        // Fairly simple to convert
        return new toplevel_block(new import_block(process::dequote_string(toplevel->Import_Block->string_2->content<wchar_t>()), toplevel->pos(), toplevel->final_pos()));
    }
    
    // Parser block
    else if (toplevel->Parser_Block) {
        
    }
    
    // Failed to parse: doh, bug
    return NULL;
}

/// \brief Turns a parser language object into a definition file object
static definition_file* definition_for(const Parser_Language* language) {
    // Create a new definition file
    definition_file* file = new definition_file();
    
    // Iterate through the top-level definitions
    for (list_of_TopLevel_Block::iterator topLevel = language->list_of_TopLevel_Block->begin(); topLevel != language->list_of_TopLevel_Block->end(); topLevel++) {
        // Get the definition for this toplevel block
        toplevel_block* newBlock = definition_for((*topLevel)->TopLevel_Block);
        
        // Blow up if the block turns out to be NULL
        if (!newBlock) {
            // Doh, bug
            delete file;
            return NULL;
        }
        
        // Add to the file
        file->add(newBlock);
    }
    
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
