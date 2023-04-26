//
//  language_parser.cpp
//  TameParse
//
//  Created by Andrew Hunter on 19/09/2011.
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

#include <sstream>

#include "TameParse/Util/stringreader.h"
#include "TameParse/Util/utf8reader.h"
#include "TameParse/Language/language_parser.h"
#include "TameParse/Language/process.h"

#include "tameparse_language.h"

using namespace std;
using namespace util;
using namespace dfa;
using namespace compiler;
using namespace yy_language;

/// \brief Creates a new language object
language_parser::language_parser()
: m_FileDefinition(NULL, true) {
}

typedef yy_tameparse_language::yy_Parser_Language_n                   Parser_Language;
typedef yy_tameparse_language::yy_list_of_TopLevel_Block_n            list_of_TopLevel_Block;
typedef yy_tameparse_language::yy_TopLevel_Block_n                    ast_TopLevel_Block;
typedef yy_tameparse_language::yy_Language_Block_n                    ast_Language_Block;
typedef yy_tameparse_language::yy_Language_Definition_n               ast_Language_Definition;
typedef yy_tameparse_language::yy_list_of_Language_Definition_n       list_of_Language_Definition;
typedef yy_tameparse_language::yy_Lexeme_Definition_n                 ast_Lexeme_Definition;
typedef yy_tameparse_language::yy_list_of_Lexeme_Definition_n         list_of_Lexeme_Definition;
typedef yy_tameparse_language::yy_list_of_Keyword_Definition_n        list_of_Keyword_Definition;
typedef yy_tameparse_language::yy_identifier_n                        identifier;
typedef yy_tameparse_language::yy_regex_n                             ast_regex;
typedef yy_tameparse_language::yy_string_n                            ast_string;
typedef yy_tameparse_language::yy_character_n                         ast_character;
typedef yy_tameparse_language::yy_list_of_Nonterminal_Definition_n    list_of_Nonterminal_Definition;
typedef yy_tameparse_language::yy_list_of__pipe__Production_n         list_of__pipe__Production;
typedef yy_tameparse_language::yy_list_of_Simple_Ebnf_Item_n          list_of_Simple_Ebnf_Item;
typedef yy_tameparse_language::yy_Nonterminal_Definition_n            ast_Nonterminal_Definition;
typedef yy_tameparse_language::yy_Production_n                        ast_Production;
typedef yy_tameparse_language::yy_Simple_Ebnf_Item_n                  ast_Simple_Ebnf_Item;
typedef yy_tameparse_language::yy_Ebnf_Item_n                         ast_Ebnf_Item;
typedef yy_tameparse_language::yy_Semantic_Specification_n            ast_Semantic_Specification;
typedef yy_tameparse_language::yy_list_of__comma__Semantic_Item_n     list_of_Semantic_Item;
typedef yy_tameparse_language::yy_Semantic_Item_n                     ast_Semantic_Item;
typedef yy_tameparse_language::yy_Test_Block_n                        ast_Test_Block;
typedef yy_tameparse_language::yy_list_of_Test_Definition_n           ast_list_of_Test_Definition;
typedef yy_tameparse_language::yy_Test_Definition_n                   ast_Test_Definition;
typedef yy_tameparse_language::yy_list_of_Test_Specification_n        ast_list_of_Test_Specification;
typedef yy_tameparse_language::yy_Parser_Block_n                      ast_Parser_Block;
typedef yy_tameparse_language::yy_list_of_Parser_StartSymbol_n        ast_list_of_Parser_StartSymbol;
typedef yy_tameparse_language::yy_list_of_Lexer_Modifier_n            list_of_Lexer_Modifier;
typedef yy_tameparse_language::yy_list_of_Lexer_Symbols_Modifier_n    list_of_Lexer_Symbols_Modifier;
typedef yy_tameparse_language::yy_Precedence_Definition_n             ast_Precedence_Definition;
typedef yy_tameparse_language::yy_list_of_Precedence_Item_n           list_of_Precedence_Item;
typedef yy_tameparse_language::yy_Equal_Precedence_Items_n            ast_Equal_Precedence_Items;

