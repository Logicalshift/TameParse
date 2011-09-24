//
//  language_parser.cpp
//  TameParse
//
//  Created by Andrew Hunter on 19/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <sstream>

#include "TameParse/Util/stringreader.h"
#include "TameParse/Util/utf8reader.h"
#include "TameParse/Language/language_parser.h"
#include "TameParse/Language/process.h"

#include "tameparse_language.h"

using namespace std;
using namespace util;
using namespace dfa;
using namespace language;

/// \brief Creates a new language object
language_parser::language_parser()
: m_FileDefinition(NULL, true) {
}

typedef tameparse_language::Parser_Language                 Parser_Language;
typedef tameparse_language::list_of_TopLevel_Block          list_of_TopLevel_Block;
typedef tameparse_language::TopLevel_Block                  ast_TopLevel_Block;
typedef tameparse_language::Language_Block                  ast_Language_Block;
typedef tameparse_language::Language_Definition             ast_Language_Definition;
typedef tameparse_language::list_of_Language_Definition     list_of_Language_Definition;
typedef tameparse_language::list_of__comma__identifier      list_of__comma__identifier;
typedef tameparse_language::list_of_Lexeme_Definition       list_of_Lexeme_Definition;
typedef tameparse_language::list_of_Keyword_Definition      list_of_Keyword_Definition;
typedef tameparse_language::identifier                      identifier;
typedef tameparse_language::regex                           ast_regex;
typedef tameparse_language::string_2                        ast_string;
typedef tameparse_language::character                       ast_character;
typedef tameparse_language::list_of_Nonterminal_Definition  list_of_Nonterminal_Definition;
typedef tameparse_language::list_of__pipe__Production       list_of__pipe__Production;
typedef tameparse_language::list_of_Simple_Ebnf_Item        list_of_Simple_Ebnf_Item;
typedef tameparse_language::Nonterminal_Definition          ast_Nonterminal_Definition;
typedef tameparse_language::Production                      ast_Production;
typedef tameparse_language::Simple_Ebnf_Item                ast_Simple_Ebnf_Item;
typedef tameparse_language::Ebnf_Item                       ast_Ebnf_Item;

static ebnf_item* definition_for(const ast_Simple_Ebnf_Item* simpleItem);

/// \brief Converts a full EBNF item into an ebnf_item object
static ebnf_item* definition_for(const ast_Ebnf_Item* ebnfItem) {
    // Stick the items together
    bool        parenthesized   = false;            // True when parenthesized
    ebnf_item*  result          = NULL;
    
    for (list_of_Simple_Ebnf_Item::iterator item = ebnfItem->list_of_Simple_Ebnf_Item->begin(); 
         item != ebnfItem->list_of_Simple_Ebnf_Item->end(); 
         item++) {
        // Get the definition for this item
        ebnf_item* nextItem = definition_for((*item)->Simple_Ebnf_Item);
        
        if (!nextItem) {
            // Bug
            if (result) delete result;
            return NULL;
        }
        
        // Set as or add to the result
        if (!result) {
            result = nextItem;
        } else {
            if (!parenthesized) {
                // Add parentheses around the result
                ebnf_item* paren = new ebnf_item(ebnf_item::ebnf_parenthesized, ebnfItem->pos(), ebnfItem->final_pos());
                paren->add_child(result);
                result          = paren;
                parenthesized   = true;
            } else {
                // Add to the existing result
                result->add_child(nextItem);
            }
        }
    }
    
    // If the result is empty, replace with an empty parenthesized item
    if (!result) {
        result = new ebnf_item(ebnf_item::ebnf_parenthesized, ebnfItem->pos(), ebnfItem->final_pos());
    }
    
    // Create an alternate if one is supplied
    if (ebnfItem->Ebnf_Item_2) {
        // Get the alternative item
        ebnf_item* alternative = definition_for(ebnfItem->Ebnf_Item_2);
        
        if (alternative == NULL) {
            // Bug
            delete result;
            return NULL;
        }
        
        // Create the new result item
        ebnf_item* alternate = new ebnf_item(ebnf_item::ebnf_alternative, ebnfItem->pos(), ebnfItem->final_pos());
        
        alternate->add_child(result);
        alternate->add_child(alternative);
        
        result = alternate;
    }
    
    // Return the result
    return result;
}

