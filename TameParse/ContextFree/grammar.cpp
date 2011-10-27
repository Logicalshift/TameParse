//
//  grammar.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/ContextFree/grammar.h"
#include "TameParse/Lr/lr_item.h"

using namespace std;
using namespace contextfree;

// The set of 'no rules'
static const rule_list empty_rule_set;

// An empty item
static empty_item an_empty_item;

// An item container with an empty item in it
static item_container an_empty_item_c(&an_empty_item, false);

/// \brief Creates an empty grammar
grammar::grammar() {
	m_EpsilonSet = new item_set(this);
    m_EpsilonSet->insert(an_empty_item_c);
}

/// \brief Destroys a grammar
grammar::~grammar() {
    // Finished with the epsilon set
    delete m_EpsilonSet;
    
    // Finished with the list of cached item sets
    for (lr1_item_set_cache::iterator oldItemSets = m_CachedItemSets.begin(); oldItemSets != m_CachedItemSets.end(); oldItemSets++) {
        delete oldItemSets->second;
    }
}

/// \brief Assignment
grammar& grammar::operator=(const grammar& assignFrom) {
    // Nothing to do if we're assigning to ourselves
    if (&assignFrom == this) return *this;
    
    // Copy the grammar
    m_Nonterminals      = assignFrom.m_Nonterminals;
    m_NameToNonterminal = assignFrom.m_NameToNonterminal;
    m_NonterminalToName = assignFrom.m_NonterminalToName;

    // Preserve the item and rule identifiers
    m_ItemIdentifiers   = assignFrom.m_ItemIdentifiers;
    m_ItemForIdentifier = assignFrom.m_ItemForIdentifier;
    m_RuleIdentifiers   = assignFrom.m_RuleIdentifiers;
    
    // Clear the caches for this object
    m_CachedFirstSets.clear();
    m_CachedFollowSets.clear();
    m_CachedItemSets.clear();
    
    // Finished
    return *this;
}

/// \brief Returns the rules for the nonterminal with the specified identifier
rule_list& grammar::rules_for_nonterminal(int id) {
    return m_Nonterminals[id];
}

/// \brief Returns the rules for the nonterminal with the specified identifier (or an empty rule set if the nonterminal is not defined)
const rule_list& grammar::rules_for_nonterminal(int id) const {
    // Try to find this identifier
    nonterminal_rule_map::const_iterator found = m_Nonterminals.find(id);
    
    // Use the empty rule set if it couldn't be found
    if (found == m_Nonterminals.end()) return empty_rule_set;
    
    // Return the set that we found
    return found->second;
}

/// \brief Returns the rules for the nonterminal with the specified name
rule_list& grammar::rules_for_nonterminal(const wstring& name) {
    return rules_for_nonterminal(id_for_nonterminal(name));
}

/// \brief Returns the nonterminal identifier for the specified name
int grammar::id_for_nonterminal(const wstring& name) {
    // Look up the name
    string_to_identifier::const_iterator found = m_NameToNonterminal.find(name);
    
    // Generate a new entry if it doesn't exist
    if (found == m_NameToNonterminal.end()) {
        // Use the next available item ID as the identifier of the new nonterminal
        // (It's technically possible to have nonterminals with arbitrary IDs, but things make a lot more sense when the 
        // nonterminal IDs match with other item IDs ['nonterminal' is a bit confusing in and of itself, as the EBNF items
        // are all also technically nonterminals, as are guards. We also use the empty symbol as a placeholder occasionally])
        int newIdentifier = (int) m_ItemIdentifiers.size();
        
        // Store as an item
        item_container newNonterminal(new nonterminal(newIdentifier), true);
        m_ItemIdentifiers[newNonterminal]   = newIdentifier;
        m_ItemForIdentifier[newIdentifier]  = newNonterminal;
        
        // Store this rule
        found = m_NameToNonterminal.insert(pair<wstring, int>(name, newIdentifier)).first;
        m_NonterminalToName[newIdentifier] = name;
    }
    
    // Return the ID that we found
    return found->second;
}

/// \brief Returns true if a nonterminal is defined with the specified name
bool grammar::nonterminal_is_defined(const std::wstring& name) {
    return m_NameToNonterminal.find(name) != m_NameToNonterminal.end();
}

/// \brief Returns an item container for the nonterminal with the specified name
item_container grammar::get_nonterminal(const std::wstring& name) {
    return item_container(new nonterminal(id_for_nonterminal(name)), true);
}

/// \brief Returns an identifier given a rule
int grammar::identifier_for_rule(const rule_container& rule) const {
    // Look up the rule
    rule_identifier_map::const_iterator found = m_RuleIdentifiers.find(rule);
    
    // Return the previous value if one exists
    if (found != m_RuleIdentifiers.end()) return found->second;
    
    // Create a new rule if we haven't set an ID for this one yet
    int newId                   = (int) m_RuleIdentifiers.size();
    m_RuleIdentifiers[rule]     = newId;
    m_RuleForIdentifier[newId]  = rule;
    
    return newId;
}

