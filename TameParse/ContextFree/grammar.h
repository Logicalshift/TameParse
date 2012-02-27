//
//  grammar.h
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the \"Software\"), to 
//  deal in the Software without restriction, including without limitation the 
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
//  sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
//  IN THE SOFTWARE.
//

#ifndef _CONTEXTFREE_GRAMMAR_H
#define _CONTEXTFREE_GRAMMAR_H

#include <map>
#include <string>

#include "TameParse/Util/container.h"

#include "TameParse/ContextFree/item.h"
#include "TameParse/ContextFree/rule.h"

namespace lr {
    /// \brief Forward declaration of an LR(1) item
    class lr1_item;
    
    /// \brief Container for an LR(1) item
    typedef util::container<lr1_item> lr1_item_container;

    /// \brief Forward declaration of a LR(1) item set
    class lr1_item_set;
}

namespace contextfree {
    /// \brief Forward declaration of an item type
    class nonterminal;
    
    ///
    /// \brief Class representing a context-free grammar
    ///
    class grammar {
    public:
        /// \brief Map of nonterminal identifiers to rules
        typedef std::map<int, rule_list> nonterminal_rule_map;
        
        /// \brief Map of rules to identifiers (filled in on request)
        typedef rule_map<int>::type rule_identifier_map;
        
        /// \brief Map of identifiers to rules
        typedef std::map<int, rule_container> identifier_rule_map;
        
        /// \brief Class that can map identifiers to their string equivalent
        typedef std::map<int, std::wstring> identifier_to_string;
        
        /// \brief Class that can map strings to the equivalent identifiers
        typedef std::map<std::wstring, int> string_to_identifier;
        
        /// \brief Item to list of items map, used for things like first and follow sets
        typedef item_map<item_set>::type item_set_map;
        
        /// \brief Map of items to identifiers
        typedef item_map<int>::type item_identifier_map;
        
        /// \brief Map of identifiers to items
        typedef std::map<int, item_container> identifier_item_map;

        /// \brief Map of item identifiers to LR(1) item sets
        typedef std::map<int, lr::lr1_item_set*> lr1_item_set_cache;
        
    private:
        /// \brief The nonterminals in this class
        nonterminal_rule_map m_Nonterminals;
        
        /// \brief Nonterminals for the names in this grammar
        string_to_identifier m_NameToNonterminal;
        
        /// \brief Names for the nonterminals in this grammar
        identifier_to_string m_NonterminalToName;
        
        /// \brief Maps rules to their identifiers
        mutable rule_identifier_map m_RuleIdentifiers;
        
        mutable identifier_rule_map m_RuleForIdentifier;
        mutable item_identifier_map m_ItemIdentifiers;
        mutable identifier_item_map m_ItemForIdentifier;
        
        /// \brief Cached map of FIRST sets for this grammar
        mutable item_set_map m_CachedFirstSets;
        
        /// \brief Cached map of FOLLOW sets for this grammar
        mutable item_set_map m_CachedFollowSets;

        /// \brief Cached item sets
        mutable lr1_item_set_cache m_CachedItemSets;
        
        /// \brief Item set containing 'epsilon', representing an empty first or follow set
        item_set* m_EpsilonSet;

    public:
        /// \brief Creates an empty grammar
        grammar();
        
        /// \brief Destructor
        ~grammar();
        
        /// \brief Assignment
        grammar& operator=(const grammar& assignFrom);
        
    public:
        /// \brief Returns the first unused item ID
        inline int max_item_identifier() const { return (int)m_ItemIdentifiers.size(); }
        
        /// \brief Returns the rules for the nonterminal with the specified identifier
        rule_list& rules_for_nonterminal(int id);
        
        /// \brief Returns the rules for the nonterminal with the specified identifier (or an empty rule set if the nonterminal is not defined)
        const rule_list& rules_for_nonterminal(int id) const;
        
        /// \brief Returns the rules for the nonterminal with the specified name
        rule_list& rules_for_nonterminal(const std::wstring& name);
        
        /// \brief Returns the rules for the nonterminal with the specified name
        inline rule_list& operator[](std::wstring& nonterminalName) { return rules_for_nonterminal(nonterminalName); }
        