/// \brief Adds a test definition to the test block
static bool add_test_definition(test_block* target, const ast_Test_Definition* defn) {
    // Sanity check
    if (!defn->yy_Nonterminal) return false;

    // Fetch the name of the nonterminal and the language that this definition is in
    wstring nonterminalLanguage;
    wstring nonterminalName;

    if (defn->yy_Nonterminal->yy_source_language) {
        nonterminalLanguage = defn->yy_Nonterminal->yy_source_language->content<wchar_t>();
    }

    nonterminalName = defn->yy_Nonterminal->yy_nonterminal->content<wchar_t>();

    // Work out the type of this definition
    test_definition::test_type type = test_definition::match;

    if (defn->yy__equals_) {
        type = test_definition::match;
    } else if (defn->yy__exclamation__equals_) {
        type = test_definition::no_match;
    } else if (defn->yy_from) {
        type = test_definition::match_from_file;
    } else {
        // Unknown definition type
        return false;
    }

    // Sanity check: there must be at least one test specification for this block to be valid
    size_t defn_count = defn->yy_list_of_Test_Specification->size();
    if (defn_count == 0) {
        return false;
    }
    
    // Iterate through the test specifications and generate new test definitions
    for (ast_list_of_Test_Specification::iterator spec = defn->yy_list_of_Test_Specification->begin();
         spec != defn->yy_list_of_Test_Specification->end(); 
         ++spec) {
        // Get the identifier for this test (if it has one)
        wstring identifier;

        if ((*spec)->yy_Test_Specification->yy_identifier) {
            identifier = (*spec)->yy_Test_Specification->yy_identifier->content<wchar_t>();
        }

        // Now the string value of the test
        wstring     testString      = process::dequote_string((*spec)->yy_Test_Specification->yy_string->content<wchar_t>());
        position    testStringPos   = (*spec)->yy_Test_Specification->yy_string->pos();

        testStringPos.increment();

        // Generate the test definition
        test_definition* newDefn = new test_definition(nonterminalLanguage, nonterminalName, type, identifier, testString, (*spec)->yy_Test_Specification->pos(), (*spec)->yy_Test_Specification->final_pos(), testStringPos);

        // Add to the target
        target->add_test_definition(newDefn);
    }

    // Win
    return true;
}

/// \brief Converts a parser block into a parser_block object
static parser_block* definition_for(const ast_Parser_Block* parserBlock) {
    // Sanity check
    if (!parserBlock->yy_name)             return NULL;
    if (!parserBlock->yy_language_name)    return NULL;

    // Get the list of start symbols
    vector<wstring> startSymbols;
    for (ast_list_of_Parser_StartSymbol::iterator startSymbol = parserBlock->yy_start_symbols->begin(); startSymbol != parserBlock->yy_start_symbols->end(); ++startSymbol) {
        startSymbols.push_back((*startSymbol)->yy_Parser_StartSymbol->yy_Nonterminal->yy_nonterminal->content<wchar_t>());
    }

    // Create the parser block
    return new parser_block(parserBlock->yy_name->content<wchar_t>(), parserBlock->yy_language_name->content<wchar_t>(), startSymbols, parserBlock->pos(), parserBlock->final_pos());
}

/// \brief Converts a test block into a test_block
static test_block* definition_for(const ast_Test_Block* testBlock) {
    // Sanity check
    if (!testBlock->yy_language_name)  return NULL;
    if (!testBlock->yy_tests)          return NULL;

    // Get the identifier for this block
    wstring languageName = testBlock->yy_language_name->content<wchar_t>();

    // Create the result
    test_block* result = new test_block(languageName, testBlock->pos(), testBlock->final_pos());

    // Iterate through the test definitions
    for (ast_list_of_Test_Definition::iterator defn = testBlock->yy_tests->begin();
         defn != testBlock->yy_tests->end();
         ++defn) {
         // Add this definition to this block
        if (!add_test_definition(result, (*defn)->yy_Test_Definition)) {
            // Bug: couldn't get this test definition
            delete result;
            return NULL;
        }
    }

    // Got the result
    return result;
}

/// \brief Modifies the attributes with the specifed semantic specification
static void modify_attribute(ebnf_item_attributes& attr, const ast_Semantic_Item* item) {
    // Action depends on the content of attr
    if (item->yy_name) {
        // Name of this item when it is represented in the AST

        // TODO: error if a name is used twice
        attr.name = item->yy_name->content<wchar_t>();
    } else if (item->yy_conflict) {
        // Action to perform when this item is in a shift/reduce conflict

        // TODO: error if more than one conflict action is specified
        if (item->yy_shift) {
            attr.conflict_action = ebnf_item_attributes::conflict_shift;
        } else if (item->yy_reduce && item->yy_weak) {
            attr.conflict_action = ebnf_item_attributes::conflict_weakreduce;
        } else if (item->yy_reduce) {
            attr.conflict_action = ebnf_item_attributes::conflict_reduce;
        }
    }
}

