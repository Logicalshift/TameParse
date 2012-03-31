//
//  lr_parser_stage.cpp
//  Parse
//
//  Created by Andrew Hunter on 27/08/2011.
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
#include "TameParse/Compiler/lr_parser_stage.h"
#include "TameParse/Lr/conflict.h"
#include "TameParse/Lr/ignored_symbols.h"
#include "TameParse/Language/formatter.h"
#include "TameParse/Lr/ast_parser.h"
#include "TameParse/Lr/lr1_rewriter.h"
#include "TameParse/Compiler/conflict_attribute_rewriter.h"

using namespace std;
using namespace dfa;
using namespace contextfree;
using namespace lr;
using namespace language;
using namespace compiler;

/// \brief Constructor
lr_parser_stage::lr_parser_stage(console_container& console, const std::wstring& filename, language_stage* languageCompiler, lexer_stage* lexerCompiler, const vector<wstring>& startSymbols)
: compilation_stage(console, filename)
, m_Language(languageCompiler)
, m_LexerCompiler(lexerCompiler)
, m_StartSymbols(startSymbols)
, m_StartPosition(position(-1,-1,-1))
, m_Parser(NULL)
, m_Tables(NULL) {
    // Add empty positions for each symbol
    for (size_t x=0; x<m_StartSymbols.size(); ++x) {
        m_SymbolStartPosition.push_back(position(-1,-1,-1));
    }
}

/// \brief Constructure which builds the list of start symbols from a parser block
lr_parser_stage::lr_parser_stage(console_container& console, const std::wstring& filename, language_stage* languageCompiler, lexer_stage* lexerCompiler, parser_block* parserBlock) 
: compilation_stage(console, filename)
, m_Language(languageCompiler)
, m_LexerCompiler(lexerCompiler)
, m_StartPosition(parserBlock->start_pos())
, m_StartSymbols(parserBlock->start_symbols())
, m_Parser(NULL)
, m_Tables(NULL) {
    // Make all the symbols begin in the same place as this block
    // TODO: actually record where the symbols are specified
    for (size_t x=0; x<m_StartSymbols.size(); ++x) {
        m_SymbolStartPosition.push_back(m_StartPosition);
    }   
}

/// \brief Destructor
lr_parser_stage::~lr_parser_stage() {
    // Finished with the parser
    if (m_Parser) {
        delete m_Parser;
        m_Parser = NULL;
    }
    
    if (m_Tables) {
        delete m_Tables;
        m_Tables = NULL;
    }
}


