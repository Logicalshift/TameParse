//
//  standard_items.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "standard_items.h"
#include "symbol_set.h"

using namespace dfa;
using namespace contextfree;

/// \brief Creates a terminal that matches the specified symbol
terminal::terminal(int sym)
: item(sym) {
}

/// \brief Destructor
terminal::~terminal() {
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

/// \brief Destructor
nonterminal::~nonterminal() {
}

/// \brief Creates a clone of this item
item* nonterminal::clone() const {
    return new nonterminal(symbol());
}

/// \brief The type of this item
item::kind nonterminal::type() const {
    return item::nonterminal;
}

/// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
///
/// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
/// set for the rules that make them up.
///
/// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
/// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
item_set nonterminal::first(const grammar& gram) const {
    // Add this item
    item_set result;
    result.insert(this);

    // TODO: return the result of getting first() from the rules of this nonterminal
    return result;
}

/// \brief Creates a guard item that matches the rule with the specified identifier
guard::guard(int sym)
: item(sym) {
}

/// \brief Destructor
guard::~guard() {
}

/// \brief Creates a clone of this item
item* guard::clone() const {
    return new guard(symbol());
}

/// \brief The type of this item
item::kind guard::type() const {
    return item::guard;
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

/// \brief Creates a terminal that matches the specified symbol
empty_item::empty_item() 
: item(symbol_set::null) {
}

/// \brief Destructor
empty_item::~empty_item() {
}

/// \brief Compares this item to another. Returns true if they are the same
bool empty_item::operator==(const item& compareTo) const {
    if (typeid(compareTo) == typeid(empty_item)) return true;
    return false;
}

/// \brief Orders this item relative to another item
bool empty_item::operator<(const item& compareTo) const {
    if (typeid(compareTo) == typeid(empty_item)) return false;
    return true;
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
