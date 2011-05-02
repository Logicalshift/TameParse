//
//  container.h
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _UTIL_CONTAINER_H
#define _UTIL_CONTAINER_H

#include <cstdlib>

namespace util {
    ///
    /// \brief Class used as a container for other classes
    ///
    /// This class actually stores a reference to an object, and copying a container will do reference counting to avoid
    /// having to copy the item or using extra memory.
    ///
    /// ItemType must implement a clone() method to create a copy of the class, and a static compare(ItemType, ItemType)
    /// method to order them (it should return true if the first item is less than the second).
    ///
    template<typename ItemType> class container {
    private:
        /// \brief Structure used to represent a reference to an item
        struct reference {
            ItemType*       m_Item;
            
        private:
            const bool m_WillDelete;
            mutable int m_RefCount;
            
        public:
            /// \brief Creates a reference to an item, with a reference count of 1. The item will be deleted if the reference count reached 0 and willDelete is true
            inline reference(ItemType* it, bool willDelete)
            : m_Item(it)
            , m_RefCount(1)
            , m_WillDelete(willDelete) {
            }
            
            inline ~reference() {
                if (m_WillDelete && m_Item) delete m_Item;
            }
            
            /// \brief Decreases the reference count and deletes this reference if it reaches 0
            inline void release() const {
                if (m_RefCount < 0) {
                    delete this;
                }
                else {
                    m_RefCount--;
                }
            }
            
            /// \brief Increases the reference count
            inline void retain() const {
                m_RefCount++;
            }
        };
        
        /// \brief Reference to the item in this container
        reference* m_Ref;
        
    public:
        /// \brief Dereferences the content of this container
        inline ItemType* operator->() {
            return m_Ref->m_Item;
        }
        
        /// \brief Dereferences the content of this container
        inline const ItemType* operator->() const {
            return m_Ref->m_Item;
        }
        
        /// \brief Dereferences the content of this container
        inline ItemType& operator*() {
            return *m_Ref->m_Item;
        }
        
        /// \brief Dereferences the content of this container
        inline const ItemType& operator*() const {
            return *m_Ref->m_Item;
        }
        
        /// \brief Dereferences the content of this container
        inline operator ItemType*() {
            return m_Ref->m_Item;
        }
        
        /// \brief Dereferences the content of this container
        inline operator const ItemType*() const {
            return m_Ref->m_Item;
        }
        
        /// \brief Ordering operator
        inline bool operator<(const container& compareTo) const {
            return ItemType::compare(m_Ref->m_Item, compareTo.m_Ref->m_Item);
        }
        
        /// \brief Comparison operator
        inline bool operator==(const container& compareTo) const {
            if (m_Ref->m_Item == compareTo.m_Ref->m_Item)                   return true;
            if (m_Ref->m_Item == NULL || compareTo.m_Ref->m_Item == NULL)   return false;
            
            return (*m_Ref->m_Item) == *compareTo;
        }
        
        /// \brief Comparison functor adapter
        ///
        /// Adapts a function designed to compare two items to one to compare a container of those items
        /// Returns true if the a is less than b.
        template<typename compare_item> class compare_adapter {
        public:
            inline bool operator()(const container& a, const container& b) const {
                static compare_item less_than;
                
                if (a.m_Ref->m_Item == b.m_Ref->m_Item) return false;
                if (!a.m_Ref->m_Item)                   return true;
                if (!b.m_Ref->m_Item)                   return false;
                
                return less_than(*a, *b);
            }
        };
        
    public:
        /// \brief Default constructor (creates a reference to a new item)
        inline container() {
            m_Ref = new reference(new ItemType(), true);
        }
        
        /// \brief Creates a new container (clones the item)
        inline container(const ItemType& it) {
            m_Ref = new reference(it.clone(), true);
        }
        
        /// \brief Creates a new container (direct reference to an existing item)
        inline container(ItemType* it) {
            if (it) {
                m_Ref = new reference(it, false);
            } else {
                m_Ref = NULL;
            }
        }
        
        /// \brief Creates a new container (clones the item)
        inline container(const ItemType* it) {
            if (it) {
                m_Ref = new reference(it->clone(), true);
            } else {
                m_Ref = NULL;
            }
        }
        
        /// \brief Creates a new container
        inline container(const container<ItemType>& copyFrom) {
            m_Ref = copyFrom.m_Ref;
            m_Ref->retain();
        }
        
        /// \brief Assigns the content of this container
        inline container<ItemType>& operator=(const container<ItemType>& assignFrom) {
            if (&assignFrom == this) return *this;
            
            assignFrom.m_Ref->retain();
            m_Ref->release();
            m_Ref = assignFrom.m_Ref;
            
            return *this;
        }
        
        /// \brief Deletes the item in this container
        inline ~container() {
            m_Ref->release();
        }        
    };
}

#endif
