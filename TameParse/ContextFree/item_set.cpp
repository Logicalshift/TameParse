//
//  item_set.cpp
//  TameParse
//
//  Created by Andrew Hunter on 27/09/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#include <cstdlib>

#include "TameParse/ContextFree/item_set.h"

using namespace contextfree;

/// \brief Item set representing the empty set (cannot be modified)
const item_set item_set::empty_set(NULL);

/// \brief Creates an empty item set for the specified grammar
item_set::item_set(const grammar& grammar) 
: m_Grammar(&grammar)
, m_MaxItem(0)
, m_Size(0)
, m_Items(NULL) {
}

/// \brief Creates an empty item set for the specified grammar
item_set::item_set(const grammar* grammar)
: m_Grammar(grammar)
, m_MaxItem(0)
, m_Size(0)
, m_Items(NULL) {
}

/// \brief Creates a copy of this item set
item_set::item_set(const item_set& copyFrom) 
: m_Grammar(copyFrom.m_Grammar)
, m_MaxItem(copyFrom.m_MaxItem)
, m_Size(copyFrom.m_Size)
, m_Items(NULL) {
	// Fill in the items
	if (m_MaxItem > 0) {
		m_Items = (unsigned int*) malloc(sizeof(unsigned int)*m_MaxItem);

		for (int item = 0; item < m_MaxItem; ++item) {
			m_Items[item] = copyFrom.m_Items[item];
		}
	}
}

/// \brief Assigns the content of another set to this one
item_set& item_set::operator=(const item_set& assignFrom) {
	// Nothing to do if we're assigning to ourself
	if (&assignFrom == this) return *this;

	// Set the grammar
	m_Grammar = assignFrom.m_Grammar;

	// Update the number of items
	m_MaxItem 	= assignFrom.m_MaxItem;
	m_Size		= assignFrom.m_Size;
	m_Items		= (unsigned int*) realloc(m_Items, sizeof(unsigned int)*m_MaxItem);

	for (int item=0; item < m_MaxItem; ++item) {
		m_Items[item] = assignFrom.m_Items[item];
	}
    
    return *this;
}

/// \brief Destructor
item_set::~item_set() {
	// Finished with the items
	if (m_Items) {
		free(m_Items);
	}
}

 /// \brief Recalculates the size (number of items) in this object
void item_set::count_size() {
	// Size is initially 0
	m_Size = 0;

	// Iterate through the items
	for (int item = 0; item < m_MaxItem; ++item) {
		// Count the number of bits set in this item (Kerningham's method)
		unsigned int bits = m_Items[item];
		while (bits) {
			++m_Size;
			bits &= bits-1;
		}
	}
}

/// \brief Adds a new item to an item set
bool item_set::insert(const item_container& newItem) {
    int itemId = m_Grammar->identifier_for_item(newItem);
    return insert(itemId);
}

/// \brief Adds the item with the specified identifier to this item.
///
/// This will return true if the item was not already in the set.
bool item_set::insert(int itemId) {
	// Get the set and bit the item is in
    int setId 	= itemId >> 5;
    int bit		= itemId&0x1f;

    // Allocate space if necessary
    if (setId >= m_MaxItem) {
    	int oldMax 	= m_MaxItem;
    	m_MaxItem 	= setId + 1;
    	m_Items 	= (unsigned int*) realloc(m_Items, sizeof(unsigned int)*m_MaxItem);

    	for (int item = oldMax; item < m_MaxItem; ++item) m_Items[item] = 0;
    }

    // Create the mask
    unsigned int mask = 1u << bit;

    // Update the bit
    if ((m_Items[setId]&mask) == 0) {
    	m_Items[setId] |= mask;
    	++m_Size;
    	return true;
    } else {
    	return false;
    }
}

/// \brief Removes the specified item
///
/// Returns true if the item was in the set
bool item_set::erase(int itemId) {
	// Get the set and bit the item is in
    int setId 	= itemId >> 5;
    int bit		= itemId&0x1f;

    // Allocate space if necessary
    if (setId >= m_MaxItem) {
        return false;
    }

    // Create the mask
    unsigned int mask = 1u << bit;

    // Update the bit
    if ((m_Items[setId]&mask) != 0) {
    	m_Items[setId] &= ~mask;
    	m_Size--;
    	return true;
    } else {
    	return false;
    }	
}

