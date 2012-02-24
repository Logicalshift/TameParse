//
//  output_stage.cpp
//  Parse
//
//  Created by Andrew Hunter on 29/08/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#include <string>
#include <sstream>

#include "TameParse/Compiler/output_stage.h"

using namespace std;
using namespace dfa;
using namespace contextfree;
using namespace compiler;

/// \brief Creates a new output stage
output_stage::output_stage(console_container& console, const std::wstring& filename, lexer_stage* lexer, language_stage* language, lr_parser_stage* parser)
: compilation_stage(console, filename)
, m_LexerStage(lexer)
, m_LanguageStage(language)
, m_ParserStage(parser) {
}

/// \brief Destructor
output_stage::~output_stage() {
}

/// \brief Compiles the parser specified by this stage
///
/// Subclasses can override this if they want to substantially change the way that the
/// compiler is generated.
void output_stage::compile() {
    // TODO: sanity check

    // Start writing the output
    begin_output();

    // Write out the symbol IDs, etc first
    define_symbols();

    // Next, write out the lexer definition
    define_lexer_tables();

    // Writes out the parser tables
    define_parser_tables();

    // Writes out the AST tables
    define_ast_tables();

    // Finished writing the output
    end_output();
}

/// \brief Defines the symbols associated with this language
void output_stage::define_symbols() {
    // TODO: remove me!
}

/// \brief Writes out the lexer tables (the symbol map and the state table)
void output_stage::define_lexer_tables() {
    // TODO: remove me!
}

/// \brief Writes out the AST tables
void output_stage::define_ast_tables() {
    // TODO: remove me!
}

/// \brief Writes out the parser tables
void output_stage::define_parser_tables() {
    // TODO: remove me
}

/// \brief About to begin writing out output
void output_stage::begin_output() {
    // Do nothing in the default implementation
}

/// \brief Finishing writing out output
void output_stage::end_output() {
    // Do nothing in the default implementation
}

// =========
//  Symbols
// =========

/// \brief Generates the terminal symbols list
void output_stage::generate_terminal_symbols() {
    // Clear out the existing symbols (so we regenerate if this is called multiple times)
    m_TerminalSymbols.clear();

    // Fill in the terminal symbols
    for (int symbolId = 0; symbolId < terminals().count_symbols(); ++symbolId) {
        m_TerminalSymbols.push_back(terminal_symbol(terminals().name_for_symbol(symbolId), symbolId, item_container(new terminal(symbolId))));
    }
}

/// \brief Generates the nonterminal symbols list
void output_stage::generate_nonterminal_symbols() {
    // Clear out the existing symbols (so we regenerate if this is called multiple times)
    m_NonterminalSymbols.clear();

    // Fill in the nonterminal symbols
    for (int symbolId = 0; symbolId < gram().max_item_identifier(); ++symbolId) {
        // Assume that the nonterminal IDs match up to item IDs (they should do)
        item_container ntItem = gram().item_with_identifier(symbolId);

        // Must be a nonterminal
        if (ntItem->type() == item::terminal) continue;

        // Output this item
        m_NonterminalSymbols.push_back(nonterminal_symbol(gram().name_for_nonterminal(symbolId), symbolId, ntItem));
    }
}

/// \brief The first terminal symbol
output_stage::terminal_symbol_iterator output_stage::begin_terminal_symbol() {
    // Generate the terminal symbols if they don't already exist (or, as an edge case, if they exist but there aren't any)
    if (m_TerminalSymbols.empty()) generate_terminal_symbols();

    return m_TerminalSymbols.begin();
}

/// \brief The symbol after the final terminal symbol
output_stage::terminal_symbol_iterator output_stage::end_terminal_symbol() {
    // Generate the terminal symbols if they don't already exist (or, as an edge case, if they exist but there aren't any)
    if (m_TerminalSymbols.empty()) generate_terminal_symbols();

    return m_TerminalSymbols.end();
}

/// \brief The first nonterminal symbol
output_stage::nonterminal_symbol_iterator output_stage::begin_nonterminal_symbol() {
    // Generate the terminal symbols if they don't already exist (or, as an edge case, if they exist but there aren't any)
    if (m_NonterminalSymbols.empty()) generate_nonterminal_symbols();

    return m_NonterminalSymbols.begin();
}