/// \brief Search for clashing guards
///
/// A clashing guard occurs when two different guards are specified for the same symbol
/// for a state. These need to be warned on as they can be an indication that the
/// parser is invalid (ie, the parser will not distinguish something that it is
/// supposed to distinguish)
static void warn_clashing_guards(console& cons, const language_stage* language, lalr_builder* builder) {
    // TODO: have a way to mark guards that are permitted to be in the same state
    
    // Iterate through the states
    for (int stateId = 0; stateId < builder->count_states(); ++stateId) {
        // Map of terminal symbol IDs to guard actions
        typedef map<item_container, set<item_container> > guard_to_symbol;
        guard_to_symbol guardForSymbol;

        // Search for guard actions 
        const lr_action_set& actions = builder->actions_for_state(stateId);

        for (lr_action_set::const_iterator nextAction = actions.begin(); nextAction != actions.end(); ++nextAction) {
            // Ignore actions that are not guard actions
            if ((*nextAction)->type() != lr_action::act_guard) continue;

            // Ignore the action if it's not on a terminal symbol
            if ((*nextAction)->item()->type() != item::terminal) continue;

            // The guard item is the nonterminal of the action rule
            guardForSymbol[(*nextAction)->item()].insert((*nextAction)->rule()->nonterminal());
        }

        // Warn of any guards that have conflicting actions
        set<item_container> warnedGuards;

        for (guard_to_symbol::iterator guarded = guardForSymbol.begin(); guarded != guardForSymbol.end(); ++guarded) {
            // No clash if there's only one guard for this symbol
            if (guarded->second.size() <= 1) continue;

            // Ignore guards that we've already warned about
            bool warned = true;
            for (set<item_container>::iterator clashing = guarded->second.begin(); clashing != guarded->second.end(); ++clashing) {
                if (warnedGuards.find(*clashing) == warnedGuards.end()) {
                    // Haven't warned about this guard yet, so this is a new kind of clash
                    warned = false;
                    break;
                }
            }

            if (warned) continue;

            // Default severity is 'warning' for the first item, and 'detail' for the remainder
            bool shownWarning = false;
            
            // Guard items that have the clashing flag set
            set<lr0_item_container> allowedToClash;

            // Get the state ID
            const lalr_state_container& state = builder->machine().state_with_id(stateId);

            // Iterate through the guards
            for (set<item_container>::iterator clashingGuard = guarded->second.begin(); clashingGuard != guarded->second.end(); ++clashingGuard) {
                // Warned about this guard
                warnedGuards.insert(*clashingGuard);

                // Find all of the LR items that have a reference to this guard
                for (lalr_state::iterator lrItem = state->begin(); lrItem != state->end(); ++lrItem) {
                    // Ignore at end items
                    if ((*lrItem)->at_end()) continue;

                    // Ignore items that do not refer to this guard
                    // (Assumption is that all usages of the guard have the same initial set, so any symbol refers to any reference to this guard)
                    if ((*lrItem)->rule()->items()[(*lrItem)->offset()] != *clashingGuard) continue;

                    // Fetch the attributes for this item
                    const ebnf_item_attributes& attr = language->get_rule_item_data().attributes_for(*(*lrItem)->rule(), (*lrItem)->offset());

                    // Don't show warnings for guards that have the 'can-clash' flag set except if they clash with guards that do not have it set
                    if (attr.guard_can_clash) {
                        allowedToClash.insert(*lrItem);
                        continue;
                    }

                    // Get the position of this rule
                    int             ruleId      = (*lrItem)->rule()->identifier(*language->grammar());
                    position        rulePos     = language->rule_definition_pos(ruleId);
                    const wstring&  ruleFile    = language->rule_definition_file(ruleId);

                    // We know the item, the guard and the position: generate the error message
                    if (!shownWarning) {
                        wstringstream warning;
                        warning << L"Found clashing guards on symbol '" 
                                << formatter::to_string(*guarded->first, builder->gram(), builder->terminals())
                                << L"'";
                        
                        cons.report_error(error(error::sev_warning, ruleFile, L"CLASHING_GUARDS", warning.str(), rulePos));
                    }

                    // Provide detail messages on where the clash is
                    wstringstream msg;
                    msg << (!shownWarning?L"when used here: ":L"or here: ")
                        << formatter::to_string(**lrItem, builder->gram(), builder->terminals());
                    
                    cons.report_error(error(error::sev_detail, language->filename(), L"CLASHING_GUARDS_DETAIL", msg.str(), rulePos));

                    shownWarning    = true;
                }
            }
            
            // Show warnings for guards that had the 'can clash' flag set if there were any that did not have it set
            if (shownWarning) {
                for (set<lr0_item_container>::const_iterator remainingGuard = allowedToClash.begin(); remainingGuard != allowedToClash.end(); ++remainingGuard) {
                    int             ruleId      = (*remainingGuard)->rule()->identifier(*language->grammar());
                    position        rulePos     = language->rule_definition_pos(ruleId);
                    
                    wstringstream msg;
                    msg << L"or here: "
                    << formatter::to_string(**remainingGuard, builder->gram(), builder->terminals());
                    
                    cons.report_error(error(error::sev_detail, language->filename(), L"CLASHING_GUARDS_DETAIL", msg.str(), rulePos));
                }
            }
        }
    }
}


