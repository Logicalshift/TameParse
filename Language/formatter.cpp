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

using namespace std;
using namespace contextfree;
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
        if (rep)    buf << L"*";
        if (repOpt) buf << L"+";
        
        // Done
        return buf.str();
    }
    
    // Unknown item type
    return L"???";
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