/// \brief Converts a simple EBNF item into an ebnf_item object
static ebnf_item* definition_for(const ast_Simple_Ebnf_Item* simpleItem) {
    // Item depends on the content
    if (simpleItem->Nonterminal) {
        // Get the basic identifier
        wstring sourceIdentifier;
        wstring ntIdentifier = simpleItem->Nonterminal->nonterminal_2->content<wchar_t>();
        
        // Get the source identifier if it exists
        if (simpleItem->Nonterminal->identifier) {
            sourceIdentifier = simpleItem->Nonterminal->identifier->content<wchar_t>();
        }
        
        // Create the item
        return new ebnf_item(ebnf_item::ebnf_nonterminal, sourceIdentifier, ntIdentifier, simpleItem->pos(), simpleItem->final_pos());
    }
    
    else if (simpleItem->Terminal) {
        // Get the basic identifier
        ebnf_item::type terminalType = ebnf_item::ebnf_terminal;
        wstring         sourceIdentifier;
        wstring         termIdentifier;
        
        // Get the terminal data
        if (simpleItem->Terminal->Basic_Terminal->identifier) {
            terminalType    = ebnf_item::ebnf_terminal;
            termIdentifier  = simpleItem->Terminal->Basic_Terminal->identifier->content<wchar_t>();
        }
        
        else if (simpleItem->Terminal->Basic_Terminal->string_2) {
            terminalType    = ebnf_item::ebnf_terminal_string;
            termIdentifier  = simpleItem->Terminal->Basic_Terminal->string_2->content<wchar_t>();
        }
        
        else if (simpleItem->Terminal->Basic_Terminal->character) {
            terminalType    = ebnf_item::ebnf_terminal_character;
            termIdentifier  = simpleItem->Terminal->Basic_Terminal->character->content<wchar_t>();
        }
        
        else {
            // Unknown type of terminal
            return NULL;
        }
        
        // Get the source identifier if it exists
        if (simpleItem->Terminal->identifier) {
            sourceIdentifier = simpleItem->Terminal->identifier->content<wchar_t>();
        }
        
        // Create the item
        return new ebnf_item(terminalType, sourceIdentifier, termIdentifier, simpleItem->pos(), simpleItem->final_pos());        
    }
    
    else if (simpleItem->Guard) {
        // Get the internal item
        ebnf_item* internalItem = definition_for(simpleItem->Guard->Ebnf_Item);
        
        if (!internalItem) {
            // Bug
            return NULL;
        }
        
        // Turn into a guard item
        ebnf_item* guardItem = new ebnf_item(ebnf_item::ebnf_guard);
        guardItem->add_child(internalItem);
        
        return guardItem;
    }
    
    else if (simpleItem->_star_) {
        // Item of the form X*
        ebnf_item* starredItem = definition_for(simpleItem->Simple_Ebnf_Item_2);
        if (starredItem == NULL) {
            // Bug
            return NULL;
        }
        
        ebnf_item* result = new ebnf_item(ebnf_item::ebnf_repeat_zero, simpleItem->pos(), simpleItem->final_pos());
        result->add_child(starredItem);
        return result;
    }
    
    else if (simpleItem->_plus_) {
        // Item of the form X+
        ebnf_item* plusItem = definition_for(simpleItem->Simple_Ebnf_Item_2);
        if (plusItem == NULL) {
            // Bug
            return NULL;
        }
        
        ebnf_item* result = new ebnf_item(ebnf_item::ebnf_repeat_one, simpleItem->pos(), simpleItem->final_pos());
        result->add_child(plusItem);
        return result;
    }
    
    else if (simpleItem->_question_) {
        // Item of the form X?
        ebnf_item* optionalItem = definition_for(simpleItem->Simple_Ebnf_Item_2);
        if (optionalItem == NULL) {
            // Bug
            return NULL;
        }
        
        ebnf_item* result = new ebnf_item(ebnf_item::ebnf_optional, simpleItem->pos(), simpleItem->final_pos());
        result->add_child(optionalItem);
        return result;        
    }
    
    else if (simpleItem->_openparen_) {
        // Item of the form (X)
        return definition_for(simpleItem->Ebnf_Item);
    }
    
    // Unknown type of item
    return NULL;
}

