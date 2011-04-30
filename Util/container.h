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
    /// \brief Class used as a container for other classes
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
            
            inline reference() {
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
        
    public:
        /// \brief Default constructor (creates a reference to a NULL item)
        inline container() {
            m_Ref = new reference(NULL, true);
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
