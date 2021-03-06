//
//  container.h
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the \"Software\"), to 
//  deal in the Software without restriction, including without limitation the 
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
//  sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
//  IN THE SOFTWARE.
//

#ifndef _UTIL_CONTAINER_H
#define _UTIL_CONTAINER_H

#include <cstdlib>

namespace util {
    ///
    /// \brief Default constructor class for the container class
    ///
    template<typename ItemType> class simple_constructor {
    public:
        /// \brief Constructs an object of type ItemType
        inline static ItemType* construct() {
            return new ItemType();
        }
        
        /// \brief Destroys an item of type ItemType
        inline static void destruct(ItemType* item) {
            delete item;
        }
    };
    
    ///
    /// \brief Class used as a container for other classes
    ///
    /// This class actually stores a reference to an object, and copying a container will do reference counting to avoid
    /// having to copy the item or using extra memory.
    ///
    /// ItemType must implement a clone() method to create a copy of the class, and a static compare(ItemType*, ItemType*)
    /// method to order them (it should return true if the first item is less than the second).
    ///
    template<typename ItemType, typename ItemAllocator = simple_constructor<ItemType> > class container {
    private:
        /// \brief Structure used to represent a reference to an item
        struct reference {
            ItemType*       item;
            
        private:
            const bool m_WillDelete;
            mutable int m_RefCount;
            
            reference(const reference& noCopying) { }
            reference& operator=(const reference& noCopying) { }
            
        public:
            /// \brief Creates a reference to an item, with a reference count of 1. The item will be deleted if the reference count reached 0 and willDelete is true
            inline reference(ItemType* it, bool willDelete)
            : item(it)
            , m_RefCount(1)
            , m_WillDelete(willDelete) {
            }
            
            inline ~reference() {
                if (m_WillDelete && item) {
                    ItemAllocator::destruct(item);
                    item = NULL;
                }
            }
            
            /// \brief Decreases the reference count and deletes this reference if it reaches 0
            inline void release() const {
                if (m_RefCount <= 1) {
                    delete this;
                } else {
                    m_RefCount--;
                }
            }
            
            /// \brief Increases the reference count
            inline void retain() const {
                ++m_RefCount;
            }
        };
        
        /// \brief Reference to the item in this container
        reference* m_Ref;
        
    private:
        
    public:
        /// \brief Dereferences the content of this container
        inline ItemType* item() { return m_Ref->item; }

        /// \brief Dereferences the content of this container
        inline const ItemType* item() const { return m_Ref->item; }
        
        /// \brief Dereferences the content of this container
        inline ItemType* operator->() {
            return m_Ref->item;
        }
        
        /// \brief Dereferences the content of this container
        inline const ItemType* operator->() const {
            return m_Ref->item;
        }
        
        /// \brief Dereferences the content of this container
        inline ItemType& operator*() {
            return *m_Ref->item;
        }
        
        /// \brief Dereferences the content of this container
        inline const ItemType& operator*() const {
            return *m_Ref->item;
        }
        
        /// \brief Dereferences the content of this container
        inline operator ItemType*() {
            return m_Ref->item;
        }
        
        /// \brief Dereferences the content of this container
        inline operator const ItemType*() const {
            return m_Ref->item;
        }
        
        /// \brief Ordering operator
        inline bool operator<(const container& compareTo) const {
            return ItemType::compare(m_Ref->item, compareTo.m_Ref->item);
        }
        
        /// \brief Ordering operator
        inline bool operator>(const container& compareTo) const {
            return compareTo.operator<(*this);
        }
        
        /// \brief Ordering operator
        inline bool operator>=(const container& compareTo) const {
            return !operator<(*this);
        }
        
        /// \brief Ordering operator
        inline bool operator<=(const container& compareTo) const {
            return !operator>(*this);
        }
        
        /// \brief Comparison operator
        inline bool operator==(const container& compareTo) const {
            if (m_Ref->item == compareTo.m_Ref->item)                   return true;
            if (m_Ref->item == NULL || compareTo.m_Ref->item == NULL)   return false;
            
            return (*m_Ref->item) == *compareTo;
        }
        
        /// \brief Comparison operator
        inline bool operator!=(const container& compareTo) const { return !operator==(compareTo); }
        
        /// \brief Comparison functor adapter
        ///
        /// Adapts a function designed to compare two items to one to compare a container of those items
        /// Returns true if the a is less than b.
        template<typename compare_item> class compare_adapter {
        public:
            inline bool operator()(const container& a, const container& b) const {
                static compare_item less_than;
                
                if (a.m_Ref->item == b.m_Ref->item)     return false;
                if (!a.m_Ref->item)                     return true;
                if (!b.m_Ref->item)                     return false;
                
                return less_than(*a, *b);
            }
        };
        
    public:
        /// \brief Default constructor (creates a reference to a new item)
        inline container() {
            m_Ref = new reference(ItemAllocator::construct(), true);
        }
        
        /// \brief Creates a new container (clones the item)
        inline container(const ItemType& it) {
            m_Ref = new reference(it.clone(), true);
        }
        
        /// \brief Creates a new container (direct reference to an existing item)
        inline container(ItemType* it) {
            m_Ref = new reference(it, false);
        }
        
        /// \brief Creates a new container (set whether or not the item should get deleted when the container is finished with)
        inline container(ItemType* it, bool shouldDelete) {
            m_Ref = new reference(it, shouldDelete);
        }
        
        /// \brief Creates a new container (clones the item)
        inline container(const ItemType* it) {
            if (it) {
                m_Ref = new reference(it->clone(), true);
            } else {
                m_Ref = new reference(NULL, false);
            }
        }
        
        /// \brief Creates a new container
        inline container(const container<ItemType, ItemAllocator>& copyFrom) {
            m_Ref = copyFrom.m_Ref;
            m_Ref->retain();
        }
        
        /// \brief Assigns the content of this container
        inline container<ItemType, ItemAllocator>& operator=(const container<ItemType, ItemAllocator>& assignFrom) {
            if (&assignFrom == this) return *this;
            
            assignFrom.m_Ref->retain();
            m_Ref->release();
            m_Ref = assignFrom.m_Ref;

            return *this;
        }
        
        /// \brief Deletes the item in this container
        inline ~container() {
            m_Ref->release();
            m_Ref = NULL;
        }        
    };
}

#endif
