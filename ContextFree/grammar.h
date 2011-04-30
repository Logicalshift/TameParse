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
        typedef rule_map<int> rule_identifier_map;
        
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