/// \brief Returns the rule for a particular identifier
const rule_container& grammar::rule_with_identifier(int id) const {
    // Empty rule that we use when we fail
    static rule             empty_rule(an_empty_item);
    static rule_container   empty_rule_container(&empty_rule, false);
    
    // Try to find this ID
    identifier_rule_map::const_iterator found = m_RuleForIdentifier.find(id);
    if (found != m_RuleForIdentifier.end()) return found->second;
    
    // Return the placeholder rule container if it wasn't found
    return empty_rule_container;
}

/// \brief Returns an identifier given an item. Identifiers are numbered from 0.
int grammar::identifier_for_item(const item_container& item) const {
    // Look up the item
    item_identifier_map::const_iterator found = m_ItemIdentifiers.find(item);
    
    // Return the previous value if one exists
    if (found != m_ItemIdentifiers.end()) return found->second;
    
    // Create a new rule if we haven't set an ID for this one yet
    int newId                   = (int) m_ItemIdentifiers.size();
    m_ItemIdentifiers[item]     = newId;
    m_ItemForIdentifier[newId]  = item;
    
    return newId;
}

/// \brief Returns the item that has the specified identifier
const item_container& grammar::item_with_identifier(int id) const {
    // Empty rule that we use when we fail
    static empty_item       empty;
    static item_container   empty_container(&empty, false);
    
    // Try to find this ID
    identifier_item_map::const_iterator found = m_ItemForIdentifier.find(id);
    if (found != m_ItemForIdentifier.end()) return found->second;
    
    // Return the placeholder rule container if it wasn't found
    return empty_container;
}

/// \brief Clears the caches associated with this grammar
///
/// This is necessary after adding new rules or items to this grammar, as doing this will cause things
/// like the cache of FIRST sets to become invalid. It is the responsibility of the object making modifications
/// to make this call at the appropriate time.
void grammar::clear_caches() const {
    m_CachedFirstSets.clear();
    m_CachedFollowSets.clear();
    m_CachedItemSets.clear();
}

/// \brief Returns the cached LR(1) item set for the item with the specified ID
///
/// This will be empty after the cache has been cleared.
lr::lr1_item_set& grammar::cached_set_for_item(int id) const {
    // Try to find the cached item set
    lr1_item_set_cache::iterator found = m_CachedItemSets.find(id);
    
    // Create a new item set if not found
    if (found == m_CachedItemSets.end()) {
        found = m_CachedItemSets.insert(pair<int, lr::lr1_item_set*>(id, new lr::lr1_item_set())).first;
    }
    
    // Return the result
    return *(found->second);
}

/// \brief Retrieves the cached value, or calculates the set FIRST(item)
///
/// This is the set of symbols that can appear at the beginning of the specified item. This is calculated recursively,
/// so if the item is a nonterminal, then the set will contain all of the terminals that can begin that item.
///
/// This will call first() on the specified item to retrieve the appropriate first set. While the call is in
/// progress, the first set for the requested item will be set to be 
const item_set& grammar::first(const item_container& item) const {
    // Try to look up the first set for this item
    item_set_map::iterator found = m_CachedFirstSets.find(item);
    
    // Return the value we found if it's already in the cache
    if (found != m_CachedFirstSets.end()) return found->second;
    
    // Not found: create an empty item set so recursive calls to this method terminate
    found = m_CachedFirstSets.insert(pair<item_container, item_set>(item, item_set(this))).first;
    
    // Ask the item for its first set, and update the cache
    found->second = item->first(*this);
    
    // Return the newly added item (have to do another lookup as map doesn't have a replace operator)
    return found->second;
}

/// \brief Computes the first set for the specified rule (or retrieves the cached version)
const item_set& grammar::first_for_rule(const rule& rule) const {
    // Return a set containing only the empty item if the rule is 0 items long
    if (rule.items().size() == 0) {
        return *m_EpsilonSet;
    }
    
    // Return the first set of the first item in the rule
    return first(*rule.items()[0]);
}


/// \brief Computes the follow set for the item with the specified identifier
///
/// This is the set of symbols that can follow a particular item, in any position in the grammar.
/// For performance reasons, terminal items are excluded from this set (they will always have an empty follow set)
const item_set& grammar::follow(const item_container& nonterminal) const {
    // Fill in the list of follow sets for the entire grammar if it's empty
    if (m_CachedFollowSets.empty()) {
        // Map from items to the items 
        item_map<item_set>::type dependencies;
        
        // Iterate through all of the rules in this grammar and build up the follow set for each one
        for (nonterminal_rule_map::const_iterator it = m_Nonterminals.begin(); it != m_Nonterminals.end(); it++) {
            for (rule_list::const_iterator ruleIt = it->second.begin(); ruleIt != it->second.end(); ruleIt++) {
                fill_follow(**ruleIt, dependencies);
            }
        }
        
        // Fill in all of the dependencies
        bool changed = true;
        while (changed) {
            changed = false;
            
            // Iterate through all of the dependencies
            for (item_map<item_set>::type::iterator depend = dependencies.begin(); depend != dependencies.end(); depend++) {
                // First item is the nonterminal that depends on other nonterminal
                const item_container& nonterminal = depend->first;
                
                // Iterate through each nonterminal for this item
                for (item_set::iterator dependency = depend->second.begin(); dependency != depend->second.end(); ++dependency) {
                    if (*dependency == nonterminal) continue;
                    
                    // Get the follow set for this dependency
                    item_set_map::iterator follow = m_CachedFollowSets.find(*dependency);

                    if (follow != m_CachedFollowSets.end()) {
                        // Add the items contained within this set to this dependency, and set changed if any of them cause a change
                        item_set_map::iterator ntFollow    = m_CachedFollowSets.find(nonterminal);
                        
                        if (ntFollow == m_CachedFollowSets.end()) {
                            ntFollow = m_CachedFollowSets.insert(item_set_map::value_type(nonterminal, item_set(this))).first;
                        }
                        
                        if (ntFollow->second.merge(follow->second)) {
                            changed = true;
                        }
                    }
                }
            }
        }
    }
    
    // Find the set for this item
    item_set_map::const_iterator found = m_CachedFollowSets.find(nonterminal);
    if (found == m_CachedFollowSets.end()) return item_set::empty_set;
    
    return found->second;
}

