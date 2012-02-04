//
//  rule.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#include "TameParse/ContextFree/rule.h"
#include "TameParse/ContextFree/standard_items.h"

using namespace contextfree;

/// \brief Creates a rule (with the empty nonterminal)
rule::rule() 
: m_NonTerminal(new empty_item(), true)
, m_LastGrammar(NULL) {
}

/// \brief Creates a copy of a rule
rule::rule(const rule& copyFrom)
: m_NonTerminal(copyFrom.m_NonTerminal)
, m_Items(copyFrom.m_Items)
, m_ItemKeys(copyFrom.m_ItemKeys)
, m_LastGrammar(NULL) {
}

/// \brief Creates a copy of a rule with an alternative nonterminal
rule::rule(const rule& copyFrom, const item_container& nonTerminal)
: m_NonTerminal(nonTerminal)
, m_Items(copyFrom.m_Items)
, m_ItemKeys(copyFrom.m_ItemKeys)
, m_LastGrammar(NULL) {
}

/// \brief Creates an empty rule, which reduces to the specified item
rule::rule(const item_container& nonTerminal)
: m_NonTerminal(nonTerminal)
, m_LastGrammar(NULL) {
}

/// \brief Creates an empty rule with a nonterminal identifier
rule::rule(const int newNt)
: m_NonTerminal(new contextfree::nonterminal(newNt), true)
, m_LastGrammar(NULL) {
}

/// \brief Copies the content of a rule into this one
rule& rule::operator=(const rule& copyFrom) {
    m_NonTerminal   = copyFrom.m_NonTerminal;
    m_Items         = copyFrom.m_Items;
    m_ItemKeys      = copyFrom.m_ItemKeys;
    m_LastGrammar   = NULL;
    
    return *this;
}

/// \brief Orders this rule relative to another by comparing only the items
int rule::compare_items(const rule& compareTo) const {
    // Number of items is the fastest thing to compare, so check that first
    if (m_Items.size() < compareTo.m_Items.size())  return -1;
    if (m_Items.size() > compareTo.m_Items.size())  return 1;

    // Need to compare each item in turn
    item_list::const_iterator ourItem   = m_Items.begin();
    item_list::const_iterator theirItem = compareTo.begin();
    
    for (;ourItem != m_Items.end() && theirItem != compareTo.end(); ++ourItem, ++theirItem) {
        if (*ourItem < *theirItem) return -1;
        if (*theirItem < *ourItem) return 1;
    }
    
    return 0;
}

/// \brief Orders this rule relative to another
bool rule::operator<(const rule& compareTo) const {
    // Never less than ourselves
    if (this == &compareTo) return false;
    
    // Number of items is the fastest thing to compare, so check that first
    if (m_Items.size() < compareTo.m_Items.size())  return true;
    if (m_Items.size() > compareTo.m_Items.size())  return false;
    
    // The nonterminal should be reasonably fast to compare
    if (m_NonTerminal < compareTo.m_NonTerminal)    return true;
    if (compareTo.m_NonTerminal < m_NonTerminal)    return false;

    // Keys aren't compared at the moment, but perhaps they should be
    
    // Compare the items
    return compare_items(compareTo) < 0;
}

/// \brief Determines if this rule is the same as another
bool rule::operator==(const rule& compareTo) const {
    // We always equal ourselves
    if (this == &compareTo) return true;
    
    // Number of items is the fastest thing to compare, so check that first
    if (m_Items.size() != compareTo.m_Items.size()) return false;
    
    // The nonterminal should be reasonably fast to compare
    if (m_NonTerminal != compareTo.m_NonTerminal)   return false;
    
    // Need to compare each item in turn
    item_list::const_iterator ourItem   = m_Items.begin();
    item_list::const_iterator theirItem = compareTo.begin();
    
    for (;ourItem != m_Items.end() && theirItem != compareTo.end(); ++ourItem, ++theirItem) {
        if (*ourItem != *theirItem) return false;
    }
    
    return true;
}

/// \brief Appends the specified item to this rule
rule& rule::operator<<(const item_container& item) {
    // Add this item to the list of items
    m_Items.push_back(item);
    m_ItemKeys.push_back(0);
    
    return *this;
}


/// \brief Appends the production in the specified rule to this item
rule& rule::operator<<(const rule& rule) {
    for (iterator sym = rule.begin(); sym != rule.end(); ++sym) {
        m_Items.push_back(*sym);
        m_ItemKeys.push_back(rule.get_key(sym));
    }
    return *this;
}

/// \brief Returns the identifier for this rule in the specified grammar
int rule::identifier(const grammar& gram) const {
    // DANGER WILL ROBINSON:
    //   If a grammar is freed, then a new grammar created and this rule put into it, there is a non-zero chance
    //   of m_LastGrammar being wrong!
    //
    //   OK for now. Rules are usually one-shot affairs.
    if (&gram == m_LastGrammar) return m_Identifier;
    
    m_LastGrammar = &gram;
    return m_Identifier = gram.identifier_for_rule(*this);
}

/// \brief Retrieves the key associated with the item at the specified index
///
/// Each item in a rule has an associated key which can be used to retrieve
/// information associated with it. Keys are set to 0 by default.
int rule::get_key(size_t offset) const {
    return m_ItemKeys[offset];
}

/// \brief Sets the key associated with the item at the specified index
void rule::set_key(size_t offset, int key) {
    m_ItemKeys[offset] = key;
}
