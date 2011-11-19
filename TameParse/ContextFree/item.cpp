//
//  item.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/common.h"

#include "TameParse/ContextFree/item.h"
#include "TameParse/Lr/lr_item.h"
#include "TameParse/Lr/lr1_item_set.h"

using namespace lr;
using namespace contextfree;

item* empty_item_constructor::default_item() {
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
    // Compare types
    int ourType     = type();
    int theirType   = compareTo.type();
    if (ourType < theirType) return true;
    if (ourType > theirType) return false;
    
    // Compare symbols if the types are same
    return symbol() < compareTo.symbol();
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
    
    // For well-known kinds we can just compare the symbols
    switch (aKind) {
        case terminal:
        case nonterminal:
        {
            int aSym = a.symbol();
            int bSym = b.symbol();
            
            return aSym < bSym;
        }
            
        default:
            // For other types, we call through to the comparison operator
            return a < b;
    }
}

static const empty_item the_empty_item;
static item_container   an_empty_item_c((item*) &the_empty_item, false);

/// \brief Fills in the items that follow this one
void item::fill_follow(item_set& follow, const lr::lr1_item& item, const grammar& gram) const {
    // Work out what follows in the item
    const rule& rule        = *item.rule();
    int         offset      = item.offset();
    int			numItems    = (int) rule.items().size();
    
    if (offset+1 >= numItems) {
        // The follow set is just the lookahead for this item
        follow = item.lookahead();
    } else {
        // The follow set is FIRST(following item)
        int followOffset = offset + 1;
        follow = gram.first(rule.items()[followOffset]);
        
        // Add further following items if the follow set can be empty, until we reach the end
        followOffset++;
        while (followOffset < numItems && follow.contains(an_empty_item_c)) {
            // Remove the empty item
            follow.erase(an_empty_item_c);
            
            // Add the items from the next item in the rule
            const item_set& newItems = gram.first(rule.items()[followOffset]);
            follow.merge(newItems);
            
            // Move on to the next item
            followOffset++;
        }
        
        // If the empty set is still included, remove it and add the item lookahead
        // (Note that if the loop above terminated early, then the empty set can't be in the follow set at this point)
        if (followOffset >= numItems && follow.contains(an_empty_item_c)) {
            follow.erase(an_empty_item_c);
            follow.merge(item.lookahead());
        }
    }    
}

/// \brief Adds a new item to the specified set. Returns true if the set was changed
///
/// If the item is the same as an existing item except for the lookahead, the lookaheads are merged and the
/// set is the same size.
bool item::add(lr1_item_set& state, const grammar& gram, const lr1_item_container& newItem) {
    return state.insert(newItem).second;
}


/// \brief Adds a new LR(1) item to a closure, completing the closure as necessary
void item::insert_closure_item(const lr::lr1_item_container& newItem, lr::lr1_item_set& state, const grammar& gram) const {
    const rule& rule = *newItem->rule();
    
    // Recursively add the closure for the new items
    if (add(state, gram, newItem) && !rule.items().empty()) {
        const class item& initial = *rule.items()[0];
        if (initial.type() != item::terminal) {
            initial.cache_closure(*newItem, state, gram);
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

static const end_of_input an_eoi_item;
static item_container an_eoi_item_c((item*)&an_eoi_item, false);

/// \brief Like closure, except this will use the grammar closure cache to improve performance
void item::cache_closure(const lr::lr1_item& it, lr::lr1_item_set& state, const grammar& gram) const {
    // If we're already trying to cache this item, then use the standard closure algorithm
    if (m_CachingClosure) {
        closure(it, state, gram);
        return;
    }
    
    // Mark this item as caching
    m_CachingClosure = true;
    
    // Fetch some information about this item
    int             itemId      = gram.identifier_for_item(item_container(const_cast<item*>(this), false));
    lr1_item_set&   cachedSet   = gram.cached_set_for_item(itemId);    
    
    // Build the cache if it isn't full yet
    if (cachedSet.empty()) {
        // Create a follow set containing the end-of-input character (which we use as a placeholder)
        item_set emptyFollow(gram);
        emptyFollow.insert(an_eoi_item_c);
        
        // Generate a fake rule for this item
        rule fakeRule(an_eoi_item_c);
        fakeRule << item_container(item_container(const_cast<item*>(this), false));
        
        // Put it into an item ($ -> ^ item [$])
        lr1_item fakeItem(&gram, rule_container(fakeRule), 0, emptyFollow);
        
        // Generate the closure for the fake item as the cached set
        lr1_item_set closed;
        closure(fakeItem, closed, gram);
        
        // Store as the cached set
        cachedSet = closed;
    }
    
    // Fill in the follow set for this item
    item_set follow(gram);
    fill_follow(follow, it, gram);
    
    // Generate the closure for this item via the cache ('$' gets substituted for the follow set)
    for (lr1_item_set::const_iterator cachedItem = cachedSet.begin(); cachedItem != cachedSet.end(); cachedItem++) {
        // Get the lookahead for this item
        const item_set& itemLookahead = (*cachedItem)->lookahead();
        
        // If it contains the empty item, generate new lookahead
        if (itemLookahead.contains(an_eoi_item_c)) {
            // Copy the lookahead
            item_set newItemLookahead = itemLookahead;
            
            // Remove the empty item
            newItemLookahead.erase(an_eoi_item_c);
            
            // Add the follow set
            newItemLookahead.merge(follow);
            
            // Add the final item to the state
            lr1_item_container newItem(new lr1_item((*cachedItem)->get_lr0_item(), newItemLookahead));
            state.insert(newItem);
        } else {
            // Can just copy this item into the final state
            state.insert(*cachedItem);
        }
    }
    
    // Caching is finished for this object
    m_CachingClosure = false;
}

/// \brief True if a transition (new state) should be generated for this item
///
/// Should return false for any item that acts like the empty item
bool item::generate_transition() const {
    // True by default
    return true;
}

/// \brief Returns NULL, or this item as an EBNF item
const ebnf* item::cast_ebnf() const {
    // This is not an EBNF item
    return NULL;
}

/// \brief Returns NULL, or this item as a guard item
const guard* item::cast_guard() const {
    // This is not a guard
    return NULL;
}