        /// \brief Returns the nonterminal identifier for the specified name
        int id_for_nonterminal(const std::wstring& name);
        
        /// \brief Returns the name for the nonterminal with the specified identifier
        inline std::wstring name_for_nonterminal(int id) const {
            identifier_to_string::const_iterator found = m_NonterminalToName.find(id);
            if (found != m_NonterminalToName.end()) return found->second;
            return L"";
        }
        
        /// \brief Returns an item container for the nonterminal with the specified name
        item_container get_nonterminal(const std::wstring& name);

        /// \brief Returns true if a nonterminal is defined with the specified name
        bool nonterminal_is_defined(const std::wstring& name);
        
    public:
        /// \brief Returns the first unused rule identifier
        ///
        /// After building a parser, this can be used to enumerate all of the rules that were generated for the
        /// grammar (including the implicit ones that were generated from EBNF items)
        inline int max_rule_identifier() const { return (int) m_RuleIdentifiers.size(); }
        
        /// \brief Returns an identifier given a rule. Identifiers are numbered from 0.
        int identifier_for_rule(const rule_container& rule) const;
        
        /// \brief Returns the identifier for a particular rule
        const rule_container& rule_with_identifier(int id) const;
        
        /// \brief Returns an identifier given an item. Identifiers are numbered from 0.
        int identifier_for_item(const item_container& item) const;
        
        /// \brief Returns the item that has the specified identifier
        const item_container& item_with_identifier(int id) const;
        
    public:
        /// \brief Clears the caches associated with this grammar
        ///
        /// This is necessary after adding new rules or items to this grammar, as doing this will cause things
        /// like the cache of FIRST sets to become invalid. It is the responsibility of the object making modifications
        /// to make this call at the appropriate time.
        void clear_caches() const;

        /// \brief Returns the cached LR(1) item set for the item with the specified ID
        ///
        /// This will be empty after the cache has been cleared.
        lr::lr1_item_set& cached_set_for_item(int id) const;
        
        /// \brief Retrieves the cached value, or calculates the set FIRST(item)
        ///
        /// This is the set of symbols that can appear at the beginning of the specified item. This is calculated recursively,
        /// so if the item is a nonterminal, then the set will contain all of the terminals that can begin that item.
        ///
        /// This will call first() on the specified item to retrieve the appropriate first set. While the call is in
        /// progress, the first set for the requested item will be set to be empty.
        const item_set& first(const item_container& item) const;
        
        /// \brief Computes the follow set for the item with the specified identifier
        ///
        /// This is the set of symbols that can follow a particular item, in any position in the grammar.
        /// For performance reasons, terminal items are excluded from this set (they will always have an empty follow set)
        const item_set& follow(const item_container& item) const;
        
        /// \brief Computes the first set for the specified rule (or retrieves the cached version)
        ///
        /// The returned set will contain an empty (epsilon) item if the rule can have no symbols in it.
        item_set first_for_rule(const rule& rule) const;
        
    private:
        /// \brief Updates the follow set cache using the content of a particular rule
        void fill_follow(const rule& rule, item_map<item_set>::type& dependencies) const;
        
    public:
        ///
        /// \brief Class for building rules
        ///
        class builder {
        private:
            friend class grammar;
            
            /// \brief The grammar that this rule belongs to
            grammar& m_Grammar;
            
            /// \brief The rule that this is building
            rule& m_Target;
            
            inline builder(rule& rule, grammar& gram) 
            : m_Target(rule)
            , m_Grammar(gram) { }
            
        public:
            /// \brief Appends an item to the rule that this is building
            builder& operator<<(const item& item);
            
            /// \brief Appends an item to the rule that this is building
            builder& operator<<(const item_container& item);
            
            /// \brief Appends a new nonterminal item to the rule that this building
            builder& operator<<(const std::wstring& nonterminal);
            
            /// \brief Appends a new terminal item to the rule that this is building
            builder& operator<<(int terminal);
        };
        
        /// \brief Begins defining a new rule for the nonterminal with the specified name
        builder operator+=(const std::wstring& newNonterminal);
        
        /// \brief Begins defining a new rule for the specified nonterminal
        builder operator+=(const nonterminal& newNonterminal);
    };
}

#include "TameParse/Lr/lr_item.h"

#endif
