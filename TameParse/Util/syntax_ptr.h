//
//  syntax_ptr.h
//  TameParse
//
//  Created by Andrew Hunter on 17/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _UTIL_SYNTAX_PTR_H
#define _UTIL_SYNTAX_PTR_H

#include <cstdlib>

namespace util {
    /// \brief Definition of a reference to a pointer of the given type
    ///
    /// This is an internal structure used by the syntax_ptr class
    struct syntax_ptr_reference {
        /// \brief Creates a reference to NULL
        syntax_ptr_reference()
        : m_UsageCount(1)
        , m_Value(NULL) {
        }
        
        /// \brief Creates a reference to a value
        syntax_ptr_reference(const void* newValue)
        : m_UsageCount(1)
        , m_Value(newValue) {
        }
        
        /// \brief Number of syntax_ptr objects that refer to this reference
        int m_UsageCount;
        
        /// \brief The value in this reference
        const void* m_Value;
        
    private:
        syntax_ptr_reference(const syntax_ptr_reference& noCopying);
        syntax_ptr_reference& operator=(const syntax_ptr_reference& noAssign);
    };

    /// \brief Shared pointer class similar to container, except without the requirements for cloning and comparisons 
    ///
    /// This will initialise to NULL if called with the default constructor. Once a given pointer is set to be managed by
    /// one of these objects, it cannot be deleted any way other than by destroying all of the referencing objects.
    template<typename ptr_type> class syntax_ptr {
    private:
        /// \brief The value that is being pointed at
        syntax_ptr_reference* m_Reference;
        
    public:
        /// \brief Constructs a syntax_ptr from a reference
        ///
        /// Generally, you should not use this constructor, it's mainly here to support pointer casting (the cast_to() function)
        inline explicit syntax_ptr(syntax_ptr_reference* ref)
        : m_Reference(ref) {
            m_Reference->m_UsageCount++;
        }
        
    public:
        /// \brief Default constructor, assigns the pointer to NULL
        inline syntax_ptr()
        : m_Reference(new syntax_ptr_reference()) {
        }
        
        /// \brief Set to a specific pointer value
        ///
        /// The pointer will be freed when this class is freed: it is invalid to assign a given pointer to more than
        /// one of these objects simultaneously.
        explicit inline syntax_ptr(const ptr_type* value)
        : m_Reference(new syntax_ptr_reference(value)) {
        }
        
        /// \brief Copy constructor
        inline syntax_ptr(const syntax_ptr<ptr_type>& copyFrom)
        : m_Reference(copyFrom.m_Reference) {
            // Increase the reference count for this object
            m_Reference->m_UsageCount++;
        }
        
        /// \brief Assignment
        syntax_ptr<ptr_type>& operator=(const syntax_ptr<ptr_type>& assignFrom) {
            // Nothing to do if the reference is the same
            if (m_Reference == assignFrom.m_Reference) return *this;
            
            // Deallocate the reference
            m_Reference->m_UsageCount--;
            if (m_Reference->m_UsageCount <= 0) {
                delete (ptr_type*) m_Reference->m_Value;
                m_Reference->m_Value = NULL;
                delete m_Reference;
            }
            m_Reference = NULL;
            
            // Switch to the reference in the other object
            m_Reference = assignFrom.m_Reference;
            m_Reference->m_UsageCount++;
            
            return *this;
        }
        
        /// \brief Destructs a syntax_ptr
        ~syntax_ptr() {
            m_Reference->m_UsageCount--;
            if (m_Reference->m_UsageCount <= 0) {
                delete (ptr_type*) m_Reference->m_Value;
                m_Reference->m_Value = NULL;
                delete m_Reference;
                m_Reference = NULL;
            }
        }
        
    public:
        /// \brief Converts this object back to its underlying type
        inline operator const ptr_type*() const { return (ptr_type*) m_Reference->m_Value; }

        /// \brief Evaluating this as a boolean returns whether or not the item is present
        inline operator bool() const { return m_Reference->m_Value != NULL; }
        
        /// \brief Casts this pointer to a pointer of a different type (but maintains reference counting)
        ///
        /// Note that the destructor will be called on an object of the type of the last syntax_ptr to be destroyed.
        /// That is, this call should only be made on objects with virtual destructors and between classes which are
        /// part of the same hierarchy.
        template<typename cast_type> inline syntax_ptr<cast_type> cast_to() const {
            return syntax_ptr<cast_type>(m_Reference);
        }

        // Other operators
        
        inline const ptr_type* operator->() const { return (ptr_type*) m_Reference->m_Value; }
        inline const ptr_type& operator*() { return *(ptr_type*) m_Reference->m_Value; }
        
        inline const ptr_type* item() const { return (ptr_type*) m_Reference->m_Value; }
    };
}

#endif