/// \brief Converts a semantic specification into item attributes
static ebnf_item_attributes definition_for(const ast_Semantic_Specification* spec) {
    // Start with the default attributes
    ebnf_item_attributes result;

    // Process each attribute in turn
    modify_attribute(result, spec->yy_first_item);

    for (list_of_Semantic_Item::iterator nextItem = spec->yy_more_items->begin(); nextItem != spec->yy_more_items->end(); ++nextItem) {
        modify_attribute(result, (*nextItem)->yy_Semantic_Item);
    }

    // Finished
    return result;
}

/// \brief Definition for a simple EBNF item
static ebnf_item* definition_for(const ast_Simple_Ebnf_Item* simpleItem);

/// \brief Converts a full EBNF item into an ebnf_item object
static ebnf_item* definition_for(const ast_Ebnf_Item* ebnfItem, const ebnf_item_attributes& attr) {
    // Stick the items together
    bool        parenthesized   = false;            // True when parenthesized
    ebnf_item*  result          = NULL;
    
    for (list_of_Simple_Ebnf_Item::iterator item = ebnfItem->yy_items->begin(); 
         item != ebnfItem->yy_items->end(); 
         ++item) {
        // Get the definition for this item
        ebnf_item* nextItem = definition_for((*item)->yy_Simple_Ebnf_Item);
        
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
                ebnf_item* paren = new ebnf_item(ebnf_item::ebnf_parenthesized, ebnf_item_attributes(), ebnfItem->pos(), ebnfItem->final_pos());
                paren->add_child(result);
                paren->add_child(nextItem);
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
        result = new ebnf_item(ebnf_item::ebnf_parenthesized, ebnf_item_attributes(), ebnfItem->pos(), ebnfItem->final_pos());
    }
    
    // Create an alternate if one is supplied
    if (ebnfItem->yy_or_item) {
        // Get the alternative item
        ebnf_item* alternative = definition_for(ebnfItem->yy_or_item, ebnf_item_attributes());
        
        if (alternative == NULL) {
            // Bug
            delete result;
            return NULL;
        }
        
        // Create the new result item
        ebnf_item* alternate = new ebnf_item(ebnf_item::ebnf_alternative, attr, ebnfItem->pos(), ebnfItem->final_pos());
        
        alternate->add_child(result);
        alternate->add_child(alternative);
        
        result = alternate;
    }
    
    // Return the result
    return result;
}

