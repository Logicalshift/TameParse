//
//  rule.h
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <set>
#include <map>
#include <vector>

#ifndef _CONTEXTFREE_RULE_H
#define _CONTEXTFREE_RULE_H

#include "ContextFree/item.h"
#include "Util/container.h"

namespace contextfree {
    /// \brief Forward declaration of grammar
    class grammar;
    
    /// \brief Forward declaration of a rule
    class rule;
    
    /// \brief Class that can contain a reference to a rule
    typedef util::container<rule> rule_container;
    
    /// \brief Set of rules
    typedef std::set<rule_container> rule_set;
    
    /// \brief List of rules
    typedef std::vector<rule_container> rule_list;
    
    /// \brief Maps rules to (something)
    template<typename Value> struct rule_map
    {
        typedef std::map<rule_container, Value> type;
    };
        
    ///
    /// \brief Class representing a rule in a context-free grammar
    ///
    class rule {
    private:
        /// \brief The non-terminal symbol that this rule reduces to
        item_container m_NonTerminal;
        
        /// \brief The items that make up this rule
        item_list m_Items;
        
    private:
        /// \brief The last grammar that identifier() was called for (or NULL)
        mutable const grammar* m_LastGrammar;
        
        /// \brief The identifier that was supplied to this rule in the specified grammar
        ///
        /// We cache the identifier for the purposes of performance: this makes it easy to very quickly
        /// compare rules to see if they're identical or not, or order rules arbitrarily within a grammar.
        mutable int m_Identifier;
        
    public:
        /// \brief Creates a rule (with the empty nonterminal)
        rule();
        
        /// \brief Creates a copy of a rule
        rule(const rule& copyFrom);
        
        /// \brief Creates a copy of a rule with an alternative nonterminal
        rule(const rule& copyFrom, const item_container& nonTerminal);
        
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
        inline item_container nonterminal() const { return m_NonTerminal; }
        
        /// \brief The items in this rule
        inline const item_list& items() const { return m_Items; }
        
        /// \brief First item in this rule
        inline iterator begin() const { return m_Items.begin(); }
        
        /// \brief Last item in this rule
        inline iterator end() const { return m_Items.end(); }
        
        /// \brief Orders this rule relative to another
        bool operator<(const rule& compareTo) const;
        
        /// \brief Determines if this rule is the same as another
        bool operator==(const rule& compareTo) const;
        
        /// \brief Returns the identifier for this rule in the specified grammar
        int identifier(const grammar& gram) const;
        
        /// \brief Clone operator, so this rule can act as part of a container
        inline rule* clone() const { return new rule(*this); }
        
        /// \brief Comparison operator, so this rule can act as part of a container
        static inline bool compare(const rule& a, const rule& b) {
            return a < b;
        }

        /// \brief Comparison operator, so this rule can act as part of a container
        static inline bool compare(const rule* a, const rule* b) {
            if (a == b) return false;
            if (!a)     return true;
            if (!b)     return false;
                
            return compare(*a, *b);
        }

    public:
        /// \brief Appends the specified item to this rule
        rule& operator<<(const item_container& item);
        
        /// \brief Appends the production in the specified rule to this item
        rule& operator<<(const rule& rule);
        
        /// \brief Appends the production in the specified rule to this item
        inline rule& operator<<(const rule_container& rule) { return operator<<(*rule); }
    };
}

#include "ContextFree/grammar.h"

#endif