/// \brief Processes a production definition
static production_definition* definition_for(const ast_Production* production) {
    // Begin a new production
    production_definition* defn = new production_definition(production->pos(), production->final_pos());
    
    // Iterate through the items in this production
    for (list_of_Simple_Ebnf_Item::iterator item = production->list_of_Simple_Ebnf_Item->begin();
         item != production->list_of_Simple_Ebnf_Item->end();
         item++) {
        // Get the next EBNF item
        ebnf_item* ebnf = definition_for((*item)->Simple_Ebnf_Item);
        
        if (ebnf == NULL) {
            // Bug
            delete defn;
            return NULL;
        }
        
        // Add to the production
        defn->add_item(ebnf);
    }
    
    // Return the result
    return defn;
}

/// \brief Processes a nonterminal definition
static nonterminal_definition* definition_for(const ast_Nonterminal_Definition* nonterminal) {
    // Get the nonterminal identifier (all types have this)
    wstring identifier = nonterminal->nonterminal_2->content<wchar_t>();
    
    // Work out the type
    nonterminal_definition::type ntType = nonterminal_definition::assignment;
    
    if (nonterminal->one_of__equals__or__plus__equals_) {
        if (nonterminal->one_of__equals__or__plus__equals_->_equals_) {
            ntType = nonterminal_definition::assignment;
        } else if (nonterminal->one_of__equals__or__plus__equals_->_plus__equals_) {
            ntType = nonterminal_definition::addition;
        }
    } else if (nonterminal->replace) {
        ntType = nonterminal_definition::replace;
    }
    
    // Start defining a new nonterminal
    nonterminal_definition* nonterm = new nonterminal_definition(ntType, identifier, nonterminal->pos(), nonterminal->final_pos());
    
    // Add the productions from this nonterminal
    production_definition* prod = definition_for(nonterminal->Production);
    
    if (prod == NULL) {
        // Doh, bug
        delete nonterm;
        return NULL;
    }
    
    nonterm->add_production(prod);
    
    for (list_of__pipe__Production::iterator nextProduction = nonterminal->list_of__pipe__Production->begin(); 
         nextProduction != nonterminal->list_of__pipe__Production->end(); 
         nextProduction++) {
        // Get the next production
        production_definition* prod = definition_for((*nextProduction)->Production);
        
        if (prod == NULL) {
            // Doh, bug
            delete nonterm;
            return NULL;
        }
        
        // Add to the nonterminal
        nonterm->add_production(prod);
    }
    
    // Return the result
    return nonterm;
}

/// \brief Turns a list of nonterminal definitions into a grammar
static language_unit* definition_for(const list_of_Nonterminal_Definition* items) {
    // Start creating the new grammar block
    grammar_block* gram = new grammar_block();
    
    // Iterate through the items
    for (list_of_Nonterminal_Definition::iterator nonterminal = items->begin(); nonterminal != items->end(); nonterminal++) {
        nonterminal_definition* defn = definition_for((*nonterminal)->Nonterminal_Definition);
        
        if (defn == NULL) {
            // Doh, bug!
            delete gram;
            return NULL;
        }
        
        gram->add_nonterminal(defn);
    }
    
    return new language_unit(gram);
}