/// \brief Converts a simple EBNF item into an ebnf_item object
static ebnf_item* definition_for(const ast_Simple_Ebnf_Item* simpleItem) {
    // Work out the semantics for this item
    ebnf_item_attributes attr;

    if (simpleItem->yy_optional_Semantic_Specification->yy_Semantic_Specification) {
        attr = definition_for(simpleItem->yy_optional_Semantic_Specification->yy_Semantic_Specification);
    }

    // Item depends on the content
    if (simpleItem->yy_Nonterminal) {
        // Get the basic identifier
        wstring sourceIdentifier;
        wstring ntIdentifier = simpleItem->yy_Nonterminal->yy_nonterminal->content<wchar_t>();
        
        // Get the source identifier if it exists
        if (simpleItem->yy_Nonterminal->yy_source_language) {
            sourceIdentifier = simpleItem->yy_Nonterminal->yy_source_language->content<wchar_t>();
        }
        
        // Create the item
        return new ebnf_item(ebnf_item::ebnf_nonterminal, sourceIdentifier, ntIdentifier, attr, simpleItem->pos(), simpleItem->final_pos());
    }
    
    else if (simpleItem->yy_Terminal) {
        // Get the basic identifier
        ebnf_item::type terminalType = ebnf_item::ebnf_terminal;
        wstring         sourceIdentifier;
        wstring         termIdentifier;
        
        // Get the terminal data
        if (simpleItem->yy_Terminal->yy_Basic_Terminal->yy_lexeme_name) {
            terminalType    = ebnf_item::ebnf_terminal;
            termIdentifier  = simpleItem->yy_Terminal->yy_Basic_Terminal->yy_lexeme_name->content<wchar_t>();
        }
        
        else if (simpleItem->yy_Terminal->yy_Basic_Terminal->yy_string) {
            terminalType    = ebnf_item::ebnf_terminal_string;
            termIdentifier  = simpleItem->yy_Terminal->yy_Basic_Terminal->yy_string->content<wchar_t>();
        }
        
        else if (simpleItem->yy_Terminal->yy_Basic_Terminal->yy_character) {
            terminalType    = ebnf_item::ebnf_terminal_character;
            termIdentifier  = simpleItem->yy_Terminal->yy_Basic_Terminal->yy_character->content<wchar_t>();
        }
        
        else {
            // Unknown type of terminal
            return NULL;
        }
        
        // Get the source identifier if it exists
        if (simpleItem->yy_Terminal->yy_source_language) {
            sourceIdentifier = simpleItem->yy_Terminal->yy_source_language->content<wchar_t>();
        }
        
        // Create the item
        return new ebnf_item(terminalType, sourceIdentifier, termIdentifier, attr, simpleItem->pos(), simpleItem->final_pos());        
    }
    
    else if (simpleItem->yy_Guard) {
        // Get the internal item
        ebnf_item* internalItem = definition_for(simpleItem->yy_Guard->yy_Ebnf_Item, ebnf_item_attributes());
        
        if (!internalItem) {
            // Bug
            return NULL;
        }

        // Check for guard attributes
        if (simpleItem->yy_Guard->yy_can_clash) {
            // This guard is allowed to clash with other guards
            attr.guard_can_clash = true;
        }
        
        // Turn into a guard item
        ebnf_item* guardItem = new ebnf_item(ebnf_item::ebnf_guard, attr);
        guardItem->add_child(internalItem);
        
        return guardItem;
    }
    
    else if (simpleItem->yy__star_) {
        // Item of the form X*
        ebnf_item* starredItem = definition_for(simpleItem->yy_Simple_Ebnf_Item);
        if (starredItem == NULL) {
            // Bug
            return NULL;
        }
        
        ebnf_item* result = new ebnf_item(ebnf_item::ebnf_repeat_zero, attr, simpleItem->pos(), simpleItem->final_pos());
        result->add_child(starredItem);
        return result;
    }
    
    else if (simpleItem->yy__plus_) {
        // Item of the form X+
        ebnf_item* plusItem = definition_for(simpleItem->yy_Simple_Ebnf_Item);
        if (plusItem == NULL) {
            // Bug
            return NULL;
        }
        
        ebnf_item* result = new ebnf_item(ebnf_item::ebnf_repeat_one, attr, simpleItem->pos(), simpleItem->final_pos());
        result->add_child(plusItem);
        return result;
    }
    
    else if (simpleItem->yy__question_) {
        // Item of the form X?
        ebnf_item* optionalItem = definition_for(simpleItem->yy_Simple_Ebnf_Item);
        if (optionalItem == NULL) {
            // Bug
            return NULL;
        }
        
        ebnf_item* result = new ebnf_item(ebnf_item::ebnf_optional, attr, simpleItem->pos(), simpleItem->final_pos());
        result->add_child(optionalItem);
        return result;        
    }
    
    else if (simpleItem->yy__openparen_) {
        // Item of the form (X)
        return definition_for(simpleItem->yy_Ebnf_Item, attr);
    }
    
    // Unknown type of item
    return NULL;
}

