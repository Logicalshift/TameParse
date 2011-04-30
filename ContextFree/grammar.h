//
//  grammar.h
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _CONTEXTFREE_GRAMMAR_H
#define _CONTEXTFREE_GRAMMAR_H

#include <map>
#include <string>

#include "ContextFree/rule.h"

namespace contextfree {
    ///
    /// \brief Class representing a context-free grammar
    ///
    class grammar {
    private:
        /// \brief Map of nonterminal identifiers to rules
        typedef std::map<int, rule_set> nonterminal_rule_map;
        
        /// \brief Map of rules to identifiers (filled in on request)
        typedef rule_map<int>::type rule_identifier_map;
        
        /// \brief Class that can map identifiers to their string equivalent
        typedef std::map<int, std::wstring> identifier_to_string;
        
        /// \brief Class that can map strings to the equivalent identifiers
        typedef std::map<std::wstring, int> string_to_identifier;
        
        /// \brief The highest known nonterminal ID
        int m_MaxNonterminal;
        
        /// \brief The nonterminals in this class
        nonterminal_rule_map m_Nonterminals;
        
        /// \brief Maps rules to their identifiers
        mutable rule_identifier_map m_RuleIdentifiers;
        
        /// \brief Nonterminals for the names in this grammar
        string_to_identifier m_NameToNonterminal;
        
        /// \brief Names for the nonterminals in this grammar
        identifier_to_string m_NonterminalToName;
        
    public:
        /// \brief Creates an empty grammar
        grammar();
        
    public:
        /// \brief Returns the rules for the nonterminal with the specified identifier
        rule_set& rules_for_nonterminal(int id);
        
        /// \brief Returns the rules for the nonterminal with the specified name
        rule_set& rules_for_nonterminal(const std::wstring& name);
        
        /// \brief Returns the nonterminal identifier for the specified name
        int id_for_nonterminal(const std::wstring& name);
        
    public:
        /// \brief Returns an identifier given a rule
        int identifier_for_rule(const rule& rule) const;
    };
}

#endif
