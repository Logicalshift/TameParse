//
//  formatter.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <sstream>

#include "formatter.h"

#include "ContextFree/grammar.h"
#include "ContextFree/item.h"
#include "ContextFree/terminal_dictionary.h"

#include "Lr/lr_item.h"
#include "Lr/lalr_state.h"
#include "Lr/lalr_machine.h"
#include "Lr/lalr_builder.h"

using namespace std;
using namespace contextfree;
using namespace lr;
using namespace language;

/// \brief Turns a terminal into a string
wstring formatter::to_string(const terminal& term, const terminal_dictionary& dict) {
    // Just look up the terminal name in the dictionary
    return dict.name_for_symbol(term.symbol());
}

/// \brief Turns a nonterminal into a string
wstring formatter::to_string(const contextfree::nonterminal& nt, const contextfree::grammar& gram) {
    return gram.name_for_nonterminal(nt.symbol());
}

/// \brief Turns a guard item into a string
wstring formatter::to_string(const guard& nt, const grammar& gram, const terminal_dictionary& dict) {
    // TODO: implement me
    return L"[GUARD]";
}

/// \brief Turns an item into a string
wstring formatter::to_string(const contextfree::item& it, const grammar& gram, const terminal_dictionary& dict) {
    // Some item types can be represented quite simply
    switch (it.type()) {
        case item::empty:   return L"[empty]";
        case item::eoi:     return L"$";
        default:            break;
    }
    
    // Try to cast the item into the various different types that we know about
    const terminal* term = dynamic_cast<const terminal*>(&it);
    if (term) return to_string(*term, dict);
    
    const nonterminal* nonterm = dynamic_cast<const nonterminal*>(&it);
    if (nonterm) return to_string(*nonterm, gram);
    
    const guard* guar = dynamic_cast<const guard*>(&it);
    if (guar) return to_string(*guar, gram, dict);
    
    // EBNF items
    wstringstream buf;
    
    const ebnf*                     eb      = dynamic_cast<const ebnf*>(&it);
    const ebnf_alternate*           alt     = dynamic_cast<const ebnf_alternate*>(&it);
    const ebnf_optional*            opt     = dynamic_cast<const ebnf_optional*>(&it);
    const ebnf_repeating*           rep     = dynamic_cast<const ebnf_repeating*>(&it);
    const ebnf_repeating_optional*  repOpt  = dynamic_cast<const ebnf_repeating_optional*>(&it);
    
    if (alt) {
        // X | Y | Z form of rule
        buf << L"(";
        
        // Convert all of the rules in this item
        bool first = true;
        for (ebnf::rule_iterator it = alt->first_rule(); it != alt->last_rule(); it++) {
            // Append the '|'
            buf << L" ";
            if (!first) buf << L"| ";
            
            // Convert the rule to a string
            buf << to_string(**it, gram, dict, -1, false);
            
            // No longer on the first item
            first = false;
        }
        
        buf << L" )";
        
        // This is the result
        return buf.str();
        
    } else if (eb && (opt || rep || repOpt)) {
        
        // Repeating style rule: first put the contents of the rule in brackets
        buf << L"( " << to_string(*eb->get_rule(), gram, dict, -1, false) << L" )";
        
        // Now add the appropriate character
        if (opt)    buf << L"?";
        if (rep)    buf << L"+";
        if (repOpt) buf << L"*";
        
        // Done
        return buf.str();
    }
    
    // Unknown item type
    return L"???";
}

/// \brief Turns an item set into a string
std::wstring formatter::to_string(const contextfree::item_set& it, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict) {
    // Start creating the result
    wstringstream res;
    
    res << L"(";
    
    // All but the first item are preceeded by commas
    bool isFirst = true;
    
    for (item_set::const_iterator nextItem = it.begin(); nextItem != it.end(); nextItem++) {
        // Add commas
        if (!isFirst) {
            res << L", ";
        }
        
        // Add the next item
        res << to_string(**nextItem, gram, dict);
        
        // Next item isn't the first any more
        isFirst = false;
    }
    
    // Finish up the string
    res << L")";
    return res.str();
}

/// \brief Turns a rule into a string, with an optional dot position
wstring formatter::to_string(const rule& rule, const grammar& gram, const terminal_dictionary& dict, int dotPos, bool showNonterminal) {
    // Create the stream where we'll put the result
    wstringstream res;
    
    // Add the nonterminal if requested
    if (showNonterminal) {
        res << to_string(*rule.nonterminal(), gram, dict) << L" = ";
    }
    
    // Append the rest of the rule
    bool first = true;
    int pos = 0;
    for (rule::iterator it = rule.begin(); it != rule.end(); it++, pos++) {
        // Space separates all except the first item
        if (!first)         res << L" ";
        
        // Add the 'dot' (we use '^' to avoid confusing ourselves with the EBNF meaning of the more traditional '*')
        if (dotPos == pos)  res << L"^ ";
        
        // Append this item
        res << to_string(**it, gram, dict);
        first = false;
    }
    
    // The 'dot' can appear at the end of the rule as well
    if (dotPos == pos) {
        if (!first) res << L" ";
        res << L"^";
    }
    
    // Return this as the result
    return res.str();
}

/// \brief Turns a grammar into a (large) string
wstring formatter::to_string(const grammar& gram, const terminal_dictionary& dict) {
    // Create the result
    wstringstream res;

    // Flag that indicates if we've previously added a newline
    bool addedNewline = true;

    // Iterate through the nonterminals in this grammar
    for (int ntId = 0; ntId < gram.max_nonterminal(); ntId++) {
        // Add a newline between nonterminals
        if (!addedNewline) {
            res << endl;
            addedNewline = true;
        }
        
        // Get the rules for this nonterminal
        const rule_list& rules = gram.rules_for_nonterminal(ntId);
        
        // Add them to the result
        for (rule_list::const_iterator rule = rules.begin(); rule != rules.end(); rule++) {
            res << to_string(**rule, gram, dict, -1, true) << endl;
            addedNewline = false;
        }
    }
    
    // Convert to a string
    return res.str();
}

