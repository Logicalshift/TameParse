//
//  item.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "item.h"
#include "Lr/lr_item.h"

using namespace lr;
using namespace contextfree;


item* empty_item_constructor::construct() {
    return new empty_item();
}

void empty_item_constructor::destruct(item* i) {
    delete i;
}

/// \brief Destructor
item::~item() {
}

/// \brief Compares this item to another. Returns true if they are the same
bool item::operator==(const item& compareTo) const {
    return compareTo.type() == type() && compareTo.symbol() == symbol();
}

/// \brief Orders this item relative to another item
bool item::operator<(const item& compareTo) const {
    return compareTo.type() < type() || compareTo.symbol() < symbol();
}

/// \brief Comparison function, returns true if a is less than b
bool item::compare(const item& a, const item& b) {
    // Compare the kinds
    kind aKind = a.type();
    kind bKind = b.type();
    
    if (aKind < bKind) return true;
    if (aKind > bKind) return false;
    
    // For objects with an 'other' type, use RTTI to compare the underlying types
    if (aKind >= other) {
        // Compare the types of these items
        bool isBefore = typeid(a).before(typeid(b));
        
        // a is less than b if its type is before b
        if (isBefore) return true;
        
        // a is greater than b if the types aren't equal
        if (typeid(a) != typeid(b)) return false;
        
        // Call through to the items own less-than operator
        return a < b;
    }
    
    // For EBNF items, use the comparison operator built into the item
    if (aKind >= ebnf) {
        return a < b;
    }
    
    // For well-known kinds, we can just compare the symbols
    int aSym = a.symbol();
    int bSym = b.symbol();
    
    return aSym < bSym;
}

static const empty_item the_empty_item;
static item_container   an_empty_item_c((item*) &the_empty_item, false);

/// \brief Fills in the items that follow this one
void item::fill_follow(item_set& follow, const lr::lr1_item& item, const grammar& gram) const {
    // Work out what follows in the item
    const rule& rule        = *item.rule();
    int         offset      = item.offset();
    size_t      numItems    = rule.items().size();
    
    if (offset+1 >= numItems) {
        // The follow set is just the lookahead for this item
        follow = item.lookahead();
    } else {
        // The follow set is FIRST(following item)
        int followOffset = offset + 1;
        follow = gram.first(rule.items()[followOffset]);
        
        // Add further following items if the follow set can be empty, until we reach the end
        followOffset++;
        while (followOffset < numItems && follow.find(an_empty_item_c) != follow.end()) {
            // Remove the empty item
            follow.erase(an_empty_item_c);
            
            // Add the items from the next item in the rule
            const item_set& newItems = gram.first(rule.items()[followOffset]);
            follow.insert(newItems.begin(), newItems.end());
            
            // Move on to the next item
            followOffset++;
        }
        
        // If the empty set is still included, remove it and add the item lookahead
        // (Note that if the loop above terminated early, then the empty set can't be in the follow set at this point)
        if (followOffset >= numItems && follow.find(an_empty_item_c) != follow.end()) {
            follow.erase(an_empty_item_c);
            follow.insert(item.lookahead().begin(), item.lookahead().end());
        }
    }    
}

/// \brief Adds a new item to the specified set. Returns true if the set was changed
///
/// If the item is the same as an existing item except for the lookahead, the lookaheads are merged and the
/// set is the same size.
bool item::add(lr1_item_set& state, const grammar& gram, const lr1_item_container& newItem) {
    // Create a variant of the item with no lookahead
    typedef lr1_item::lookahead_set lookahead_set;
    static lookahead_set emptyLookaheadSet;
    
    lr1_item_container noLookahead(new lr1_item(&gram, newItem->rule(), newItem->offset(), emptyLookaheadSet), true);
    
    // Search for the first item that is >= noLookahead (items with any lookahead will be greater than this item)
    lr1_item_set::iterator maybeExisting = state.lower_bound(noLookahead);
    
    // If there's no rule then we can just add the item
    if (maybeExisting == state.end()) {
        state.insert(newItem);
        return true;
    }
    
    // If the rule is not the same (using the rule identifier for performance) then we can just add the item
    if ((*maybeExisting)->offset() != newItem->offset()
        || (*maybeExisting)->rule()->identifier(gram) != newItem->rule()->identifier(gram)) {
        state.insert(newItem);
        return true;
    }

    // If there are any new lookahead items, then we need to merge the lookahead sets
    // TODO: is there a quicker way of detecting when the sets are similar?
    
    // If the sets are the same, then this item has already been added and there's nothing more to do
    if (newItem->lookahead().size() == (*maybeExisting)->lookahead().size()) {
        if (newItem->lookahead() == (*maybeExisting)->lookahead()) {
            return false;
        }
    }
    
    // Merge the sets, and update the item if the size changes
    lookahead_set   mergedLookahead((*maybeExisting)->lookahead());
    size_t          initialSize = mergedLookahead.size();
    
    mergedLookahead.insert(newItem->lookahead().begin(), newItem->lookahead().end());
    
    // If the merged set is the same size as before then there have been no changes (these lookahead items already existed)
    if (mergedLookahead.size() == initialSize) {
        return false;
    }
    
    // Replace the existing item with the merged item
    lr1_item_container mergedItem(new lr1_item(&gram, newItem->rule(), newItem->offset(), mergedLookahead), true);
    
    state.erase(maybeExisting);
    state.insert(mergedItem);
    
    return true;
}


/// \brief Adds a new LR(1) item to a closure, completing the closure as necessary
void item::insert_closure_item(const lr::lr1_item_container& newItem, lr::lr1_item_set& state, const grammar& gram) const {
    const rule& rule = *newItem->rule();
    
    // Recursively add the closure for the new items
    if (add(state, gram, newItem) && !rule.items().empty()) {
        const class item& initial = *rule.items()[0];
        if (initial.type() != item::terminal) {
            initial.closure(*newItem, state, gram);
        }
    }
}

/// \brief Computes the closure of this rule in the specified grammar
///
/// This is the set of spontaneously generated LR(0) items for this item, and is used to generate the closure when
/// producing a parser. This call is supplied the item for which the closure is being generated, and a set of states
/// to which new items can be added (and the grammar so rules can be looked up).
///
/// A spontaneously generated rule is one that is implied by this item. For example, if parser is trying to match the
/// nonterminal 'X', then the rules for that nonterminal are spontaneously generated.
void item::closure(const lr1_item& item, lr1_item_set& state, const grammar& gram) const {
    // Nothing to do by default
}
/// \brief True if a transition (new state) should be generated for this item
///
/// Should return false for any item that acts like the empty item
bool item::generate_transition() const {
    // True by default
    return true;
}
