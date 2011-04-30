//
//  ebnf_items.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "ebnf_items.h"
#include "symbol_set.h"

using namespace dfa;
using namespace contextfree;

static const empty_item the_empty_item;

/// \brief Creates a new EBNF item
ebnf::ebnf()
: m_Rules(new rule_list())
, item(symbol_set::null) {
    m_Rules->push_back(rule(this));
}

/// \brief Creates a new EBNF item from a rule
ebnf::ebnf(const rule& copyFrom) 
: item(symbol_set::null)
, m_Rules(new rule_list()) {
    m_Rules->push_back(rule(copyFrom, *this));
}

/// \brief Copy constructor
ebnf::ebnf(const ebnf& copyFrom)
: item(symbol_set::null)
, m_Rules(new rule_list()) {
    for (rule_list::const_iterator it = copyFrom.m_Rules->begin(); it != copyFrom.m_Rules->end(); it++) {
        m_Rules->push_back(rule(**it, *this));
    }
}

/// \brief Copy constructor
ebnf::ebnf(const rule_list& copyFrom)
: item(symbol_set::null)
, m_Rules(new rule_list()) {
    for (rule_list::const_iterator it = copyFrom.begin(); it != copyFrom.end(); it++) {
        m_Rules->push_back(rule(**it, *this));
    }
}

/// \brief Destructor
ebnf::~ebnf() {
    delete m_Rules;
}

/// \brief Adds a new rule to this construction
void ebnf::add_rule(const rule& newRule) {
    m_Rules->push_back(new rule(newRule, *this));
}

/// \brief Compares this item to another. Returns true if they are the same
bool ebnf::operator==(const item& compareTo) const {
    const ebnf* compareEbnf = dynamic_cast<const ebnf*>(&compareTo);
    if (!compareEbnf) return false;
    
    if (m_Rules->size() < compareEbnf->m_Rules->size()) return false;
    if (m_Rules->size() > compareEbnf->m_Rules->size()) return false;
    
    rule_iterator ourIt     = first_rule();
    rule_iterator theirIt   = compareEbnf->first_rule();
    
    for (;ourIt != last_rule() && theirIt != compareEbnf->last_rule(); ourIt++, theirIt++) {
        if (*ourIt == *theirIt) continue;
        return false;
    }
    
    // Equal if we reach here
    return true;
}

/// \brief Orders this item relative to another item
bool ebnf::operator<(const item& compareTo) const {
    const ebnf* compareEbnf = dynamic_cast<const ebnf*>(&compareTo);
    if (!compareEbnf) return false;

    if (m_Rules->size() < compareEbnf->m_Rules->size()) return true;
    if (m_Rules->size() > compareEbnf->m_Rules->size()) return false;

    rule_iterator ourIt     = first_rule();
    rule_iterator theirIt   = compareEbnf->first_rule();
    
    for (;ourIt != last_rule() && theirIt != compareEbnf->last_rule(); ourIt++, theirIt++) {
        if (*ourIt == *theirIt) continue;
        return *ourIt < *theirIt;
    }
    
    // Equal if we reach here
    return false;
}

/// \brief The rule that defines this item
rule& ebnf::get_rule() { return *(*m_Rules)[0]; }

/// \brief The rule that defines this item
const rule& ebnf::get_rule() const { return *(*m_Rules)[0]; }

/// \brief Creates a clone of this item
item* ebnf_optional::clone() const {
    return new ebnf_optional(*this);
}

/// \brief The type of this item
item::kind ebnf_optional::type() const {
    return optional;
}

/// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
///
/// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
/// set for the rules that make them up.
///
/// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
/// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
item_set ebnf_optional::first(const grammar& gram) const {
    // Result is the first item in the rule
    item_set result;
    
    // Get the rule
    const rule& r = get_rule();
    
    // Add items if the rule has a first item
    if (r.items().size() > 0) {
        const item_set& ruleItems = gram.first(*(r.items()[0]));
        result.insert(ruleItems.begin(), ruleItems.end());
    }
    
    return result;
}

/// \brief Creates a clone of this item
item* ebnf_repeating::clone() const {
    return new ebnf_repeating(*this);
}

/// \brief The type of this item
item::kind ebnf_repeating::type() const {
    return repeat;
}

/// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
///
/// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
/// set for the rules that make them up.
///
/// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
/// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
item_set ebnf_repeating::first(const grammar& gram) const {
    // Result is the empty item, plus the first item in the rule
    item_set result;
    result.insert(the_empty_item);
    
    // Get the rule
    const rule& r = get_rule();
    
    // Add items if the rule has a first item
    if (r.items().size() > 0) {
        const item_set& ruleItems = gram.first(*(r.items()[0]));
        result.insert(ruleItems.begin(), ruleItems.end());
    } else {
        result.insert(the_empty_item);
    }
    
    return result;    
}

/// \brief Adds a new rule
rule_container ebnf_alternate::add_rule() {
    // Add a new rule
    ebnf::add_rule(rule(*this));
    
    return rules()[rules().size()-1];
}
    
/// \brief Creates a clone of this item
item* ebnf_alternate::clone() const {
    return new ebnf_alternate(*this);
}

/// \brief The type of this item
item::kind ebnf_alternate::type() const {
    return alternative;
}

/// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
///
/// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
/// set for the rules that make them up.
///
/// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
/// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
item_set ebnf_alternate::first(const grammar& gram) const {
    // Result is combination of the first items in the rules
    item_set result;
    
    // Iterate through all of the rules
    for (rule_iterator it = first_rule(); it != last_rule(); it++) {
        // Get the rule
        const rule& r = **it;
        
        // Add items if the rule has a first item
        if (r.items().size() > 0) {
            const item_set& ruleItems = gram.first(*(r.items()[0]));
            result.insert(ruleItems.begin(), ruleItems.end());
        } else {
            result.insert(the_empty_item);
        }
    }
    
    return result;
}
