//
//  rule.h
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _CONTEXTFREE_RULE_H
#define _CONTEXTFREE_RULE_H

#include "ContextFree/item.h"

namespace contextfree {
    /// \brief Forward declaration of grammar
    class grammar;
    
    ///
    /// \brief Class representing a rule in a context-free grammar
    ///
    class rule {
    private:
        /// \brief The non-terminal symbol that this rule reduces to
        item_container m_NonTerminal;
        
        /// \brief The items that make up this rule
        item_list m_Items;
        
    public:
        /// \brief Creates a copy of a rule
        rule(const rule& copyFrom);
        
        /// \brief Creates an empty rule, which reduces to the specified item
        rule(const item_container& nonTerminal);
        
        /// \brief Creates an empty rule with a nonterminal identifier
        rule(const int nonTerminal);
        
        /// \brief Copies the content of a rule into this one
        rule& operator=(const rule& copyFrom);
        
        /// \brief Iterator class for inspecting the items that make up this rule
        typedef item_list::const_iterator iterator;
        
        /// \brief The nonterminal that this rule reduces to
        ///
        /// (Rules can be set to reduce to any kind of item, but only nonterminals are useful by default)
        inline item_container nonterminal() { return m_NonTerminal; }
        
        /// \brief First item in this rule
        inline iterator begin() const { return m_Items.begin(); }
        
        /// \brief Last item in this rule
        inline iterator end() const { return m_Items.end(); }
        
    public:
        /// \brief Appends the specified item to this rule
        rule& operator<<(const item_container& item);
    };
}

#endif

#include "ContextFree/grammar.h"