/// \brief The symbol after the final nonterminal symbol
output_stage::nonterminal_symbol_iterator output_stage::end_nonterminal_symbol() {
    // Generate the terminal symbols if they don't already exist (or, as an edge case, if they exist but there aren't any)
    if (m_NonterminalSymbols.empty()) generate_nonterminal_symbols();

    return m_NonterminalSymbols.end();
}

// =======
//  Lexer
// =======

/// \brief Generates the lexer symbol map
void output_stage::generate_lexer_symbol_map() {
    // Clear out the symbol map
    m_LexerSymbolMap.clear();

    // Get the symbol map
    const dfa::symbol_map& symbols = m_LexerStage->dfa()->symbols();

    // Go through all of the symbol sets
    for (dfa::symbol_map::iterator setIt = symbols.begin(); setIt != symbols.end(); ++setIt) {
        // Go through the ranges in each set
        for (symbol_set::iterator rangeIt = setIt->first->begin(); rangeIt != setIt->first->end(); ++rangeIt) {
            m_LexerSymbolMap.push_back(symbol_map(*rangeIt, setIt->second));
        }
    }
}

void output_stage::generate_lexer_transitions() {
    // Clear the transitions
    m_LexerTransitions.clear();

    // Get the DFA
    const ndfa* dfa = m_LexerStage->dfa();

    // Write out each state in turn
    for (int stateId = 0; stateId < dfa->count_states(); ++stateId) {
        // Start writing out this state
        const state& state = dfa->get_state(stateId);

        for (state::iterator transit = state.begin(); transit != state.end(); ++transit) {
            m_LexerTransitions.push_back(lexer_state_transition(stateId, transit->symbol_set(), transit->new_state()));
        }
    }
}

void output_stage::generate_lexer_actions() {
    // Clear the actions
    m_LexerActions.clear();

    // Get the DFA
    const ndfa* dfa = m_LexerStage->dfa();

    // Write out the actions for each state
    for (int stateId = 0; stateId < dfa->count_states(); ++stateId) {
        // Get the actions for this state
        typedef ndfa::accept_action_list accept_action_list;
        const accept_action_list& actions = dfa->actions_for_state(stateId);

        // Nothing to do if there are no actions for this state
        if (actions.begin() == actions.end()) {
            m_LexerActions.push_back(lexer_state_action(stateId, false, -1));
            continue;
        }

        // Write out the highest action
        accept_action_list::const_iterator thisAction = actions.begin();
        const accept_action* highest = *thisAction;

        ++thisAction;
        for (; thisAction != actions.end(); ++thisAction) {
            if ((*highest) < **thisAction) {
                highest = *thisAction;
            }
        }

        // Write out this action
        m_LexerActions.push_back(lexer_state_action(stateId, true, highest->symbol()));
    }
}

/// \brief The first item in the symbol map
output_stage::symbol_map_iterator output_stage::begin_symbol_map() {
    if (m_LexerSymbolMap.empty()) generate_lexer_symbol_map();
    return m_LexerSymbolMap.begin();
}

/// \brief The item after the final item in the symbol map
output_stage::symbol_map_iterator output_stage::end_symbol_map() {
    if (m_LexerSymbolMap.empty()) generate_lexer_symbol_map();
    return m_LexerSymbolMap.end();
}

/// \brief The first lexer state transition
///
/// Lexer state transitions are returned in sorted order (by state, then by symbol set ID)
output_stage::lexer_state_transition_iterator output_stage::begin_lexer_state_transition() {
    if (m_LexerTransitions.empty()) generate_lexer_transitions();
    return m_LexerTransitions.begin();
}

/// \brief The final lexer state transition
output_stage::lexer_state_transition_iterator output_stage::end_lexer_state_transition() {
    if (m_LexerTransitions.empty()) generate_lexer_transitions();
    return m_LexerTransitions.end();
}