/// \brief Compiles the parser specified by the parameters to this stage
void lr_parser_stage::compile() {
    // Verbose message to say which stage we're at
    cons().verbose_stream() << L"  = Building parser" << endl;

    // Recycle the parser generator if it already exists
    if (m_Parser) {
        delete m_Parser;
        m_Parser = NULL;
    }
    
    if (m_Tables) {
        delete m_Tables;
        m_Tables = NULL;
    }

    // Sanity check (language)
    if (!m_Language) {
        cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE", L"Language compiler stage was not supplied to parser stage", m_StartPosition));
        return;
    }

    if (!m_Language->lexer()) {
        cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE_LEXER", L"Language compiler stage has not generated a lexer", m_StartPosition));
        return;
    }

    if (!m_Language->terminals()) {
        cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE_TERMINALS", L"Language compiler stage has not generated a terminal dictionary", m_StartPosition));      
        return;
    }

    if (!m_Language->weak_symbols()) {
        cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE_WEAK_SYMBOLS", L"Language compiler stage has not the set of weak symbols", m_StartPosition));
        return;
    }

    if (!m_Language->ignored_symbols()) {
        cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE_IGNORE_SYMBOLS", L"Language compiler stage has not generated the set of ignore symbols", m_StartPosition));
    }

    if (!m_Language->grammar()) {
        cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LANGUAGE_GRAMMAR", L"Language compiler stage has not generated a grammar", m_StartPosition));        
        return;
    }

    // Sanity check (lexer)
    if (!m_LexerCompiler) {
        cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LEXER", L"Lexer compiler stage was not supplied to parser stage", m_StartPosition));
        return;
    }

    if (!m_LexerCompiler->dfa()) {
        cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LEXER_DFA", L"Lexer compiler stage has not generate a DFA", m_StartPosition));
        return;
    }

    if (!m_LexerCompiler->weak_symbols()) {
        cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_LEXER_DFA", L"Lexer compiler stage has not generate a weak symbols rewriter", m_StartPosition));
        return;
    }

    // Create a new parser builder
    m_Parser = new lalr_builder(*m_Language->grammar(), *m_Language->terminals());

    // Get the nonterminal items corresponding to the start symbols
    vector<item_container> startItems;

    for (size_t startSymbolId = 0; startSymbolId < m_StartSymbols.size(); ++startSymbolId) {
        // Get the symbol
        const wstring& startSymbol = m_StartSymbols[startSymbolId];

        // Find the nonterminal item corresponding to this symbol
        if (!m_Language->grammar()->nonterminal_is_defined(startSymbol)) {
            // Report an error if this nonterminal is not defined
            wstringstream msg;
            msg << L"Start symbol is not defined: " << startSymbol;

            cons().report_error(error(error::sev_error, filename(), L"UNDEFINED_NONTERMINAL", msg.str(), m_SymbolStartPosition[startSymbolId]));
            continue;
        }

        // Add to the list of start items
        startItems.push_back(m_Language->grammar()->get_nonterminal(startSymbol));
    }

    // Give up if there are no symbols defined
    if (startItems.empty()) {
        cons().report_error(error(error::sev_error, filename(), L"NO_START_SYMBOLS", L"No start symbols are defined", m_StartPosition));
        return;
    }
    
    // Generate the ignore actions
    ignored_symbols* ignored = new ignored_symbols();
    action_rewriter_container ignoreContainer(ignored, true);
    
    for (set<int>::const_iterator ignoredTerminalId = m_Language->ignored_symbols()->begin(); ignoredTerminalId != m_Language->ignored_symbols()->end(); ++ignoredTerminalId) {
        item_container newTerm(new terminal(*ignoredTerminalId), true);
        ignored->add_item(newTerm);
    }

    // Add the initial states to the LALR builder
    m_InitialStates.clear();
    for (vector<item_container>::iterator initialItem = startItems.begin(); initialItem != startItems.end(); ++initialItem) {
        m_InitialStates.push_back(m_Parser->add_initial_state(*initialItem));
    }

    // Add any language rewriters that might be defined
    typedef language_stage::rewriter_list rewriter_list;
    for (rewriter_list::const_iterator languageRewriter = m_Language->action_rewriters()->begin(); languageRewriter != m_Language->action_rewriters()->end(); ++languageRewriter) {
        m_Parser->add_rewriter(*languageRewriter);
    }
    
    // Add the weak symbols and ignore items actions
    // TODO: it might be good to have a way to supply extra rewriters from other stages instead of just having them
    // hardcoded here. This is good enough for now, though.
    m_Parser->add_rewriter(action_rewriter_container(m_LexerCompiler->weak_symbols(), false));
    m_Parser->add_rewriter(action_rewriter_container(new conflict_attribute_rewriter(&m_Language->get_rule_item_data())));
    if (!cons().get_option(L"enable-lr1-resolver").empty()) {
        m_Parser->add_rewriter(action_rewriter_container(new lr1_rewriter()));
    }
    m_Parser->add_rewriter(ignoreContainer);

    // Build the parser
    m_Parser->complete_parser();

    // Get any conflicts that might exist
    conflict_list conflictList;
    cons().verbose_stream() << L"  = Checking for conflicts" << endl;
    conflict::find_conflicts(*m_Parser, conflictList);

    // Report the conflicts
    error::severity shiftReduceSev  = error::sev_warning;
    error::severity reduceReduceSev = error::sev_error;

    // The allow-reduce-conflicts option can be used to allow reduce/reduce conflicts
    if (!cons().get_option(L"allow-reduce-conflicts").empty()) {
        reduceReduceSev = error::sev_warning;
    }

    // The no-conflicts option can be used to report an error on all conflicts
    if (!cons().get_option(L"no-conflicts").empty()) {
        shiftReduceSev = reduceReduceSev = error::sev_error;
    }

    for (conflict_list::iterator conflict = conflictList.begin(); conflict != conflictList.end(); ++conflict) {
        // We only show detail if the show-conflict-details option is set
        bool showDetail = !cons().get_option(L"show-conflict-details").empty();
        
        // Test the type of this conflict
        if ((*conflict)->first_shift_item() != (*conflict)->last_shift_item()) {
            // Shift/reduce conflict: we report the 'shift' part of the conflict as the first line
            for (lr0_item_set::const_iterator shiftItem = (*conflict)->first_shift_item(); shiftItem != (*conflict)->last_shift_item(); ++shiftItem) {
                // Start building the message
                wstringstream   shiftMessage;
                error::severity sev = shiftReduceSev;

                // Message is different if this is the initial message for this conflict vs a detail message
                if (shiftItem == (*conflict)->first_shift_item()) {
                    // Displaying the shift/reduce warning if we're on the first shift item
                    shiftMessage << L"Shift/reduce conflict on";
                    shiftMessage << L" '" << formatter::to_string(*(*conflict)->token(), *m_Language->grammar(), *m_Language->terminals()) << L"':";
                } else {
                    // Displaying additional items
                    shiftMessage << L"  in:";
                    sev = error::sev_detail;
                }

                // Add the item being shifted
                shiftMessage << L" " << formatter::to_string(**shiftItem, *m_Language->grammar(), *m_Language->terminals());

                // Display the warning/error
                int             ruleId      = (*shiftItem)->rule()->identifier(*m_Language->grammar());
                position        rulePos     = m_Language->rule_definition_pos(ruleId);
                const wstring&  ruleFile    = m_Language->rule_definition_file(ruleId);
                
                if (sev != error::sev_detail || showDetail) {
                    cons().report_error(error(sev, ruleFile, L"CONFLICT_SHIFT_REDUCE", shiftMessage.str(), rulePos));
                }
            }
        }

        // Display the reductions for this conflict
        for (conflict::reduce_iterator reduceItem = (*conflict)->first_reduce_item(); reduceItem != (*conflict)->last_reduce_item(); ++reduceItem) {
            // Start building the message
            wstring         reduceCode      = L"DETAIL_REDUCE";
            error::severity reductionSev    = error::sev_detail;
            wstringstream   reduceMessage;

            // This is a reduce/reduce conflict if this is the first conflict in the list (ie, no other shift or reduce items)
            if (reduceItem == (*conflict)->first_reduce_item() && (*conflict)->first_shift_item() == (*conflict)->last_shift_item()) {
                // This is a reduce/reduce conflict
                reductionSev = reduceReduceSev;
                reduceCode = L"CONFLICT_REDUCE_REDUCE";
                reduceMessage << L"Reduce/reduce conflict on";
                reduceMessage << L" '" << formatter::to_string(*(*conflict)->token(), *m_Language->grammar(), *m_Language->terminals()) << L"':";
            } else {
                // Displaying additional items
                reduceMessage << L"or reduce:";
            }

            // Add the item being reduced
            reduceMessage << L" " << formatter::to_string(*reduceItem->first->rule(), *m_Language->grammar(), *m_Language->terminals());
            
            // Display the message for this item
            int             ruleId      = reduceItem->first->rule()->identifier(*m_Language->grammar());
            position        rulePos     = m_Language->rule_definition_pos(ruleId);
            const wstring&  ruleFile    = m_Language->rule_definition_file(ruleId);
            cons().report_error(error(reductionSev, ruleFile, reduceCode, reduceMessage.str(), rulePos));

            // For reduce/reduce conflicts, display the context in which the reduction can occur
            if (showDetail) {
                set<item_container> displayedNonterminals;
                report_reduce_conflict(reduceItem, reduceItem->first->rule()->nonterminal(), displayedNonterminals, 0);
            }
        }

        // We don't understand the conflict type if there are no reduce items
        if ((*conflict)->first_reduce_item() == (*conflict)->last_reduce_item()) {
            if ((*conflict)->first_shift_item() == (*conflict)->last_shift_item()) {
                cons().report_error(error(error::sev_bug, filename(), L"BUG_CONFLICT_NO_ACTIONS", L"Found a conflict with no actions of any kind", m_StartPosition));                
            } else {
                cons().report_error(error(error::sev_bug, filename(), L"BUG_CONFLICT_NO_REDUCE", L"Found a conflict with no reduce actions", m_StartPosition));
            }
            
            // Write out the state and the symbol
            cons().message_stream() << L"  on symbol: " << formatter::to_string(*(*conflict)->token(), *m_Language->grammar(), *m_Language->terminals()) << endl;
            cons().message_stream() << L"  in state: " << formatter::to_string(*m_Parser->machine().state_with_id((*conflict)->state()), *m_Language->grammar(), *m_Language->terminals());
            
            continue;
        }
    }

    // Warn on any clashing guards
    warn_clashing_guards(cons(), m_Language, m_Parser);
    
    // Build an actual AST parser so we can display some stats
    m_Tables = new parser_tables(*m_Parser, m_LexerCompiler->weak_symbols());
    
    // Display some stats
    int totalActions = 0;
    for (int stateId = 0; stateId < m_Tables->count_states(); ++stateId) {
        totalActions += m_Tables->count_actions_for_state(stateId);
    }
    
    cons().verbose_stream() << L"    Number of states in the parser:         " << m_Parser->count_states() << endl;
    cons().verbose_stream() << L"    Total number of parse actions:          " << totalActions << endl;
    cons().verbose_stream() << L"    Average number of actions per state:    " << totalActions / m_Tables->count_states() << endl;
    cons().verbose_stream() << L"    Approximate size of final parse tables: " << m_Tables->size()/1024 << L" kilobytes" << endl;
}

