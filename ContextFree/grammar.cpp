//
//  grammar.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "grammar.h"

using namespace std;
using namespace contextfree;

/// \brief Creates an empty grammar
grammar::grammar() 
: m_MaxNonterminal(0) {
}

/// \brief Returns the rules for the nonterminal with the specified identifier
rule_set& grammar::rules_for_nonterminal(int id) {
    if (id >= m_MaxNonterminal) m_MaxNonterminal = id + 1;
    
    return m_Nonterminals[id];
}

// The set of 'no rules'
static const rule_set empty_rule_set;

/// \brief Returns the rules for the nonterminal with the specified identifier (or an empty rule set if the nonterminal is not defined)
const rule_set& grammar::rules_for_nonterminal(int id) const {
    // Try to find this identifier
    nonterminal_rule_map::const_iterator found = m_Nonterminals.find(id);
    
    // Use the empty rule set if it couldn't be found
    if (found == m_Nonterminals.end()) return empty_rule_set;
    
    // Return the set that we found
    return found->second;
}

/// \brief Returns the rules for the nonterminal with the specified name
rule_set& grammar::rules_for_nonterminal(const wstring& name) {
    return rules_for_nonterminal(id_for_nonterminal(name));
}


/// \brief Returns the nonterminal identifier for the specified name
int grammar::id_for_nonterminal(const wstring& name) {
    // Look up the name
    string_to_identifier::const_iterator found = m_NameToNonterminal.find(name);
    
    // Generate a new entry if it doesn't exist
    if (found == m_NameToNonterminal.end()) {
        // Use the next available nonterminal ID
        int newIdentifier = m_MaxNonterminal;
        m_MaxNonterminal++;
        
        // Store this rule
        found = m_NameToNonterminal.insert(pair<wstring, int>(name, newIdentifier)).first;
        m_NonterminalToName[newIdentifier] = name;
    }
    
    // Return the ID that we found
    return found->second;
}

/// \brief Returns an identifier given a rule
int grammar::identifier_for_rule(const rule& rule) const {
    // Look up the rule
    rule_identifier_map::const_iterator found = m_RuleIdentifiers.find(rule);
    
    // Return the previous value if one exists
    if (found != m_RuleIdentifiers.end()) return found->second;
    
    // Create a new rule if we haven't set an ID for this one yet
    int newId               = (int) m_RuleIdentifiers.size();
    m_RuleIdentifiers[rule] = newId;
    
    return newId;
}

/// \brief Clears the caches associated with this grammar
///
/// This is necessary after adding new rules or items to this grammar, as doing this will cause things
/// like the cache of FIRST sets to become invalid. It is the responsibility of the object making modifications
/// to make this call at the appropriate time.
void grammar::clear_caches() const {
    m_CachedFirstSets.clear();
}

/// \brief Retrieves the cached value, or calculates the set FIRST(item)
///
/// This is the set of symbols that can appear at the beginning of the specified item. This is calculated recursively,
/// so if the item is a nonterminal, then the set will contain all of the terminals that can begin that item.
///
/// This will call first() on the specified item to retrieve the appropriate first set. While the call is in
/// progress, the first set for the requested item will be set to be 
const item_set& grammar::first(const item& item) const {
    // Try to look up the first set for this item
    item_set_map::const_iterator found = m_CachedFirstSets.find(item);
    
    // Return the value we found if it's already in the cache
    if (found != m_CachedFirstSets.end()) return found->second;
    
    // Not found: create an empty item set so recursive calls to this method terminate
    m_CachedFirstSets[item] = item_set();
    
    // Ask the item for its first set, and update the cache
    m_CachedFirstSets[item] = item.first(*this);
    
    // Return the newly added item (have to do another lookup as map doesn't have a replace operator)
    return m_CachedFirstSets[item];
}