/// \brief The first lexer state action
///
/// Actions are returned ordered by state
output_stage::lexer_state_action_iterator output_stage::begin_lexer_state_action() {
    if (m_LexerActions.empty()) generate_lexer_actions();
    return m_LexerActions.begin();
}

/// \brief The final lexer state action
output_stage::lexer_state_action_iterator output_stage::end_lexer_state_action() {
    if (m_LexerActions.empty()) generate_lexer_actions();
    return m_LexerActions.end();
}

// =====
//  AST
// =====

/// \brief Returns a name for a grammar rule
wstring output_stage::name_for_rule(const contextfree::rule_container& thisRule) {
    // Zero-length rules are called 'empty'
    if (thisRule->items().size() == 0) {
        return L"empty";
    }

    // Short rules are just named after the items
    else if (thisRule->items().size() <= 3) {
        bool            first = true;
        wstringstream   res;

        for (size_t itemId = 0; itemId < thisRule->items().size(); ++itemId) {
            // Append divider
            if (!first) {
                res << L"_";
            }

            // Append this item
            res << name_for_item(thisRule->items()[itemId]);

            // No longer first
            first = false;
        }

        return res.str();
    }

    // Other rules are named after the first item and _etc
    else {
        return name_for_item(thisRule->items()[0]) + L"_etc";       
    }
}

/// \brief Returns a name for an EBNF item
wstring output_stage::name_for_ebnf_item(const contextfree::ebnf& ebnfItem) {
    // Work out the number of rules in this item
    size_t numRules = ebnfItem.count_rules();

    // Items with no rules are just called 'empty'
    if (numRules == 0) {
        return L"empty";
    }

    // Items with rules are called 'x' or 'y' or 'z' etc
    else {
        wstringstream   res;
        bool            first = true;

        for (ebnf::rule_iterator nextRule = ebnfItem.first_rule(); nextRule != ebnfItem.last_rule(); ++nextRule) {
            // Append _or_
            if (!first) {
                res << L"_or_";
            }

            // Append the name for this rule
            res << name_for_rule(*nextRule);
            
            // Move on
            first = false;
        }

        // Convert to a string
        return res.str();
    }
}

/// \brief Strips quote characters from a symbol name
static wstring strip(const wstring& symbolName) {
    // Must have at least one quoted character
    if (symbolName.size() < 3) {
        return symbolName;
    }

    // Quotes are '<>', '"' and '''
    wchar_t firstChar   = symbolName[0];
    wchar_t lastChar    = symbolName[symbolName.size()-1];

    if ((firstChar == L'<' && lastChar == L'>')
        || (firstChar == L'"' && lastChar == L'"')
        || (firstChar == L'\'' && lastChar == L'\'')) {
        return symbolName.substr(1, symbolName.size()-2);
    }

    // Default is to pass the name through untouched
    return symbolName;
}

/// \brief Gets a string name that can be used to represent a specific grammar item
wstring output_stage::name_for_item(const contextfree::item_container& it) {
    // Start building up the result
    wstringstream res;

    // Action depends on the kind of item
    switch (it->type()) {
    case item::empty:
        res << L"epsilon";
        break;

    case item::eoi:
        res << L"end_of_input";
        break;

    case item::eog:
        res << L"end_of_guard";
        break;

    case item::terminal:
        res << strip(terminals().name_for_symbol(it->symbol()));
        break;

    case item::nonterminal:
        res << strip(gram().name_for_nonterminal(it->symbol()));
        break;

    case item::optional:
        res << L"optional_" << name_for_ebnf_item((const ebnf&)*it);
        break;

    case item::repeat:
    case item::repeat_zero_or_one:
        res << L"list_of_" << name_for_ebnf_item((const ebnf&)*it);
        break;

    case item::alternative:
        res << L"one_of_" << name_for_ebnf_item((const ebnf&)*it);
        break;

    default:
        // Unknown type of item
        res << L"unknown_item";
        break;
    }

    // Don't allow 0-length item names
    wstring name = res.str();
    if (name.empty()) {
        name = L"item";
    }

    // Return the result
    return name;    
}