/// \brief Updates the follow set cache using the content of a particular rule
void grammar::fill_follow(const rule& rule, item_map<item_set>::type& dependencies) const {
    // Empty rules don't change the follow set for anything
    if (rule.items().size() == 0) return;
    
    // Iterate through the items in this rule
    for (size_t pos=0; pos<rule.items().size(); pos++) {
        // Get the current item
        const item_container& thisItem = rule.items()[pos];
        
        // Terminal items aren't processed by this call (we don't bother to generate follow sets for them)
        if (thisItem->type() == item::terminal) continue;
        
        // Retrieve the follow set for this item
        item_set_map::iterator followSet = m_CachedFollowSets.find(thisItem);
        if (followSet == m_CachedFollowSets.end()) {
            followSet = m_CachedFollowSets.insert(item_set_map::value_type(thisItem, item_set(this))).first;
        }
        
        // The follow set of this item is the combination of the first sets for all of the following items
        // If it's at the end, it also includes the follow set for the nonterminal for this rule
        size_t nextPos = pos+1;
        for (;nextPos < rule.items().size(); nextPos++) {
            // Get this following item
            const item_container& followingItem = rule.items()[nextPos];
            
            // Get the set FIRST(followingItem)
            const item_set& firstSet = first(followingItem);
            
            // Add to the follow set
            followSet->second.merge(firstSet);
            
            // Finished if the first set doesn't include the empty set
            if (!firstSet.contains(an_empty_item_c)) break;
        }
        
        // If we reach the end, then we need to include FOLLOW(rule.nonterminal) in the set for FOLLOW(thisItem)
        if (nextPos >= rule.items().size()) {
            item_set_map::iterator depend = dependencies.find(thisItem);
            if (depend == dependencies.end()) {
                depend = dependencies.insert(item_map<item_set>::type::value_type(thisItem, item_set(this))).first;
            }
            
            depend->second.insert(rule.nonterminal());
        }
        
        // If this item is an EBNF rule, then we need to process each of its children
        const ebnf* ebnfItem = thisItem->cast_ebnf();
        if (ebnfItem) {
            for (ebnf::rule_iterator subRule = ebnfItem->first_rule(); subRule != ebnfItem->last_rule(); subRule++) {
                fill_follow(**subRule, dependencies);
            }
        }
    }
}

/// \brief Appends an item to the rule that this is building
grammar::builder& grammar::builder::operator<<(const item& item) {
    m_Target << item;
    return *this;
}

/// \brief Appends an item to the rule that this is building
grammar::builder& grammar::builder::operator<<(const item_container& item) {
    m_Target << item;
    return *this;
}

/// \brief Appends a new nonterminal item to the rule that this building
grammar::builder& grammar::builder::operator<<(const std::wstring& ntName) {
    nonterminal nt(m_Grammar.id_for_nonterminal(ntName));
    m_Target << item_container(new nonterminal(m_Grammar.id_for_nonterminal(ntName)), true);
    return *this;
}

/// \brief Appends a new terminal item to the rule that this is building
grammar::builder& grammar::builder::operator<<(int term) {
    m_Target << item_container(new terminal(term), true);
    return *this;
}

/// \brief Begins defining a new rule for the nonterminal with the specified name
grammar::builder grammar::operator+=(const std::wstring& newNonterminal) {
    int nonTerminalId = id_for_nonterminal(newNonterminal);
    
    rule_list&  list = rules_for_nonterminal(nonTerminalId);
    item_container newNt(new nonterminal(nonTerminalId), true);
    list.push_back(rule_container(new rule(newNt), true));
    
    return builder(**list.rbegin(), *this);
}

/// \brief Begins defining a new rule for the specified nonterminal
grammar::builder grammar::operator+=(const nonterminal& newNt) {
    rule_list&  list = rules_for_nonterminal(newNt.symbol());
    list.push_back(rule_container(new rule(newNt), true));
    
    return builder(**list.rbegin(), *this);    
}
