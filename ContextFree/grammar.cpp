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
