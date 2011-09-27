//
//  ebnf_items.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stack>
#include <set>

#include "TameParse/ContextFree/ebnf_items.h"
#include "TameParse/Dfa/symbol_set.h"
#include "TameParse/Lr/lr_item.h"

using namespace std;
using namespace dfa;
using namespace lr;
using namespace contextfree;

static const empty_item the_empty_item;

/// \brief Creates a new EBNF item
ebnf::ebnf()
: m_Rules(new rule_list())
, item(symbol_set::null) {
    item_container ourselves(this, false);
    m_Rules->push_back(rule(ourselves));
}

/// \brief Creates a new EBNF item from a rule
ebnf::ebnf(const rule& copyFrom) 
: item(symbol_set::null)
, m_Rules(new rule_list()) {
    item_container ourselves(this, false);
    m_Rules->push_back(rule(copyFrom, ourselves));
}

/// \brief Copy constructor
ebnf::ebnf(const ebnf& copyFrom)
: item(symbol_set::null)
, m_Rules(new rule_list()) {
    item_container ourselves(this, false);
    
    for (rule_list::const_iterator it = copyFrom.m_Rules->begin(); it != copyFrom.m_Rules->end(); it++) {
        m_Rules->push_back(rule(**it, ourselves));
    }
}

/// \brief Copy constructor
ebnf::ebnf(const rule_list& copyFrom)
: item(symbol_set::null)
, m_Rules(new rule_list()) {
    item_container ourselves(this, false);

    for (rule_list::const_iterator it = copyFrom.begin(); it != copyFrom.end(); it++) {
        m_Rules->push_back(rule(**it, ourselves));
    }
}

/// \brief Destructor
ebnf::~ebnf() {
    delete m_Rules;
}

/// \brief Adds a new rule to this construction
void ebnf::add_rule(const rule& newRule) {
    item_container ourselves(this, false);

    m_Rules->push_back(new rule(newRule, ourselves));
}

/// \brief Compares this item to another. Returns true if they are the same
bool ebnf::operator==(const item& compareTo) const {
    const ebnf* compareEbnf = compareTo.cast_ebnf();
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
    if (&compareTo == this) return false;
    
    const ebnf* compareEbnf = compareTo.cast_ebnf();
    if (!compareEbnf) return false;

    if (m_Rules->size() < compareEbnf->m_Rules->size()) return true;
    if (m_Rules->size() > compareEbnf->m_Rules->size()) return false;

    rule_iterator ourIt     = first_rule();
    rule_iterator theirIt   = compareEbnf->first_rule();
    
    for (;ourIt != last_rule() && theirIt != compareEbnf->last_rule(); ourIt++, theirIt++) {
        int compare = (*ourIt)->compare_items(**theirIt);
        if (compare < 0) return true;
        if (compare > 0) return false;
    }
    
    // Equal if we reach here
    return false;
}

/// \brief The rule that defines this item
rule_container& ebnf::get_rule() { return (*m_Rules)[0]; }

/// \brief The rule that defines this item
const rule_container& ebnf::get_rule() const { return (*m_Rules)[0]; }

/// \brief Returns NULL, or this item as an EBNF item
const ebnf* ebnf::cast_ebnf() const {
    return this;
}

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
    // Result is the empty item, plus the first item in the rule
    item_set result(gram);
    result.insert(the_empty_item);
    
    // Get the rule
    const rule& r = *get_rule();
    
    // Add items if the rule has a first item
    if (r.items().size() > 0) {
        const item_set& ruleItems = gram.first(*(r.items()[0]));
        result.merge(ruleItems);
    } else {
        result.insert(the_empty_item);
    }
    
    return result;
}

/// \brief Computes the closure of this rule in the specified grammar
void ebnf_optional::closure(const lr::lr1_item& item, lr::lr1_item_set& state, const grammar& gram) const {
    // Use the item_container from the item to save on copying
    const item_container& ourItem = item.rule()->items()[item.offset()];

    // Empty rule, or the rule in this container
    rule_container empty_rule(new rule(ourItem), true);
    
    // Work out the follow set
    item_set follow(gram);
    fill_follow(follow, item, gram);
    
    // Add new items
    insert_closure_item(lr1_item_container(new lr1_item(&gram, empty_rule, 0, follow), true), state, gram);
    insert_closure_item(lr1_item_container(new lr1_item(&gram, get_rule(), 0, follow), true), state, gram);
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
    // Result is the first item in the rule
    item_set result(gram);
    
    // Get the rule
    const rule& r = *get_rule();
    
    // Add items if the rule has a first item
    if (r.items().size() > 0) {
        const item_set& ruleItems = gram.first(*(r.items()[0]));
        result.merge(ruleItems);
    }
    
    return result;
}

