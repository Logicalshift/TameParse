//
//  standard_items.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "standard_items.h"
#include "symbol_set.h"
#include "Lr/lr_item.h"

using namespace dfa;
using namespace lr;
using namespace contextfree;

/// \brief Creates a terminal that matches the specified symbol
terminal::terminal(int sym)
: item(sym) {
}

/// \brief Creates a clone of this item
item* terminal::clone() const {
    return new terminal(symbol());
}

/// \brief The type of this item
item::kind terminal::type() const {
    return item::terminal;
}

/// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
///
/// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
/// set for the rules that make them up.
///
/// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
/// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
item_set terminal::first(const grammar& gram) const {
    // Just this item
    item_set result;
    result.insert(this);
    return result;
}

/// \brief Creates a non-terminal item that matches the specified symbol
nonterminal::nonterminal(int sym)
: item(sym) {
}

/// \brief Creates a clone of this item
item* nonterminal::clone() const {
    return new nonterminal(symbol());
}

/// \brief The type of this item
item::kind nonterminal::type() const {
    return item::nonterminal;
}

/// \brief Constant empty item
static const empty_item an_empty_item;
static item_container an_empty_item_c((item*)&an_empty_item, false);

/// \brief Computes the closure of this rule in the specified grammar
void nonterminal::closure(const lr1_item& item, lr1_item_set& state, const grammar& gram) const {
    // Get the rules for this nonterminal
    const rule_list& ntRules = gram.rules_for_nonterminal(symbol());
    
    // Work out what follows in the item
    item_set follow;
    fill_follow(follow, item, gram);

    // Generate new rules for each of these, and add to the state
    for (rule_list::const_iterator it = ntRules.begin(); it != ntRules.end(); it++) {
        // Create the LR(1) item for the new item
        lr1_item_container newItem(new lr1_item(&gram, *it, 0, follow), true);
        
        // Add to the closure
        insert_closure_item(newItem, state, gram);
    }
}

/// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
///
/// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
/// set for the rules that make them up.
///
/// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
/// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
item_set nonterminal::first(const grammar& gram) const {
    item_set result;
    
    // Add this item
    result.insert(this);

    // Ask the grammar for the rules for this nonterminal
    const rule_list& rules = gram.rules_for_nonterminal(symbol());
    
    // Merge in the first set for each of the initial symbols
    for (rule_list::const_iterator it = rules.begin(); it != rules.end(); it++) {
        // Get the items for this rule
        const item_list& ruleItems = (*it)->items();
        
        // Add the empty item if this is an empty rule
        if (ruleItems.size() == 0) {
            result.insert(an_empty_item_c);
            continue;
        }
        
        // Get the first set for the first item in this rule
        const item_set& ruleFirst = gram.first(ruleItems[0]);

        // Insert into the result
        result.insert(ruleFirst.begin(), ruleFirst.end());

        if (ruleFirst.find(an_empty_item_c) != ruleFirst.end()) {
            // If there's an empty item, then merge the first sets from the later parts of the rule
            int pos;
            for (pos = 1; pos < ruleItems.size(); pos++) {
                // Remove the empty item from the current set
                result.erase(an_empty_item);
                
                // Get the first set for the next item
                const item_set& nextFirst = gram.first(*(ruleItems[pos]));
                
                // Merge it with the current set
                result.insert(nextFirst.begin(), nextFirst.end());
                
                // Stop if there's no longer an empty item
                if (nextFirst.find(an_empty_item) == nextFirst.end()) break;
            }
        }
    }

    return result;
}

/// \brief Creates a terminal that matches the specified symbol
empty_item::empty_item() 
: item(symbol_set::null) {
}

/// \brief Compares this item to another. Returns true if they are the same
bool empty_item::operator==(const item& compareTo) const {
    return compareTo.type() == item::empty;
}

/// \brief Orders this item relative to another item
bool empty_item::operator<(const item& compareTo) const {
    return item::empty < compareTo.type();
}

/// \brief Creates a clone of this item
item* empty_item::clone() const {
    return new empty_item();
}

/// \brief The type of this item
item::kind empty_item::type() const {
    return item::empty;
}

/// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
///
/// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
/// set for the rules that make them up.
///
/// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
/// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
item_set empty_item::first(const grammar& gram) const {
    // Just this item
    item_set result;
    result.insert(this);
    return result;
}

/// \brief Computes the closure of this rule in the specified grammar
void empty_item::closure(const lr1_item& item, lr1_item_set& state, const grammar& gram) const {
    // The empty item can always be immediately skipped
    lr1_item newItem(&gram, item.rule(), item.offset()+1, item.lookahead());
    
    // Insert the new item into the state
    state.insert(newItem);
}


/// \brief True if a transition (new state) should be generated for this item
bool empty_item::generate_transition() const {
    // The empty item never appears as a symbol by itself, so the parser should not generate a transition for it
    return false;
}

/// \brief Creates a terminal that matches the specified symbol
end_of_input::end_of_input() 
: item(symbol_set::null) {
}

/// \brief Compares this item to another. Returns true if they are the same
bool end_of_input::operator==(const item& compareTo) const {
    return compareTo.type() == item::eoi;
}

/// \brief Orders this item relative to another item
bool end_of_input::operator<(const item& compareTo) const {
    return item::eoi < compareTo.type();
}

/// \brief Creates a clone of this item
item* end_of_input::clone() const {
    return new end_of_input();
}

/// \brief The type of this item
item::kind end_of_input::type() const {
    return item::eoi;
}

/// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
///
/// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
/// set for the rules that make them up.
///
/// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
/// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
item_set end_of_input::first(const grammar& gram) const {
    // Just this item
    item_set result;
    result.insert(this);
    return result;
}
