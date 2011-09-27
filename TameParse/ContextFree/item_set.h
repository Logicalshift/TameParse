//
//  item_set.h
//  TameParse
//
//  Created by Andrew Hunter on 27/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _CONTEXTFREE_ITEM_SET_H
#define _CONTEXTFREE_ITEM_SET_H

#include "TameParse/Util/container.h"

namespace contextfree {
    /// \brief Forward declaration of the grammar class
    class grammar;
    
    class item;
    class empty_item_constructor;
    typedef util::container<item, empty_item_constructor> item_container;
    
    ///
    /// \brief Class representing a set of context-free items
    ///
    class item_set {
    private:
        /// \brief The grammar that these items come from
        const grammar* m_Grammar;
        
        /// \brief The number of items in the m_Items array (1/32nd of the maximum item ID that can be stored in this set)
        int m_MaxItem;

        /// \brief The size of this set (number of bits set)
        size_t m_Size;
        
        /// \brief Bits indicating which items are in this set (bit n = item n from the grammar)
        unsigned int* m_Items;
        
    public:
        /// \brief Item set representing the empty set (cannot be modified, not associated with a grammar)
        static const item_set empty_set;

        /// \brief Creates an empty item set for the specified grammar
        explicit item_set(const grammar& grammar);
        
        /// \brief Creates an empty item set for the specified grammar
        explicit item_set(const grammar* grammar);
        
        /// \brief Creates a copy of this item set
        item_set(const item_set& copyFrom);
        
        /// \brief Assigns the content of another set to this one
        item_set& operator=(const item_set& assignFrom);
        
        /// \brief Destructor
        ~item_set();

    private:
        /// \brief Recalculates the size (number of items) in this object
        void count_size();
        
    public:
        /// \brief Adds the item with the specified identifier to this item.
        ///
        /// This will return true if the item was not already in the set.
        bool insert(int itemId);
        
        /// \brief Adds a new item to this set
        ///
        /// This will return true if the item was not already in the set.
        bool insert(const item_container& newItem);

        /// \brief Removes the specified item
        ///
        /// Returns true if the item was in the set
        bool erase(int itemId);

        /// \brief Removes the specified item
        ///
        /// Returns true if the item was in the set
        bool erase(const item_container& oldItem);

        /// \brief Merges this item set with another
        bool merge(const item_set& mergeWith);

    public:
        /// \brief The number of items in this set
        inline size_t size() const { return m_Size; }

        /// \brief True if this set is empty
        inline bool empty() const { return m_MaxItem == 0; }

        /// \brief True if this set contains the specified item
        bool contains(int itemId) const;

        /// \brief True if this set contains the specified item
        bool contains(const item_container& newItem) const;

    public:
        /// \brief Returns true if this item set is equal to another
        inline bool operator==(const item_set& compareTo) const {
            // Compare the m_MaxItem first
            if (compareTo.m_MaxItem != m_MaxItem) return false;

            // Compare the elements
            for (int item = 0; item < m_MaxItem; item++) {
                if (m_Items[item] != compareTo.m_Items[item]) return false;
            }

            // Sets match
            return true;
        }

        /// \brief Returns true if this item set is not equal to another
        inline bool operator!=(const item_set& compareTo) const { return !operator==(compareTo); }

        /// \brief Orders this set relative to another
        inline bool operator<(const item_set& compareTo) const {
            // Compare the m_MaxItem first
            if (m_MaxItem < compareTo.m_MaxItem) return true;
            if (compareTo.m_MaxItem < m_MaxItem) return false;

            // Compare the elements
            for (int item = 0; item < m_MaxItem; item++) {
                if (m_Items[item] < compareTo.m_Items[item]) return true;
                if (m_Items[item] > compareTo.m_Items[item]) return false;
            }

            // Sets match
            return false;
        }

        /// \brief Orders this set relative to another
        inline bool operator>(const item_set& compareTo) const { return compareTo.operator<(*this); }

        /// \brief Orders this set relative to another
        inline bool operator>=(const item_set& compareTo) const { return !operator<(compareTo); }

        /// \brief Orders this set relative to another
        inline bool operator<=(const item_set& compareTo) const { return !compareTo.operator<(*this); }

    private:
        /// \brief Finds the item ID following the specified ID
        ///
        /// Returns m_MaxSet<<5 if the item is the last in the set
        int next_item_id(int itemId) const;

    public:
        class const_iterator;
        friend class const_iterator;

        /// \brief Iterator for the item set
        class const_iterator {
        private:
            friend class item_set;

            /// \brief The item set that this iterator belongs to
            const item_set& m_ItemSet;

            /// \brief The current item ID
            int m_CurItemId;

        private:
            const_iterator(const item_set& set, int firstItem)
            : m_ItemSet(set)
            , m_CurItemId(firstItem) {
            }

        public:
            /// \brief Moves this iterator on
            inline const_iterator& operator++() {
                // Move on to the next item
                m_CurItemId = m_ItemSet.next_item_id(m_CurItemId);
                return *this;
            }

            /// \brief Returns the current item
            const item_container& operator*();

            /// \brief Returns the current item
            inline const item_container& operator->() { return operator*(); }

            // \brief Compares this iterator to another
            inline bool operator==(const const_iterator& compareTo) const {
                return compareTo.m_CurItemId == m_CurItemId;
            }

            // \brief Compares this iterator to another
            inline bool operator!=(const const_iterator& compareTo) const { return !operator==(compareTo); }
        };

        /// \brief The iterator type is the same as the const_iterator type
        typedef const_iterator iterator;

        /// \brief The first item in this set
        const_iterator begin() const;

        /// \brief The item after the last item in this set
        inline const_iterator end() const {
            return const_iterator(*this, m_MaxItem<<5);
        }
    };
}

#include "TameParse/ContextFree/item.h"

#endif
