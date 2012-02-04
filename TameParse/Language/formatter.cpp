//
//  formatter.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/05/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#include <sstream>

#include "TameParse/Language/formatter.h"

#include "TameParse/Util/astnode.h"

#include "TameParse/ContextFree/grammar.h"
#include "TameParse/ContextFree/item.h"
#include "TameParse/ContextFree/terminal_dictionary.h"

#include "TameParse/Lr/lr_item.h"
#include "TameParse/Lr/lalr_state.h"
#include "TameParse/Lr/lalr_machine.h"
#include "TameParse/Lr/lalr_builder.h"
#include "TameParse/Lr/conflict.h"

using namespace std;
using namespace util;
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
    // Create a stream to build the result up into
    wstringstream res;
    
    res << L"[=> ";
    res << to_string(*nt.get_rule(), gram, dict, -1, false);
    res << L"]";
    
    return res.str();
}

/// \brief Turns an item into a string
wstring formatter::to_string(const contextfree::item& it, const grammar& gram, const terminal_dictionary& dict) {
    // Some item types can be represented quite simply
    switch (it.type()) {
        case item::empty:   return L"[empty]";
        case item::eoi:     return L"$";
        case item::eog:     return L"%";
        default:            break;
    }
    
    // Try to cast the item into the various different types that we know about
    if (it.type() == item::terminal)    return dict.name_for_symbol(it.symbol());
    if (it.type() == item::nonterminal) return gram.name_for_nonterminal(it.symbol());
    
    const guard* guar = it.cast_guard();
    if (guar) return to_string(*guar, gram, dict);
    
    // EBNF items
    wstringstream buf;
    
    const ebnf*                     eb      = it.cast_ebnf();
    
    if (eb && eb->type() == item::alternative) {
        // X | Y | Z form of rule
        buf << L"(";
        
        // Convert all of the rules in this item
        bool first = true;
        for (ebnf::rule_iterator nextRule = eb->first_rule(); nextRule != eb->last_rule(); ++nextRule) {
            // Append the '|'
            buf << L" ";
            if (!first) buf << L"| ";
            
            // Convert the rule to a string
            buf << to_string(**nextRule, gram, dict, -1, false);
            
            // No longer on the first item
            first = false;
        }
        
        buf << L" )";
        
        // This is the result
        return buf.str();
        
    } else if (eb && (eb->type() == item::optional || eb->type() == item::repeat || eb->type() == item::repeat_zero_or_one)) {
        
        // Repeating style rule: first put the contents of the rule in brackets
        buf << L"( " << to_string(*eb->get_rule(), gram, dict, -1, false) << L" )";
        
        // Now add the appropriate character
        if (eb->type() == item::optional)           buf << L"?";
        if (eb->type() == item::repeat)             buf << L"+";
        if (eb->type() == item::repeat_zero_or_one) buf << L"*";
        
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
    
    for (item_set::const_iterator nextItem = it.begin(); nextItem != it.end(); ++nextItem) {
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
    for (rule::iterator nextRule = rule.begin(); nextRule != rule.end(); ++nextRule, ++pos) {
        // Space separates all except the first item
        if (!first)         res << L" ";
        
        // Add the 'dot' (we use '^' to avoid confusing ourselves with the EBNF meaning of the more traditional '*')
        if (dotPos == pos)  res << L"^ ";
        
        // Append this item
        res << to_string(**nextRule, gram, dict);
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
    for (int ntId = 0; ntId < gram.max_item_identifier(); ++ntId) {
        // Add a newline between nonterminals
        if (!addedNewline) {
            res << endl;
            addedNewline = true;
        }
        
        // Get the rules for this nonterminal
        const rule_list& rules = gram.rules_for_nonterminal(ntId);
        
        // Add them to the result
        for (rule_list::const_iterator rule = rules.begin(); rule != rules.end(); ++rule) {
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
    
    for (lookahead_set::const_iterator nextLookahead = la.begin(); nextLookahead != la.end(); ++nextLookahead) {
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
std::wstring formatter::to_string(const lr::lalr_state& state,  const contextfree::grammar& gram, 
                                  const contextfree::terminal_dictionary& dict, bool showClosure) {
    // Create the result as a string stream
    wstringstream res;
    
    // Output all of the items and their lookahead
    bool first = true;
    
    for (lalr_state::iterator nextItem = state.begin(); nextItem != state.end(); ++nextItem) {
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
            for (lookahead_set::const_iterator nextLa = la->begin(); nextLa != la->end(); ++nextLa) {
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
    
    // Generate the closure items if requested
    if (showClosure) {
        // Add a separator between the original text and the closure
        if (!first) {
            res << endl << L"--";
        }
        
        // Generate the closure for this item
        lr1_item_set closure;
        lalr_builder::generate_closure(state, closure, &gram);
        
        // Write them out
        for (lr1_item_set::iterator nextItem = closure.begin(); nextItem != closure.end(); ++nextItem) {
            // Newline between items
            if (!first) {
                res << endl;
            }
            
            // Write out the next item
            res << to_string(**nextItem, gram, dict);
            
            // No longer first
            first = false;
        }
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
            
        case lr_action::act_divert:
            res << L"Divert ";
            break;
            
        case lr_action::act_guard:
            res << L"Guard ";
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
            
        case lr_action::act_shiftstrong:
            res << L"Shift strong equivalent to " << act.next_state() << L" ";
            break;
    }
    
    // Output the symbol this action occurs on
    res << L"on " << to_string(*act.item(), gram, dict);
    
    // For reductions, show the rule being reduced
    switch (act.type()) {
        case lr_action::act_reduce:
        case lr_action::act_weakreduce:
        case lr_action::act_accept:
        case lr_action::act_guard:
            res << L" (" << to_string(*act.rule(), gram, dict) << L")";
            break;
            
        default:
            break;
    }
    
    return res.str();
}

/// \brief Turns a LALR state machine into an enormous string
std::wstring formatter::to_string(const lr::lalr_machine& machine,  const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict, bool showClosure) {
    // Create a stream to hold the result
    wstringstream res;
    
    bool first = true;
    
    // Iterate through the states
    for (lalr_machine::state_iterator nextState = machine.first_state(); nextState != machine.last_state(); ++nextState) {
        if (!first) res << endl << endl;
        
        // Write out the state
        res << L"State #" << (*nextState)->identifier() << endl;
        res << to_string(**nextState, gram, dict, showClosure);
        
        // Write out the transitions
        typedef lalr_machine::transition_set transition_set;
        const transition_set& transitions = machine.transitions_for_state((*nextState)->identifier());
        
        if (!transitions.empty()) {
            res << endl;
            for (transition_set::const_iterator transit = transitions.begin(); transit != transitions.end(); ++transit) {
                res << endl << to_string(*transit->first, gram, dict) << L" -> " << transit->second;
            }
        }
        
        first = false;
    }
    
    return res.str();
}

/// \brief Turns a LALR builder into an enormous string
std::wstring formatter::to_string(const lr::lalr_builder& builder, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict, bool showClosure) {
    // Create a stream to hold the result
    wstringstream res;
    
    bool first = true;
    
    // Iterate through the states
    for (lalr_machine::state_iterator nextState = builder.machine().first_state(); nextState != builder.machine().last_state(); ++nextState) {
        if (!first) res << endl << endl;
        
        // Write out the state
        res << L"State #" << (*nextState)->identifier() << endl;
        res << to_string(**nextState, gram, dict, showClosure);
        
        // Write out the actions
        const lr_action_set& actions = builder.actions_for_state((*nextState)->identifier());
        
        if (!actions.empty()) {
            res << endl;
            for (lr_action_set::const_iterator action = actions.begin(); action != actions.end(); ++action) {
                res << endl << to_string(**action, gram, dict);
            }
        }
        
        first = false;
    }
    
    return res.str();
}

/// \brief Turns a LALR conflict into a string description
wstring formatter::to_string(const conflict& conf, const grammar& gram, const terminal_dictionary& dict) {
    // Create a stream to write the result to
    wstringstream res;
    
    // This is a reduce/reduce conflict unless there is at least one shift action in the conflict
    bool isShiftReduce;
    
    if (conf.first_shift_item() == conf.last_shift_item()) {
        res << L"Reduce/reduce conflict";
        isShiftReduce = false;
    } else {
        res << L"Shift/reduce conflict";
        isShiftReduce = true;
    }
    
    // Write out the state that this conflict occurred in
    res << L" in state " << conf.state();
    
    // And the token the conflict occurs on
    res << L" on " << to_string(*conf.token(), gram, dict);
    
    // Write out the shift actions, if there are any
    bool first = true;
    for (lr0_item_set::const_iterator shiftItem = conf.first_shift_item(); shiftItem != conf.last_shift_item(); ++shiftItem) {
        // Indicate these are shift items if this is the first
        res << endl;
        if (first) {
            res << L"  Shift:" << endl;
        }
        
        // Write out this item
        res << L"    " << to_string(**shiftItem, gram, dict);
        
        // No longer first
        first = false;
    }
    
    // Write out the reduce actions, if there are any
    if (isShiftReduce) {
        res << endl << L"  Or reduce:";
    } else {
        res << endl << L"  Reduce:";
    }
    
    for (conflict::reduce_iterator reduceItem = conf.first_reduce_item(); reduceItem != conf.last_reduce_item(); ++reduceItem) {
        // Write out the item that's being reduced
        res << endl;
        res << L"    " << to_string(*reduceItem->first, gram, dict);
        
        // TODO: write out the states that can be reached by this reduction (these aren't worked out at the moment)
    }
    
    // Produce the final string
    return res.str();
}

/// \brief Turns a AST node into a string description
std::wstring formatter::to_string(const util::astnode& node, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict) {
    // Begin building the result
    wstringstream res;
    
    // Write out this node
    if (node.item_identifier() >= 0) {
        const item_container& thisItem  = gram.item_with_identifier(node.item_identifier());
        
        res << to_string(*thisItem, gram, dict);
        if (node.lexeme().item()) {
            res << L" " << dict.name_for_symbol(node.lexeme()->matched()) << L" \"" << node.lexeme()->content<wchar_t>() << L"\"";
        }
    } else if (node.lexeme().item()) {
        res << dict.name_for_symbol(node.lexeme()->matched()) << L" \"" << node.lexeme()->content<wchar_t>() << L"\"";
    } else {
        res << L"(Unknown symbol)";
    }
    
    // Write out the children for this node
    for (astnode::node_list::const_iterator child = node.children().begin(); child != node.children().end(); ++child) {
        // Get the conversion for this child
        wstring childString = to_string(**child, gram, dict);
        
        // True if there are more children after this one
        bool moreChildren = (child + 1) != node.children().end();
        
        // Append to the result (indenting as we go)
        res << endl << L"+- ";
        for (wstring::iterator nextChar = childString.begin(); nextChar != childString.end(); ++nextChar) {
            res << *nextChar;
            if (*nextChar == L'\n') {
                if (moreChildren) {
                    res << L"|  ";
                } else {
                    res << L"   ";
                }
            }
        }
    }
    
    // Return this as the result
    return res.str();
}