/// \brief Computes the closure of this rule in the specified grammar
void ebnf_repeating::closure(const lr::lr1_item& item, lr::lr1_item_set& state, const grammar& gram) const {
    // Use the item_container from the item to save on copying
    const item_container& ourItem = item.rule()->items()[item.offset()];
    
    // Just the rule in this item, or a repetition
    rule_container many_rule(new rule(ourItem), true);
    (*many_rule) << ourItem << get_rule();
    
    // Work out the follow set
    item_set follow(gram);
    fill_follow(follow, item, gram);
    
    // Add new items
    insert_closure_item(lr1_item_container(new lr1_item(&gram, get_rule(), 0, follow), true), state, gram);
    insert_closure_item(lr1_item_container(new lr1_item(&gram, many_rule, 0, follow), true), state, gram);
}

/// \brief Creates a clone of this item
item* ebnf_repeating_optional::clone() const {
    return new ebnf_repeating_optional(*this);
}

/// \brief The type of this item
item::kind ebnf_repeating_optional::type() const {
    return item::repeat_zero_or_one;
}

/// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
///
/// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
/// set for the rules that make them up.
///
/// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
/// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
item_set ebnf_repeating_optional::first(const grammar& gram) const {
    // Result is the empty item, plus the first item in the rule
    item_set result(gram);
    result.insert(the_empty_item);
    
    // Get the rule
    const rule& r = *get_rule();
    
    // Add items if the rule has a first item
    if (r.items().size() > 0) {
        const item_set& ruleItems = gram.first(*(r.items()[0]));
        result.merge(ruleItems);
    } else {
        result.insert(the_empty_item);
    }
    
    return result;    
}

/// \brief Computes the closure of this rule in the specified grammar
void ebnf_repeating_optional::closure(const lr::lr1_item& item, lr::lr1_item_set& state, const grammar& gram) const {
    // Use the item_container from the item to save on copying
    const item_container& ourItem = item.rule()->items()[item.offset()];
    
    // Empty, or a repetition
    rule_container empty_rule(new rule(ourItem), true);
    rule_container many_rule(new rule(ourItem), true);
    (*many_rule) << ourItem << get_rule();
    
    // Work out the follow set
    item_set follow(gram);
    fill_follow(follow, item, gram);
    
    // Add new items
    insert_closure_item(lr1_item_container(new lr1_item(&gram, empty_rule, 0, follow), true), state, gram);
    insert_closure_item(lr1_item_container(new lr1_item(&gram, many_rule, 0, follow), true), state, gram);
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
    item_set result(gram);
    
    // Iterate through all of the rules
    for (rule_iterator it = first_rule(); it != last_rule(); it++) {
        // Get the rule
        const rule& r = **it;
        
        // Add items if the rule has a first item
        if (r.items().size() > 0) {
            const item_set& ruleItems = gram.first(*(r.items()[0]));
            result.merge(ruleItems);
        } else {
            result.insert(the_empty_item);
        }
    }
    
    return result;
}

/// \brief Computes the closure of this rule in the specified grammar
void ebnf_alternate::closure(const lr::lr1_item& item, lr::lr1_item_set& state, const grammar& gram) const {
    // Work out the follow set
    item_set follow(gram);
    fill_follow(follow, item, gram);
    
    // Any of the items in this rule
    for (rule_list::const_iterator nextRule = rules().begin(); nextRule != rules().end(); nextRule++) {
        // If this particular rule is just another alternative, then flatten things out instead of referencing it
        // This simplifies the AST and also reduces the number of reductions that need to be performed
        if ((*nextRule)->items().size() == 1 && (*nextRule)->items()[0]->type() == item::alternative) {
            // Reference ourselves
            item_container ourselves(const_cast<ebnf_alternate*>(this), false);

            // Recursively flatten this rule
            stack<rule_container>   toFlatten;
            set<item_container>     flattened;
            toFlatten.push(*nextRule);

            // We're already flattening ourselves
            flattened.insert(ourselves);

            // Loop until there are no more rules to add
            while (!toFlatten.empty()) {
                // Get the next rule to flatten (will be a rule containing a single alternative)
                rule_container alternateRule = toFlatten.top();

                // Pop from the stack
                toFlatten.pop();

                // Get the alternate item for this rule
                item_container alternate = alternateRule->items()[0];

                // Nothing to do if we've already encountered it
                if (flattened.find(alternate) != flattened.end()) continue;

                // Remember this as flattened, so we won't process it again
                flattened.insert(alternate);

                // Iterate through the rules in this item
                const ebnf* alternateEbnf = (const ebnf*) alternate.item();

                for (rule_list::const_iterator ruleToFlatten = alternateEbnf->first_rule(); ruleToFlatten != alternateEbnf->last_rule(); ruleToFlatten++) {
                    // Single-item alternates get flattened later on
                    if ((*ruleToFlatten)->items().size() == 1 && (*ruleToFlatten)->items()[0]->type() == item::alternative) {
                        toFlatten.push(*ruleToFlatten);
                    } 

                    // Other rules get rewritten with this item as the target
                    else {
                        rule_container rewrittenRule(new rule(**ruleToFlatten, ourselves));
                        insert_closure_item(lr1_item_container(new lr1_item(&gram, rewrittenRule, 0, follow)), state, gram);
                    }
                }
            }
        } 

        // All other rules are passed through untouched
        else {
            insert_closure_item(lr1_item_container(new lr1_item(&gram, *nextRule, 0, follow)), state, gram);
        }
    }
}
