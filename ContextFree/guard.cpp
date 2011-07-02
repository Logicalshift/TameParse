//
//  guard.cpp
//  Parse
//
//  Created by Andrew Hunter on 29/06/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "guard.h"
#include "Dfa/symbol_set.h"

using namespace dfa;
using namespace contextfree;

/// \brief Creates a guard item that matches the rule with the specified identifier
guard::guard(int priority)
: item(symbol_set::null)
, m_Priority(priority)
, m_Rule(NULL, true) {
    // Create an item container pointing at this object (won't free this item)
    item_container ourselves(this, false);
    
    // Update the rule to use this item container
    // NOTE: if this goes out of scope later than the rule then we will end up with a duff reference
    m_Rule = rule_container(new contextfree::rule(ourselves), true);
}

/// \brief Creates a new guard item by copying a rule
guard::guard(const contextfree::rule& copyFrom, int priority)
: item(symbol_set::null)
, m_Priority(priority)
, m_Rule(NULL, true) {
    // Create an item container pointing at this object (won't free this item)
    item_container ourselves(this, false);
    
    // Update the rule to use this item container
    // NOTE: if this goes out of scope later than the rule then we will end up with a duff reference
    m_Rule = rule_container(new contextfree::rule(copyFrom, ourselves), true);    
}

/// \brief Creates a new guard by copying an old one
guard::guard(const guard& copyFrom)
: item(symbol_set::null)
, m_Priority(copyFrom.m_Priority)
, m_Rule(NULL, true) {
    // Create an item container pointing at this object (won't free this item)
    item_container ourselves(this, false);
    
    // Update the rule to use this item container
    // NOTE: if this goes out of scope later than the rule then we will end up with a duff reference
    m_Rule = rule_container(new contextfree::rule(*copyFrom.get_rule(), ourselves), true);    
}

/// \brief Creates a clone of this item
item* guard::clone() const {
    // Create a new guard
    return new guard(*this);
}

/// \brief The type of this item
item::kind guard::type() const {
    return item::guard;
}

/// \brief The priority of this guard relative to other guards (higher values = higher priorities, -1 by default)
int guard::priority() const {
    return m_Priority;
}

/// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
///
/// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
/// set for the rules that make them up.
///
/// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
/// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
item_set guard::first(const grammar& gram) const {
    // Just this item
    item_set result;
    result.insert(this);
    return result;
}

/// \brief Orders this item relative to another item
bool guard::operator<(const item& compareTo) const {
    if (&compareTo == this) return false;
    
    if (item::guard < compareTo.type()) return true;
    if (item::guard > compareTo.type()) return false;
    
    // Cast the comparison to a guard object
    const guard* compareGuard = dynamic_cast<const guard*>(&compareTo);
    if (!compareGuard) return false;
    
    // Ordering is initial by priority
    int theirPriority = compareGuard->priority();
    
    if (m_Priority < theirPriority) return true;
    if (m_Priority > theirPriority) return false;
    
    // If the priorities are the same, then use the rule to do ordering
    return get_rule() < compareGuard->get_rule();
}

/// \brief Computes the set of symbols that can form the initial symbol of a lookahead that matches this guard
item_set guard::initial(const grammar& gram) const {
    return gram.first_for_rule(*get_rule());
}

/// \brief The rule that should be matched for this guard to be successful
rule_container& guard::get_rule() {
    return m_Rule;
}

/// \brief The rule that should be matched for this guard to be successful
const rule_container& guard::get_rule() const {
    return m_Rule;
}