/// \brief Turns a LR(0) item into a string
std::wstring formatter::to_string(const lr::lr0_item& item, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict) {
    return to_string(*item.rule(), gram, dict, item.offset(), true);
}

/// \brief Turns a LR(1) item into a string
std::wstring formatter::to_string(const lr::lr1_item& item, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict) {
    wstringstream res;
    
    // Write out the LR(0) item
    const lr0_item& lr0item = item;
    res << to_string(lr0item, gram, dict);
    
    // Write out the lookahead
    typedef lr1_item::lookahead_set lookahead_set;
    const lookahead_set& la = item.lookahead();
    bool first = true;
    
    for (lookahead_set::const_iterator nextLookahead = la.begin(); nextLookahead != la.end(); nextLookahead++) {
        if (first) {
            res << L" [";
        } else {
            res << L", ";
        }
        
        res << to_string(**nextLookahead, gram, dict);
        
        first = false;
    }
    
    if (!first) {
        res << L"]";
    }
    
    return res.str();
}

/// \brief Turns a LALR state into a string
std::wstring formatter::to_string(const lr::lalr_state& state,  const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict) {
    // Create the result as a string stream
    wstringstream res;
    
    // Output all of the items and their lookahead
    bool first = true;
    
    for (lalr_state::all_iterator nextItem = state.begin(); nextItem != state.end(); nextItem++) {
        // Insert newlines
        if (!first) {
            res << endl;
        }
        
        // Append the LR(0) item
        res << to_string(**nextItem, gram, dict);
        
        // Append the lookahead, if any
        typedef lr1_item::lookahead_set lookahead_set;
        const lookahead_set* la = state.lookahead_for(*nextItem);
        
        // Only append lookahead if there is more than one item
        if (la && !la->empty()) {
            res << L" (";
            
            // Append each LA item in turn
            bool firstLookahead = true;
            for (lookahead_set::const_iterator nextLa = la->begin(); nextLa != la->end(); nextLa++) {
                if (!firstLookahead) {
                    res << L", ";
                }
                res << to_string(**nextLa, gram, dict);
                firstLookahead = false;
            }
            
            res << L")";
        }
        
        // No longer first
        first = false;
    }
    
    return res.str();
}

/// \brief Turns a LR action into a string
std::wstring formatter::to_string(const lr::lr_action& act, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict) {
    // Create a stream to contain the result
    wstringstream res;
    
    // Output the action name
    switch (act.type()) {
        case lr_action::act_accept:
            res << L"Accept ";
            break;
            
        case lr_action::act_goto:
            res << L"Goto " << act.next_state() << L" ";
            break;
            
        case lr_action::act_ignore:
            res << L"Ignore ";
            break;
            
        case lr_action::act_reduce:
            res << L"Reduce ";
            break;
            
        case lr_action::act_weakreduce:
            res << L"Weak reduce ";
            break;
            
        case lr_action::act_shift:
            res << L"Shift to " << act.next_state() << L" ";
            break;
    }
    
    // Output the symbol this action occurs on
    res << L"on " << to_string(*act.item(), gram, dict);
    
    // For reductions, show the rule being reduced
    switch (act.type()) {
        case lr_action::act_reduce:
        case lr_action::act_weakreduce:
            res << L" (" << to_string(*act.rule(), gram, dict) << L")";
            break;
            
        default:
            break;
    }
    
    return res.str();
}

/// \brief Turns a LALR state machine into an enormous string
std::wstring formatter::to_string(const lr::lalr_machine& machine,  const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict) {
    // Create a stream to hold the result
    wstringstream res;
    
    bool first = true;
    
    // Iterate through the states
    for (lalr_machine::state_iterator nextState = machine.first_state(); nextState != machine.last_state(); nextState++) {
        if (!first) res << endl << endl;
        
        // Write out the state
        res << L"State #" << (*nextState)->identifier() << endl;
        res << to_string(**nextState, gram, dict);
        
        // Write out the transitions
        typedef lalr_machine::transition_set transition_set;
        const transition_set& transitions = machine.transitions_for_state((*nextState)->identifier());
        
        if (!transitions.empty()) {
            res << endl;
            for (transition_set::const_iterator transit = transitions.begin(); transit != transitions.end(); transit++) {
                res << endl << to_string(*transit->first, gram, dict) << L" -> " << transit->second;
            }
        }
        
        first = false;
    }
    
    return res.str();
}

/// \brief Turns a LALR builder into an enormous string
std::wstring formatter::to_string(const lr::lalr_builder& builder, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict) {
    // Create a stream to hold the result
    wstringstream res;
    
    bool first = true;
    
    // Iterate through the states
    for (lalr_machine::state_iterator nextState = builder.machine().first_state(); nextState != builder.machine().last_state(); nextState++) {
        if (!first) res << endl << endl;
        
        // Write out the state
        res << L"State #" << (*nextState)->identifier() << endl;
        res << to_string(**nextState, gram, dict);
        
        // Write out the actions
        const lr_action_set& actions = builder.actions_for_state((*nextState)->identifier());
        
        if (!actions.empty()) {
            res << endl;
            for (lr_action_set::const_iterator action = actions.begin(); action != actions.end(); action++) {
                res << endl << to_string(**action, gram, dict);
            }
        }
        
        first = false;
    }
    
    return res.str();
}