/// \brief Interprets a keyword symbol definition block
static language_unit* definition_for(const list_of_Keyword_Definition* items, language_unit::unit_type type) {
    // Start building up the lexer block
    lexer_block* lexerBlock = new lexer_block(items->pos(), items->final_pos());
    
    // Iterate through the items
    for (list_of_Keyword_Definition::iterator keyword = items->begin(); keyword != items->end(); keyword++) {
        // Get the item for the new keyword
        const identifier* keywordId = (*keyword)->Keyword_Definition->identifier;
        
        // Deal with strings/characters/regexes
        if ((*keyword)->Keyword_Definition->one_of_regex_or_one_of_string_or_character) {
            // Get the three alternatives
            const ast_regex*        regex       = (*keyword)->Keyword_Definition->one_of_regex_or_one_of_string_or_character->regex;
            const ast_string*       string      = (*keyword)->Keyword_Definition->one_of_regex_or_one_of_string_or_character->string_2;
            const ast_character*    character   = (*keyword)->Keyword_Definition->one_of_regex_or_one_of_string_or_character->character;
            
            if (regex) {
                lexerBlock->add_definition(new lexeme_definition(lexeme_definition::regex, keywordId->content<wchar_t>(), regex->content<wchar_t>(), (*keyword)->pos(), (*keyword)->final_pos()));
            } else if (string) {
                lexerBlock->add_definition(new lexeme_definition(lexeme_definition::string, keywordId->content<wchar_t>(), string->content<wchar_t>(), (*keyword)->pos(), (*keyword)->final_pos()));
            } else if (character) {
                lexerBlock->add_definition(new lexeme_definition(lexeme_definition::character, keywordId->content<wchar_t>(), character->content<wchar_t>(), (*keyword)->pos(), (*keyword)->final_pos()));
            } else {
                // Doh, bug: fail
                delete lexerBlock;
                return NULL;
            }
        } else {
            // A literal keyword defined only by its identifier
            lexerBlock->add_definition(new lexeme_definition(lexeme_definition::literal, keywordId->content<wchar_t>(), keywordId->content<wchar_t>(), (*keyword)->pos(), (*keyword)->final_pos()));
        }
    }
    
    // Create the language unit
    return new language_unit(type, lexerBlock);
}

/// \brief Interprets a lexer symbol definition block
static language_unit* definition_for(const list_of_Lexeme_Definition* items, language_unit::unit_type type) {
    // Start building up the lexer block
    lexer_block* lexerBlock = new lexer_block(items->pos(), items->final_pos());
    
    // Iterate through the items
    for (list_of_Lexeme_Definition::iterator lexeme = items->begin(); lexeme != items->end(); lexeme++) {
        // Get the identifier for the new lexeme
        const identifier* lexemeId = (*lexeme)->Lexeme_Definition->identifier;
        
        // Lexeme should either be a string/character/regex or a reference
        if ((*lexeme)->Lexeme_Definition->one_of_regex_or_one_of_string_or_character) {
            // Get the three alternatives
            const ast_regex*        regex       = (*lexeme)->Lexeme_Definition->one_of_regex_or_one_of_string_or_character->regex;
            const ast_string*       string      = (*lexeme)->Lexeme_Definition->one_of_regex_or_one_of_string_or_character->string_2;
            const ast_character*    character   = (*lexeme)->Lexeme_Definition->one_of_regex_or_one_of_string_or_character->character;
            
            if (regex) {
                lexerBlock->add_definition(new lexeme_definition(lexeme_definition::regex, lexemeId->content<wchar_t>(), regex->content<wchar_t>(), (*lexeme)->pos(), (*lexeme)->final_pos()));
            } else if (string) {
                lexerBlock->add_definition(new lexeme_definition(lexeme_definition::string, lexemeId->content<wchar_t>(), string->content<wchar_t>(), (*lexeme)->pos(), (*lexeme)->final_pos()));
            } else if (character) {
                lexerBlock->add_definition(new lexeme_definition(lexeme_definition::character, lexemeId->content<wchar_t>(), character->content<wchar_t>(), (*lexeme)->pos(), (*lexeme)->final_pos()));
            } else {
                // Doh, bug: fail
                delete lexerBlock;
                return NULL;
            }
        } else if ((*lexeme)->Lexeme_Definition->identifier_2) {
            // Reference (IMPLEMENT ME)
            delete lexerBlock;
            return NULL;
        } else {
            // Doh, bug: fail
            delete lexerBlock;
            return NULL;
        }
    }
    
    return new language_unit(type, lexerBlock);
}

