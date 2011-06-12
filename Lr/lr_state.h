//
//  lr_state.h
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_LR_STATE_H
#define _LR_LR_STATE_H

#include <set>
#include "Util/container.h"
#include "Lr/lr_item.h"

namespace lr {
    ///
    /// \brief Functor used in cases where LR states do not need to be merged
    ///
    template<typename item_type> class state_no_merge {
    public:
        inline bool operator()(item_type& mergeInto, const item_type& mergeFrom) {
            return false;
        }
    };
    
    ///
    /// \brief Representation of a generic LR state
    ///
    /// item_type represents the type of the LR item (for example, lr0_item)
    /// compare_items represents how an item is compared
    /// merge_items represents how an item is merged with an identical item (a merge operation must not affect the item ordering)
    ///
    /// A state is essentially just a unique collection of items. There are two different representations for a state in a LR
    /// parser. The 'kernel' representation is the set of initial states generated by a particular transition. The 'closure'
    /// is the kernel with any productions required by items matching a nonterminal added.
    ///
    /// For example, consider this language:
    ///
    ///     A -> a B
    ///     B -> b c
    ///
    /// The kernel of a state might be as follows: { A -> a * B }. This state is trying to match the nonterminal 'B', so the
    /// closure of this state will include the production for that nonterminal. In other words, the closure of this state will
    /// be { A -> a * B, B -> * b c }. Things get a little more complicated with LR(1) states, as these have lookaheads
    /// associated with them, but the principle is still roughly the same.
    ///
    template<typename item_type, typename compare_items = std::less<item_type>, typename merge_items = state_no_merge<item_type> > class lr_state {
    public:
        /// \brief Container for an item
        typedef util::container<item_type> container;
        
        /// \brief Item container comparator type
        typedef typename container::template compare_adapter<compare_items> comparator;
        
        /// \brief Set of items within a state, mapped to identifiers (which count from 0)
        typedef std::map<container, int, comparator> item_set;
        
        /// \brief List of items
        typedef std::vector<container> item_list;
        
        /// \brief Iterator that can be used to retrieve the contents of this state (maps to a pair of the item and its identifier)
        ///
        /// TODO: could use a boost transform iterator here to hide the (pointless) identifier and make code more
        /// portable (it's really rubbish that the STL can't just do this)
        typedef typename item_set::const_iterator set_iterator;

        /// \brief Iterator that can be used to retrieve the contents of this state
        typedef typename item_list::const_iterator all_iterator;
        
    private:
        /// \brief The items making up the kernel of this state
        item_set m_ItemsToIdentifier;
        
        /// \brief Number of items in the kernel set
        int m_NumKernel;
        
        /// \brief The items that form the closure of this set
        item_set m_ClosureItems;
        
        /// \brief A list of items (index is the identifier)
        item_list m_ItemList;
        
        /// \brief The identifier for this state if set (-1 if not)
        ///
        /// Note that the identifier is not used when comparing states
        int m_Identifier;
        
    public:
        /// \brief Constructs an empty state
        lr_state() : m_Identifier(-1), m_NumKernel(0) { }
        
        lr_state(const lr_state& copyFrom)
        : m_ItemList(copyFrom.m_ItemList)
        , m_ItemsToIdentifier(copyFrom.m_ItemsToIdentifier)
        , m_ClosureItems(copyFrom.m_ClosureItems)
        , m_Identifier(-1)
        , m_NumKernel(copyFrom.m_NumKernel) {
        }
        
    public:
        /// \brief Equality operator
        bool operator==(const lr_state& compareTo) const {
            if (m_NumKernel != compareTo.m_NumKernel) return false;
            
            static comparator less_than;
            
            // Two sets are equal if their items are the same, but identifiers are allowed to be different
            for (typename item_set::const_iterator ours = m_ItemsToIdentifier.begin(), theirs = compareTo.m_ItemsToIdentifier.begin(); 
                 ours != m_ItemsToIdentifier.end() && theirs != compareTo.m_ItemsToIdentifier.end(); 
                 ours++, theirs++) {
                // Not equal if either greater than or less that this item,
                if (less_than(ours->first, theirs->first) || less_than(theirs->first, ours->first)) return false;
            }
            
            return true;
        }
        
        /// \brief Ordering operator
        bool operator<(const lr_state& compareTo) const { 
            if (m_NumKernel < compareTo.m_NumKernel) return true;
            if (m_NumKernel > compareTo.m_NumKernel) return false;

            static comparator less_than;

            for (typename item_set::const_iterator ours = m_ItemsToIdentifier.begin(), theirs = compareTo.m_ItemsToIdentifier.begin(); 
                 ours != m_ItemsToIdentifier.end() && theirs != compareTo.m_ItemsToIdentifier.end();
                 ours++, theirs++) {
                // Not equal if either greater than or less that this item,
                if (less_than(ours->first, theirs->first)) return true;
                if (less_than(theirs->first, ours->first)) return false;
            }
            
            // Equal if we reach here
            return false;
        }

        inline bool operator!=(const lr_state& compareTo) const { return !operator==(compareTo); }
        inline bool operator>(const lr_state& compareTo) const { return !operator<(compareTo) && !operator==(compareTo); }
        inline bool operator<=(const lr_state& compareTo) const { return operator<(compareTo) || operator==(compareTo); }
        inline bool operator>=(const lr_state& compareTo) const { return !operator<(compareTo); }
        
    public:
        /// \brief Clones this state
        lr_state* clone() { return new lr_state(*this); }
        
        /// \brief Comparison function
        static inline bool compare(const lr_state& a, const lr_state& b) { return a < b; }

        /// \brief Comparison function
        static inline bool compare(const lr_state* a, const lr_state* b) { 
            if (a == b) return false;
            if (!a) return true;
            if (!b) return false;
            
            return a < b;
        }
        
    public:
        /// \brief Adds a new item to this object. Returns the identifier for the new/existing item
        int add(const container& newItem, bool isKernel) {
            // Pick which item set the new item should be added to
            item_set& sourceSet = isKernel ? m_ItemsToIdentifier : m_ClosureItems;
            
            // Try to find an existing example of this item
            typename item_set::iterator found = sourceSet.find(newItem);
            
            static merge_items do_merge;
            
            if (found == sourceSet.end()) {
                // For closure items, do not add new items if this is already in the kernel set
                if (!isKernel) {
                    if (m_ItemsToIdentifier.find(newItem) != m_ItemsToIdentifier.end()) {
                        // Merge with the existing kernel item
                        return add(newItem, true);
                    }
                }
                
                // Just add the item if it wasn't found
                int         newItemId = (int)m_ItemList.size();
                container   newContainer(newItem);
                
                if (isKernel) m_NumKernel++;
                
                m_ItemList.push_back(newContainer);
                sourceSet[newContainer] = newItemId;
                                     
                return newItemId;
            } else {
                // Merge the items if it already exists
                int         itemId  = found->second;
                item_type   merged  = *found->first;
                
                // Replace the old item if the merge was successful
                if (do_merge(merged, *newItem)) {
                    container mergedItem(merged);
                    
                    sourceSet.erase(found);
                    sourceSet[mergedItem] = itemId;
                    m_ItemList[itemId] = mergedItem;
                    
                    return itemId;
                } else {
                    return itemId;
                }
            }
        }

    public:
        /// \brief The identifier for this state, or -1 if it's not set
        inline int identifier() const { return m_Identifier; }
        
        /// \brief Changes the identifier for this state
        inline void set_identifier(int newId) { m_Identifier = newId; }
        
        /// \brief The first kernel item in this state
        inline set_iterator begin_kernel() const { return m_ItemsToIdentifier.begin(); }
        
        /// \brief The final kernel item in this state
        inline set_iterator end_kernel() const { return m_ItemsToIdentifier.end(); }

        /// \brief The first item in this state
        inline all_iterator begin_all() const { return m_ItemList.begin(); }
        
        /// \brief The final item in this state
        inline all_iterator end_all() const { return m_ItemList.end(); }
        
        /// \brief Finds the item identifier in this set that matches the specified item (or -1 if the item isn't in this state)
        inline int find_identifier(const container& target) const {
            typename item_set::const_iterator found = m_ItemsToIdentifier.find(target);
            if (found != m_ItemsToIdentifier.end()) return found->second;
            else return -1;
        }
        
        /// \brief Number of items in this state (kernel or non-kernel)
        inline size_t count_items() const { return m_ItemList.size(); }
        
        /// \brief Item with the specified identifier
        inline const container& operator[](int identifier) const { return m_ItemList[identifier]; }
    };
}

#endif