/// \brief Reports errors for a particular reduce conflict (the 'in' and 'to' messages)
void lr_parser_stage::report_reduce_conflict(lr::conflict::reduce_iterator& reduceItem, item_container nonterminal, set<item_container>& displayedNonterminals, int level) {
    // Only display the set for a given target nonterminal once
    if (displayedNonterminals.find(nonterminal) != displayedNonterminals.end()) {
        return;
    }

    // Mark this nonterminal as being displayed (so we won't iterate over it)
    displayedNonterminals.insert(nonterminal);
    
    // For reduce/reduce conflicts, display the context in which the reduction can occur
    for (conflict::possible_reduce_states::const_iterator possibleState = reduceItem->second.begin(); possibleState != reduceItem->second.end(); ++possibleState) {
        // Generate a detail message for this item
        const conflict::lr_item_id& itemId = *possibleState;
        
        // Get the relevant item
        const lr0_item_container& item = (*m_Parser->machine().state_with_id(itemId.state_id))[itemId.item_id];

        // Ignore this item if it's not on the correct nonterminal
        if (item->at_end()) continue;
        if (*item->rule()->items()[item->offset()] != *nonterminal) continue;

        // Work out the rule position for this item
        int             reducedRuleId   = item->rule()->identifier(*m_Language->grammar());
        position        reducedRulePos  = m_Language->rule_definition_pos(reducedRuleId);
        const wstring&  reducedRuleFile = m_Language->rule_definition_file(reducedRuleId);
        
        // Generate a message
        wstringstream detailMessage;
        detailMessage << wstring((level+1)*2, L' ');
        detailMessage << (level==0?L"in: ":L"to: ");
        detailMessage << formatter::to_string(*item, *m_Language->grammar(), *m_Language->terminals());
        
        // Write it out
        cons().report_error(error(error::sev_detail, reducedRuleFile, L"DETAIL_REDUCE_IN", detailMessage.str(), reducedRulePos));
        
        // Display the set for the nonterminals for this rule
        report_reduce_conflict(reduceItem, item->rule()->nonterminal(), displayedNonterminals, level+1);
    }
}