/// \brief Interprets a language unit
static language_unit* definition_for(const ast_Language_Definition* defn) {
    // Action depends on the typeof node in this AST node
    
    // Most of the lexer type nodes are very similar, except for the node type
    if (defn->Lexer_Symbols_Definition) {
        return definition_for(defn->Lexer_Symbols_Definition->list_of_Lexeme_Definition, language_unit::unit_lexer_symbols);
    } else if (defn->Lexer_Definition) {
        bool isWeak = defn->Lexer_Definition->optional_weak->weak;
        
        return definition_for(defn->Lexer_Definition->list_of_Lexeme_Definition, isWeak?language_unit::unit_weak_lexer_definition:language_unit::unit_lexer_definition);
    } else if (defn->Ignore_Definition) {
        return definition_for(defn->Ignore_Definition->list_of_Keyword_Definition, language_unit::unit_ignore_definition);
    } else if (defn->Keywords_Definition) {
        bool isWeak = defn->Keywords_Definition->optional_weak->weak;
        
        return definition_for(defn->Keywords_Definition->list_of_Keyword_Definition, isWeak?language_unit::unit_weak_keywords_definition:language_unit::unit_keywords_definition);
    } else if (defn->Grammar_Definition) {
        return definition_for(defn->Grammar_Definition->list_of_Nonterminal_Definition);
    }
    
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
        for (list_of__comma__identifier::iterator inherit = 
             language->optional_Language_Inherits->Language_Inherits->list_of__comma__identifier->begin();
             inherit != language->optional_Language_Inherits->Language_Inherits->list_of__comma__identifier->end();
             inherit++) {
            // Add each one to the result
            result->add_inherits((*inherit)->identifier->content<wchar_t>());
        }
    }
    
    // Add the language definitions
    for (list_of_Language_Definition::iterator langDefinition = language->list_of_Language_Definition->begin();
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
    
    return file;
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
        const tameparse_language::Parser_Language* root = static_cast<const tameparse_language::Parser_Language*>(parser_state->get_item().item());
        
        // Turn into a definition
        m_FileDefinition = definition_file_container(definition_for(root), true);
    }
    
    // Finished with the parser
    delete parser_state;
    delete stream;
    
    // Done
    return result;
}

/// \brief Parses the language file specified in the given string and stores it in this object.
///
/// This will return true if the file parsed correctly. If this is the case, then the file_definition() function
/// will return the result. If there is any existing definition, this will be replaced by this call.
bool language_parser::parse(const std::string& language) {
    bool result = false;
    
    // Convert to a string stream
    stringstream utf8stream(language);
    
    // Create a UTF-8 reader
    utf8reader languageReader(&utf8stream);
    
    // Clear the definition
    m_FileDefinition = definition_file_container(NULL, true);
    
    // Create the parser for this language
    typedef tameparse_language::ast_parser_type::state  state;
    typedef tameparse_language::parser_actions          parser_actions;
    
    // Create a lexer for this string
    lexeme_stream* stream = tameparse_language::lexer.create_stream_from<wchar_t>(languageReader);
    
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
        const tameparse_language::Parser_Language* root = static_cast<const tameparse_language::Parser_Language*>(parser_state->get_item().item());
        
        // Turn into a definition
        m_FileDefinition = definition_file_container(definition_for(root), true);
    }
    
    // Finished with the parser
    delete parser_state;
    delete stream;
    
    // Done
    return result;
}

/// \brief Parses the language file specified in the given string and stores it in this object.
///
/// This will return true if the file parsed correctly. If this is the case, then the file_definition() function
/// will return the result. If there is any existing definition, this will be replaced by this call.
bool language_parser::parse(std::istream& language) {
    bool result = false;
    
    // Create a UTF-8 reader
    utf8reader languageReader(&language);
    
    // Clear the definition
    m_FileDefinition = definition_file_container(NULL, true);
    
    // Create the parser for this language
    typedef tameparse_language::ast_parser_type::state  state;
    typedef tameparse_language::parser_actions          parser_actions;
    
    // Create a lexer for this string
    lexeme_stream* stream = tameparse_language::lexer.create_stream_from<wchar_t>(languageReader);
    
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
        const tameparse_language::Parser_Language* root = static_cast<const tameparse_language::Parser_Language*>(parser_state->get_item().item());
        
        // Turn into a definition
        m_FileDefinition = definition_file_container(definition_for(root), true);
    }
    
    // Finished with the parser
    delete parser_state;
    delete stream;
    
    // Done
    return result;
}
