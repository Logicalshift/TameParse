//
//  precedence_block_rewriter.cpp
//  TameParse
//
//  Created by Andrew Hunter on 22/01/2012.
//  Copyright (c) 2012 Andrew Hunter. All rights reserved.
//

#include "TameParse/Compiler/precedence_block_rewriter.h"

using namespace std;
using namespace contextfree;
using namespace lr;
using namespace language;
using namespace compiler;

/// \brief Construcuts a new rewriter by interpreting a language block
precedence_block_rewriter::precedence_block_rewriter(const contextfree::terminal_dictionary& terminals, const language::precedence_block& precedence) {
    // Precedence of the current item
    int currentPrecedence = 0;

    // Iterate through the items
    for (precedence_block::iterator precedenceItem = precedence.begin(); precedenceItem != precedence.end(); ++precedenceItem) {
        // Increase the precedence of these items
        ++currentPrecedence;

        // Work out the associativity of this item
        associativity assoc = nonassociative;
        switch (precedenceItem->assoc) {
        case precedence_block::left:    assoc = left; break;
        case precedence_block::right:   assoc = right; break;
        default:                        break;
        }

        // Iterate through the items at this level
        typedef precedence_block::ebnf_item_list ebnf_item_list;
        for (ebnf_item_list::const_iterator ebnfItem = precedenceItem->items.begin(); ebnfItem != precedenceItem->items.end(); ++ebnfItem) {
            // Must be a terminal item
            ebnf_item::type itemType = (*ebnfItem)->get_type();
            if (itemType != ebnf_item::ebnf_terminal
                && itemType != ebnf_item::ebnf_terminal_string
                && itemType != ebnf_item::ebnf_terminal_character) {
                continue;
            }

            // Can't deal with items with source identifiers at the moment
            if (!(*ebnfItem)->source_identifier().empty()) {
                continue;
            }

            // Look this item up in the terminal dictionary
            int symbol = terminals.symbol_for_name((*ebnfItem)->identifier());

            // Can't add symbols that don't have an identifier
            if (symbol < 0) continue;

            // Add this symbol
            m_TerminalPrecedence[symbol]    = currentPrecedence;
            m_TerminalAssociativity[symbol] = assoc;
        }
    }
}

/// \brief Creates a clone of this rewriter
action_rewriter* precedence_block_rewriter::clone() const {
    return new precedence_block_rewriter(*this);
}

/// \brief Retrieves the precedence of a particular symbol
///
/// Subclasses should implement this to supply information about symbol
/// precedence. A value of c_NoPrecedence indicates that a symbol does not
/// supply a precedence. Symbols with higher precedence are reduced first.
int precedence_block_rewriter::get_precedence(const contextfree::item_container& terminal, const lr0_item_set& shiftItems) const {
    // Only deal with terminal items
    if (terminal->type() != item::terminal) {
        return precedence_rewriter::no_precedence;
    }

    // Try to fetch the precedence for this symbol
    map<int, int>::const_iterator found = m_TerminalPrecedence.find(terminal->symbol());
    if (found != m_TerminalPrecedence.end()) {
        return found->second;
    } else {
        return precedence_rewriter::no_precedence;
    }
}

/// \brief Retrieves the associativity of a particular symbol
///
/// This is used when trying to disambiguate a shift/reduce conflict where
/// both sides have equal precedence. Note that only the associativity of
/// the rule has an effect on the result; this method is called by the default
/// implementation of get_rule_associativity to determine the associativity of
/// the operator it finds in the rule.
precedence_rewriter::associativity precedence_block_rewriter::get_associativity(const contextfree::item_container& terminal, const lr0_item_set& shiftItems) const {
    // Only deal with terminal items
    if (terminal->type() != item::terminal) {
        return precedence_rewriter::nonassociative;
    }

    // Try to fetch the precedence for this symbol
    map<int, associativity>::const_iterator found = m_TerminalAssociativity.find(terminal->symbol());
    if (found != m_TerminalAssociativity.end()) {
        return found->second;
    } else {
        return precedence_rewriter::nonassociative;
    }
}