/// \brief Returns true if the specified name should be considered 'valid'
bool output_stage::name_is_valid(const std::wstring& name) {
    // Only empty names are invalid by default
    return !name.empty();
}

/// \brief Generates the rules for each nonterminal
void output_stage::generate_ast_rules() {
    // Maps nonterminals to their corresponding rules
    map<int, rule_list> rulesForNonterminal;

    int maxNtId = -1;
    
    // Iterate through the rules
    for (int ruleId = 0; ruleId < gram().max_rule_identifier(); ++ruleId) {
        // Fetch this rule
        const rule_container& nextRule = gram().rule_with_identifier(ruleId);
        
        // Get the nonterminal ID
        int nonterminalId = gram().identifier_for_item(nextRule->nonterminal());
        if (nonterminalId > maxNtId) maxNtId = nonterminalId;
        
        // Append to the list
        rulesForNonterminal[nonterminalId].push_back(nextRule);
    }

    // Allocate space for the rules
    m_RulesForNonterminal.clear();
    m_RulesForNonterminal.resize(maxNtId+1);

    // Append the rules to the list
    for (int nonterminalId = 0; nonterminalId <= maxNtId; ++nonterminalId) {
        // Fetch the nonterminal this corresponds to
        ast_nonterminal& thisNt = m_RulesForNonterminal[nonterminalId];

        // Set it up
        thisNt.nonterminalId = nonterminalId;

        for (rule_list::iterator nextRule = rulesForNonterminal[nonterminalId].begin(); nextRule != rulesForNonterminal[nonterminalId].end(); ++nextRule) {
            // Get the identifier for this rule
            int ruleId = gram().identifier_for_rule(*nextRule);

            // Get information about the type of nonterminal that this rule is for
            item::kind  nonterminalType = (*nextRule)->nonterminal()->type();
            bool        isRepeating     = nonterminalType == item::repeat || nonterminalType == item::repeat_zero_or_one;

            // Get the list for this rule (which will be empty at this point)
            ast_rule_item_list& ruleList = thisNt.rules[ruleId];

            /// Names that have been used within this rule
            set<wstring> usedNames;

            // Fill in the items for this rule
            for (rule::iterator ruleItem = (*nextRule)->begin(); ruleItem != (*nextRule)->end(); ++ruleItem) {
                // Generate a unique name for this item within this rule; by default it is the same as the item name
                wstring baseName = m_LanguageStage->get_rule_item_data().attributes_for(**nextRule, ruleItem).name;
                
                // Name the item after its type if there is no associated name
                if (baseName.empty()) {
                    baseName = name_for_item(*ruleItem);
                }

                // Use the base name as the default name
                wstring uniqueName  = baseName;
                int     offset      = 1;

                while (usedNames.find(uniqueName) != usedNames.end() || !name_is_valid(uniqueName)) {
                    // Append _2, etc if this name has already been encountered in this rule or is invalid
                    offset++;

                    // Generate a new unique name
                    wstringstream newUnique;
                    newUnique << baseName << L"_" << offset;

                    uniqueName = newUnique.str();
                }

                // Remember the unique name
                usedNames.insert(uniqueName);

                // Set the 'EBNF repetition' flag
                bool isEbnfRepeat = false;
                if (isRepeating && ruleItem == (*nextRule)->begin() && (**ruleItem) == *(*nextRule)->nonterminal()) {
                    isEbnfRepeat = true;
                }

                // Add a new item for this rule
                ruleList.push_back(ast_rule_item((*ruleItem)->type() == item::terminal, (*ruleItem)->symbol(), *ruleItem, uniqueName, isEbnfRepeat));
            }
        }
    }
}

/// \brief Placeholder nonterminal used when there are no rules
static output_stage::ast_nonterminal s_NoNonterminal;

/// \brief Returns the AST definition for the specified nonterminal
const output_stage::ast_nonterminal& output_stage::get_ast_nonterminal(int nonterminalId) {
    if (m_RulesForNonterminal.empty()) generate_ast_rules();

    if (nonterminalId < 0 || nonterminalId >= (int)m_RulesForNonterminal.size()) return s_NoNonterminal;

    return m_RulesForNonterminal[nonterminalId];
}