/// \brief Removes the specified item
///
/// Returns true if the item was in the set
bool item_set::erase(const item_container& oldItem) {
	int itemId = m_Grammar->identifier_for_item(oldItem);
    return erase(itemId);
}

/// \brief Merges this item set with another
bool item_set::merge(const item_set& mergeWith) {
	// Resize the item set if necessary
	if (mergeWith.m_MaxItem > m_MaxItem) {
		int oldMax 	= m_MaxItem;
    	m_MaxItem 	= mergeWith.m_MaxItem;
    	m_Items 	= (unsigned int*) realloc(m_Items, sizeof(unsigned int)*m_MaxItem);

    	for (int item = oldMax; item < m_MaxItem; ++item) m_Items[item] = 0;
	}

	// Remember if the size has changed
	bool sizeChanged = false;

	// Iterate through the items in the set we're merging with
	for (int item = 0; item < mergeWith.m_MaxItem; ++item) {
		// Get the items in this set
		unsigned int bits = mergeWith.m_Items[item];

		// Trivially ignore the case where the bits are zero
		if (!bits) continue;

		// If the same sets of bits are set in both, then there is nothing to do
		if ((bits & ~m_Items[item]) == 0) continue;

		// The number of bits in this item will have changed
		sizeChanged 	= true;
		m_Items[item] 	|= bits;
	}

	// Recalculate the size if it has changed
	if (sizeChanged) {
		count_size();
	}

	return sizeChanged;
}

/// \brief True if this set contains the specified item
bool item_set::contains(int itemId) const {
	// Get the set and bit the item is in
    int setId 	= itemId >> 5;
    int bit		= itemId&0x1f;

    // Doesn't contain this set if it's out of range of the items we have
    if (setId >= m_MaxItem || setId < 0) {
    	return false;
    }

    // Check this bit
    return (m_Items[setId] & (1<<bit)) != 0;
}

/// \brief True if this set contains the specified item
bool item_set::contains(const item_container& newItem) const {
    int itemId = m_Grammar->identifier_for_item(newItem);
    return contains(itemId);
}

/// \brief The first item in this set
item_set::const_iterator item_set::begin() const {
    // Fairly trivial if the set is empty 
    if (m_MaxItem == 0) {
        return const_iterator(*this, 0);
    }

    // Need to find the first item
    if (m_Items[0]&1) {
    	return const_iterator(*this, 0);
    } else {
    	return const_iterator(*this, next_item_id(0));
    }
}

/// \brief Finds the item ID following the specified ID
///
/// Returns m_MaxSet<<5 if the item is the last in the set
int item_set::next_item_id(int itemId) const {
	// Get the set and the bit of this item
	int 			set 	= itemId>>5;
	int 			bit 	= itemId&0x1f;
	unsigned int 	mask	= 1<<bit;

	while (set < m_MaxItem) {
		// Move on rapidly if this set is empty
		if (bit == 0 && m_Items[set] == 0) {
			++set;
            
            // Check the first bit of the new set
            if (set < m_MaxItem && (m_Items[set]&1) != 0) {
                return (set<<5);
            }
		}

		// Test this bit
		else {
            // Move on to the next item
            ++bit;
            mask <<= 1;
            
            if (bit >= 0x20) {
                mask 	= 1;
                bit 	= 0;
                ++set;
                
                if (set >= m_MaxItem) {
                    return set<<5;
                }
            }

            // Test this bit
			if ((m_Items[set]&mask) != 0) {
				// Found the next item
				return (set<<5) | bit;
			}
		}
	}

	// No item found
	return m_MaxItem<<5;
}

/// \brief Returns the current item
const item_container& item_set::const_iterator::operator*() {
    return m_ItemSet.m_Grammar->item_with_identifier(m_CurItemId);
}