/// \brief Processes a production definition
static production_definition* definition_for(const ast_Production* production) {
    // Begin a new production
    production_definition* defn = new production_definition(production->pos(), production->final_pos());
    
    // Iterate through the items in this production
    for (list_of_Simple_Ebnf_Item::iterator item = production->yy_items->begin();
         item != production->yy_items->end();
         ++item) {
        // Get the next EBNF item
        ebnf_item* ebnf = definition_for((*item)->yy_Simple_Ebnf_Item);
        
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
    wstring identifier = nonterminal->yy_nonterminal->content<wchar_t>();
    
    // Work out the type
    nonterminal_definition::type ntType = nonterminal_definition::assignment;
    
    if (nonterminal->yy_one_of__equals__or__pipe__equals_) {
        if (nonterminal->yy_one_of__equals__or__pipe__equals_->yy__equals_) {
            ntType = nonterminal_definition::assignment;
        } else if (nonterminal->yy_one_of__equals__or__pipe__equals_->yy__pipe__equals_) {
            ntType = nonterminal_definition::addition;
        }
    } else if (nonterminal->yy_replace) {
        ntType = nonterminal_definition::replace;
    }
    
    // Start defining a new nonterminal
    nonterminal_definition* nonterm = new nonterminal_definition(ntType, identifier, nonterminal->pos(), nonterminal->final_pos());
    
    // Add the productions from this nonterminal
    production_definition* prod = definition_for(nonterminal->yy_Production);
    
    if (prod == NULL) {
        // Doh, bug
        delete nonterm;
        return NULL;
    }
    
    nonterm->add_production(prod);
    
    for (list_of__pipe__Production::iterator nextProduction = nonterminal->yy_list_of__pipe__Production->begin(); 
         nextProduction != nonterminal->yy_list_of__pipe__Production->end(); 
         ++nextProduction) {
        // Get the next production
        production_definition* prod = definition_for((*nextProduction)->yy_Production);
        
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
    for (list_of_Nonterminal_Definition::iterator nonterminal = items->begin(); nonterminal != items->end(); ++nonterminal) {
        nonterminal_definition* defn = definition_for((*nonterminal)->yy_Nonterminal_Definition);
        
        if (defn == NULL) {
            // Doh, bug!
            delete gram;
            return NULL;
        }
        
        gram->add_nonterminal(defn);
    }
    
    return new language_unit(gram);
}

/// \brief Adds a lexeme definition to a lexer block
static bool add_lexeme_definition(const ast_Lexeme_Definition* defn, lexer_block* lexerBlock) {
    // Sanity check
    if (!defn || !lexerBlock) {
        return false;
    }

    // Get the identifier for the new lexeme
    const identifier* lexemeId = defn->yy_name;
    
    // Lexeme should either be a string/character/regex or a reference
    if (defn->yy_one_of_regex_or_one_of_string_or_character) {
        // Get the three alternatives
        const ast_regex*        regex       = defn->yy_one_of_regex_or_one_of_string_or_character->yy_regex;
        const ast_string*       string      = defn->yy_one_of_regex_or_one_of_string_or_character->yy_string;
        const ast_character*    character   = defn->yy_one_of_regex_or_one_of_string_or_character->yy_character;
        
        bool                    alternate   = false;
        bool                    replace     = defn->yy_replace;
        
        if (defn->yy_one_of__equals__or__pipe__equals_) {
            alternate = defn->yy_one_of__equals__or__pipe__equals_->yy__pipe__equals_;
        }
        
        if (regex) {
            lexerBlock->add_definition(new lexeme_definition(lexeme_definition::regex, lexemeId->content<wchar_t>(), regex->content<wchar_t>(), alternate, replace, defn->pos(), defn->final_pos(), regex->pos()));
        } else if (string) {
            lexerBlock->add_definition(new lexeme_definition(lexeme_definition::string, lexemeId->content<wchar_t>(), string->content<wchar_t>(), alternate, replace, defn->pos(), defn->final_pos(), string->pos()));
        } else if (character) {
            lexerBlock->add_definition(new lexeme_definition(lexeme_definition::character, lexemeId->content<wchar_t>(), character->content<wchar_t>(), alternate, replace, defn->pos(), defn->final_pos(), character->pos()));
        } else {
            // Doh, bug: fail
            delete lexerBlock;
            return false;
        }

        // Success
        return true;
    } else if (defn->yy_source_language) {
        // Reference (IMPLEMENT ME)
        return false;
    } else {
        // Doh, bug: fail
        return false;
    }
}

/// \brief Interprets a keyword symbol definition block
static language_unit* definition_for(const list_of_Keyword_Definition* items, const list_of_Lexer_Modifier* modifiers1, const list_of_Lexer_Symbols_Modifier* modifiers2, language_unit::unit_type type) {
    // Work out the modifiers
    bool isWeak             = false;
    bool isCaseInsensitive  = false;
    bool isCaseSensitive    = false;

    if (modifiers1) {
        for (list_of_Lexer_Modifier::iterator modifier = modifiers1->begin(); modifier != modifiers1->end(); ++modifier) {
            if ((*modifier)->yy_Lexer_Modifier->yy_weak) {
                isWeak = true;
            } else if ((*modifier)->yy_Lexer_Modifier->yy_insensitive) {
                isCaseInsensitive = true;
            } else if ((*modifier)->yy_Lexer_Modifier->yy_sensitive) {
                isCaseSensitive = true;
            }
        }
    }

    if (modifiers2) {
        for (list_of_Lexer_Symbols_Modifier::iterator modifier = modifiers2->begin(); modifier != modifiers2->end(); ++modifier) {
            if ((*modifier)->yy_Lexer_Symbols_Modifier->yy_insensitive) {
                isCaseInsensitive = true;
            } else if ((*modifier)->yy_Lexer_Symbols_Modifier->yy_sensitive) {
                isCaseSensitive = true;
            }
        }
    }

    // Start building up the lexer block
    lexer_block* lexerBlock = new lexer_block(isWeak, isCaseInsensitive, isCaseSensitive, items->pos(), items->final_pos());
    
    // Iterate through the items
    for (list_of_Keyword_Definition::iterator keyword = items->begin(); keyword != items->end(); ++keyword) {
        if ((*keyword)->yy_Keyword_Definition->yy_lexeme) {
            // Add the lexeme definition
            if (!add_lexeme_definition((*keyword)->yy_Keyword_Definition->yy_lexeme, lexerBlock)) {
                // Doh, fail
                delete lexerBlock;
                return NULL;
            }
        } else if ((*keyword)->yy_Keyword_Definition->yy_literal) {
            // A literal keyword defined only by its identifier
            const identifier* keywordId = (*keyword)->yy_Keyword_Definition->yy_literal;
            lexerBlock->add_definition(new lexeme_definition(lexeme_definition::literal, keywordId->content<wchar_t>(), keywordId->content<wchar_t>(), false, false, (*keyword)->pos(), (*keyword)->final_pos(), keywordId->pos()));
        } else {
            // Unknown keyword type
            delete lexerBlock;
            return NULL;
        }
    }
    
    // Create the language unit
    return new language_unit(type, lexerBlock);
}

/// \brief Interprets a lexer symbol definition block
static language_unit* definition_for(const list_of_Lexeme_Definition* items, const list_of_Lexer_Modifier* modifiers1, const list_of_Lexer_Symbols_Modifier* modifiers2, const language_unit::unit_type type) {
    // Work out the modifiers
    bool isWeak             = false;
    bool isCaseInsensitive  = false;
    bool isCaseSensitive    = false;

    if (modifiers1) {
        for (list_of_Lexer_Modifier::iterator modifier = modifiers1->begin(); modifier != modifiers1->end(); ++modifier) {
            if ((*modifier)->yy_Lexer_Modifier->yy_weak) {
                isWeak = true;
            } else if ((*modifier)->yy_Lexer_Modifier->yy_insensitive) {
                isCaseInsensitive = true;
            } else if ((*modifier)->yy_Lexer_Modifier->yy_sensitive) {
                isCaseSensitive = true;
            }
        }
    }

    if (modifiers2) {
        for (list_of_Lexer_Symbols_Modifier::iterator modifier = modifiers2->begin(); modifier != modifiers2->end(); ++modifier) {
            if ((*modifier)->yy_Lexer_Symbols_Modifier->yy_insensitive) {
                isCaseInsensitive = true;
            } else if ((*modifier)->yy_Lexer_Symbols_Modifier->yy_sensitive) {
                isCaseSensitive = true;
            }
        }
    }

    // Start building up the lexer block
    lexer_block* lexerBlock = new lexer_block(isWeak, isCaseInsensitive, isCaseSensitive, items->pos(), items->final_pos());
    
    // Iterate through the items
    for (list_of_Lexeme_Definition::iterator lexeme = items->begin(); lexeme != items->end(); ++lexeme) {
        // Add this definition
        if (!add_lexeme_definition((*lexeme)->yy_Lexeme_Definition, lexerBlock)) {
            // Give up if it fails
            delete lexerBlock;
            return NULL;
        }
    }
    
    return new language_unit(type, lexerBlock);
}

/// \brief Creates a language unit from a precedence definition
static language_unit* definition_for(const ast_Precedence_Definition* precedence) {
    // Create the precedence block
    position start  = precedence->pos();
    position end    = precedence->final_pos();

    precedence_block* precBlock = new precedence_block(start, end);

    // Iterate through the items in this definition
    for (list_of_Precedence_Item::iterator item = precedence->yy_items->begin(); item != precedence->yy_items->end(); ++item) {
        precedence_block::item newItem;

        // Set the associativity
        newItem.assoc = precedence_block::nonassoc;
        if ((*item)->yy_Precedence_Item->yy_left) {
            newItem.assoc = precedence_block::left;
        } else if ((*item)->yy_Precedence_Item->yy_right) {
            newItem.assoc = precedence_block::right;
        }
    
        // Get the definition for this item
        const ast_Equal_Precedence_Items* equalItem = (*item)->yy_Precedence_Item->yy_Equal_Precedence_Items;

        // Get the items inside this one
        if (equalItem->yy_Simple_Ebnf_Item) {
            // left 'x' style of item
            ebnf_item* singleItem = definition_for(equalItem->yy_Simple_Ebnf_Item);
            if (!singleItem) {
                // Oops; failed to get the item
                delete precBlock;
                return NULL;
            }

            // TODO: must be a terminal item

            // Add this item
            newItem.items.push_back(singleItem);
        }

        if (equalItem->yy_terminals) {
            // left { 'x' 'y' } style of item
            for (list_of_Simple_Ebnf_Item::iterator ebnfItem = equalItem->yy_terminals->begin(); ebnfItem != equalItem->yy_terminals->end(); ++ebnfItem) {
                ebnf_item* nextItem = definition_for((*ebnfItem)->yy_Simple_Ebnf_Item);
                if (!nextItem) {
                    // Oops; failed to get the item
                    delete precBlock;
                    return NULL;
                }

                // TODO: must be a terminal item

                // Add this item
                newItem.items.push_back(nextItem);
            }
        }

        // Add this item
        precBlock->add_item(newItem);
    }

    // Create the final result
    return new language_unit(precBlock);
}

/// \brief Interprets a language unit
static language_unit* definition_for(const ast_Language_Definition* defn) {
    // Action depends on the typeof node in this AST node
    
    // Most of the lexer type nodes are very similar, except for the node type
    if (defn->yy_Lexer_Symbols_Definition) {
        return definition_for(defn->yy_Lexer_Symbols_Definition->yy_definitions, NULL, defn->yy_Lexer_Symbols_Definition->yy_modifiers, language_unit::unit_lexer_symbols);
    } else if (defn->yy_Lexer_Definition) {
        return definition_for(defn->yy_Lexer_Definition->yy_definitions, defn->yy_Lexer_Definition->yy_modifiers, NULL, language_unit::unit_lexer_definition);
    } else if (defn->yy_Ignore_Definition) {
        return definition_for(defn->yy_Ignore_Definition->yy_definitions, NULL, NULL, language_unit::unit_ignore_definition);
    } else if (defn->yy_Keywords_Definition) {
        return definition_for(defn->yy_Keywords_Definition->yy_definitions, defn->yy_Keywords_Definition->yy_modifiers, NULL, language_unit::unit_keywords_definition);
    } else if (defn->yy_Grammar_Definition) {
        return definition_for(defn->yy_Grammar_Definition->yy_nonterminals);
    } else if (defn->yy_Precedence_Definition) {
        return definition_for(defn->yy_Precedence_Definition);
    }
    
    return NULL;
}

/// \brief Interprets a language block
static language_block* definition_for(const ast_Language_Block* language) {
    // Create the language block
    language_block* result = new language_block(language->yy_name->content<wchar_t>(), language->pos(), language->final_pos());
    
    // Deal with the inherits block, if it exists
    if (language->yy_optional_Language_Inherits->yy_Language_Inherits) {
        // Add the first identifier
        // For the moment, we can only inherit from one language at a time
        result->add_inherits(language->yy_optional_Language_Inherits->yy_Language_Inherits->yy_inherit_from->content<wchar_t>());
    }
    
    // Add the language definitions
    for (list_of_Language_Definition::iterator langDefinition = language->yy_list_of_Language_Definition->begin();
         langDefinition != language->yy_list_of_Language_Definition->end();
         ++langDefinition) {
        // Get the next definition
        language_unit* nextUnit = definition_for((*langDefinition)->yy_Language_Definition);
        
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
    if (toplevel->yy_Language_Block) {
        // Get the definition for this language block
        language_block* language = definition_for(toplevel->yy_Language_Block);
        if (!language) {
            // Doh
            return NULL;
        }
        
        // Turn into a toplevel block
        return new toplevel_block(language);
    }
    
    // Import block
    else if (toplevel->yy_Import_Block) {
        // Fairly simple to convert
        return new toplevel_block(new import_block(process::dequote_string(toplevel->yy_Import_Block->yy_filename->content<wchar_t>()), toplevel->pos(), toplevel->final_pos()));
    }

    // Test block
    else if (toplevel->yy_Test_Block) {
        test_block* test = definition_for(toplevel->yy_Test_Block);
        if (!test) {
            // Doh
            return NULL;
        }

        // Turn into a toplevel block
        return new toplevel_block(test);
    }
    
    // Parser block
    else if (toplevel->yy_Parser_Block) {
        parser_block* parser = definition_for(toplevel->yy_Parser_Block);
        if (!parser) {
            // Doh
            return NULL;
        }

        // Turn into a toplevel block
        return new toplevel_block(parser);
    }
    
    // Failed to parse: doh, bug
    return NULL;
}

/// \brief Turns a parser language object into a definition file object
static definition_file* definition_for(const Parser_Language* language) {
    // Create a new definition file
    definition_file* file = new definition_file();
    
    // Iterate through the top-level definitions
    for (list_of_TopLevel_Block::iterator topLevel = language->yy_list_of_TopLevel_Block->begin(); topLevel != language->yy_list_of_TopLevel_Block->end(); ++topLevel) {
        // Get the definition for this toplevel block
        toplevel_block* newBlock = definition_for((*topLevel)->yy_TopLevel_Block);
        
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
static definition_file* definition_for(const yy_tameparse_language::yy_epsilon_n* root) {
    // Pass the parser language straight through
    return definition_for(root->yy_Parser_Language);
}

/// \brief Type of a parser state
typedef yy_tameparse_language::state parser_state;

/// \brief Parser actions type
typedef yy_tameparse_language::parser_actions parser_actions;

/// \brief A list of errors
typedef language_parser::error_list error_list;

/// \brief Creates an error list when the parser fails
static error_list get_errors(parser_state* state, const wstring& filename, bool result) {
    error_list res;
    
    // No errors if the result was successful
    if (result) return res;
    
    // Add a single error and attempt no recovery
    // TODO: work out the 'expecting' list
    if (state->look().item()) {
        res.push_back(error(error::sev_error, filename, L"SYNTAX_ERROR", L"Syntax error", state->look()->pos()));
    } else {
        res.push_back(error(error::sev_error, filename, L"SYNTAX_END_OF_FILE", L"Unexpected end of file", position(-1, -1, -1)));        
    }
    
    // Return the result
    return res;
}

/// \brief Parses the language file specified in the given string and stores it in this object.
///
/// This will return true if the file parsed correctly. If this is the case, then the file_definition() function
/// will return the result. If there is any existing definition, this will be replaced by this call.
bool language_parser::parse(const std::wstring& language) {
    bool result = false;
    
    // Clear the definition
    m_FileDefinition = definition_file_container(NULL, true);
    m_RecentErrors.clear();

    // Create a stream for this string
    wstringreader reader(language);
    
    // Create the parser
    parser_state* parser_state = yy_tameparse_language::create_yy_Parser_Language<wchar_t>(reader);
    
    // Parse the language
    result          = parser_state->parse();
    m_RecentErrors  = get_errors(parser_state, m_Filename, result);
    
    // Convert to a definition
    if (result) {
        // Fetch the root item (which will be an epsilon item at the moment due to the way the parser is built up)
        // The name of this item will probably change to something more sensible at some point (and I'll forget to remove this comment)
        const yy_tameparse_language::yy_Parser_Language_n* root = static_cast<const yy_tameparse_language::yy_Parser_Language_n*>(parser_state->get_item().item());
        
        // Turn into a definition
        m_FileDefinition = definition_file_container(definition_for(root), true);
    }
    
    // Finished with the parser
    delete parser_state;
    
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
    m_RecentErrors.clear();
    
    // Create a lexer for this string
    lexeme_stream* stream = yy_tameparse_language::lexer.create_stream_from<wchar_t>(languageReader);
    
    // Create the parser
    // Currently using the 'raw' parser here (due to the state of the C++ generator at this point in time: I imagine it will have
    // a few more interesting/easy ways of creating parsers later on)
    parser_state* parser_state = yy_tameparse_language::ast_parser.create_parser(new parser_actions(stream));
    
    // Parse the language
    result          = parser_state->parse();
    m_RecentErrors  = get_errors(parser_state, m_Filename, result);
    
    // Convert to a definition
    if (result) {
        // Fetch the root item (which will be an epsilon item at the moment due to the way the parser is built up)
        // The name of this item will probably change to something more sensible at some point (and I'll forget to remove this comment)
        const yy_tameparse_language::yy_Parser_Language_n* root = static_cast<const yy_tameparse_language::yy_Parser_Language_n*>(parser_state->get_item().item());
        
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
    m_RecentErrors.clear();
    
    // Create a lexer for this string
    lexeme_stream* stream = yy_tameparse_language::lexer.create_stream_from<wchar_t>(languageReader);
    
    // Create the parser
    // Currently using the 'raw' parser here (due to the state of the C++ generator at this point in time: I imagine it will have
    // a few more interesting/easy ways of creating parsers later on)
    parser_state* parser_state = yy_tameparse_language::ast_parser.create_parser(new parser_actions(stream));
    
    // Parse the language
    result          = parser_state->parse();
    m_RecentErrors  = get_errors(parser_state, m_Filename, result);
    
    // Convert to a definition
    if (result) {
        // Fetch the root item (which will be an epsilon item at the moment due to the way the parser is built up)
        // The name of this item will probably change to something more sensible at some point (and I'll forget to remove this comment)
        const yy_tameparse_language::yy_Parser_Language_n* root = static_cast<const yy_tameparse_language::yy_Parser_Language_n*>(parser_state->get_item().item());
        
        // Turn into a definition
        m_FileDefinition = definition_file_container(definition_for(root), true);
    }
    
    // Finished with the parser
    delete parser_state;
    delete stream;
    
    // Done
    return result;
}
