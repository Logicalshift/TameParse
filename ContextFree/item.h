//
//  item.h
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _CONTEXTFREE_ITEM_H
#define _CONTEXTFREE_ITEM_H

#include <set>
#include <map>
#include <vector>
#include <typeinfo>

namespace contextfree {
    /// \brief Forward declaration of a container for items
    class item_container;
    
    /// \brief Forward declaration of a grammar
    class grammar;
    
    /// \brief Set of items
    typedef std::set<item_container> item_set;
    
    /// \brief Ordered list of items
    typedef std::vector<item_container> item_list;
    
    /// \brief Maps items to a type of value (item_map<T>::type gives the map type, this structure is useless by itself)
    template<typename Value> struct item_map { typedef std::map<item_container, Value> type; };

    ///
    /// \brief Abstract base class for items in a rule in a context free grammar
    ///
    class item {
    private:
        /// \brief The symbol for this item
        int m_Symbol;
        
        /// \brief Copying isn't allowed
        item(const item& copyFrom);
        
    public:
        /// \brief Standard constructor
        explicit inline item(int symbol) : m_Symbol(symbol) { }
        
        /// \brief Destructor
        virtual ~item();
        
        /// \brief Compares this item to another. Returns true if they are the same
        virtual bool operator==(const item& compareTo) const;
        
        /// \brief Orders this item relative to another item
        virtual bool operator<(const item& compareTo) const;
        
        inline bool operator!=(const item& compareTo) const { return !operator==(compareTo); }
        inline bool operator<=(const item& compareTo) const { return operator<(compareTo) || operator==(compareTo); }
        inline bool operator>=(const item& compareTo) const { return !operator<(compareTo); }
        inline bool operator>(const item& compareTo) const  { return !operator<(compareTo) && !operator==(compareTo); }
        
        /// \brief Creates a clone of this item
        virtual item* clone() const = 0;
        
    public:
        ///
        /// \brief Well-known kinds of item
        ///
        /// 
        ///
        enum kind {
            /// \brief This is the empty ('epsilon') item
            ///
            /// This isn't really an item at all, but rather a representation of an absence of an item.
            empty,
            
            /// \brief This item represents a lexical symbol
            terminal,
            
            /// \brief This item represents a nonterminal production
            nonterminal,
            
            /// \brief This item represents a guard condition
            ///
            /// Guard conditions are used to make it possible to parse ambiguous grammars. If a guard
            /// condition is in the lookahead for a symbol, it is evaluated and if it is matched then
            /// the parser will take that path (otherwise it will take the path a normal LALR parser
            /// would take.
            ///
            /// This is particularly useful for things like distinguishing an angle bracket starting
            /// a list of type parameters from an angle bracket indicating a less than operator
            /// (a very common ambiguity thanks to the prevalence of C++ syntax in many languages)
            guard,
            
            /// \brief Items 0x100-0x1ff represent various kinds of EBNF item
            ebnf = 0x100,
            
            /// \brief EBNF rule representing an optional item
            optional = ebnf,
            
            /// \brief EBNF rule representing an item that can repeat (1 or more times, combine with optional for 0 or more)
            repeat = ebnf + 1,
            
            /// \brief EBNF rule representing an alternative (either one of two different rules)
            alternative = ebnf + 2,
            
            /// \brief Values of this value or greater are available for users who want to create custom items 
            ///
            /// Note that when comparing item
            other = 0x10000
        };
        
        /// \brief The type of this item
        virtual kind type() const = 0;
        
        /// \brief The symbol identifier of this item (the kind of thing that this symbol represents is specified by the type call)
        ///
        /// If type is greater than or equal to kind::other, then the meaning of the symbol is defined by the item.
        /// In all other cases, this is a reference to the definition of an item within the grammar.
        inline int symbol() const { return m_Symbol; }
        
        /// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
        ///
        /// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
        /// set for the rules that make them up.
        ///
        /// The 'empty' item indicates the first set should be extended to include anything after in the rule.
        virtual item_set first(const grammar& gram) const = 0;
        
    public:
        /// \brief Comparison function, returns true if a is less than b, by content
        static inline bool compare(const item* a, const item* b) {
            // Pointer comparision
            if (a == b) return false;
            if (!a)     return true;
            if (!b)     return false;
            
            // Compare the contents of these items
            return compare(*a, *b);
        };
        
        /// \brief Comparison function, returns true if a is less than b
        static bool compare(const item& a, const item& b);
    };
    
    /// \brief Class used to contain a reference to an item
    class item_container {
    private:
        /// \brief Structure used to represent a reference to an item
        struct item_reference {
            /// \brief Creates a reference to an item, with a reference count of 1. The item will be deleted if the reference count reached 0 and willDelete is true
            inline item_reference(item* it, bool willDelete)
            : m_Item(it)
            , m_RefCount(1)
            , m_WillDelete(willDelete) {
            }
            
            inline ~item_reference() {
                if (m_WillDelete && m_Item) delete m_Item;
            }
            
            item*       m_Item;
            
        private:
            const bool m_WillDelete;
            mutable int m_RefCount;
            
        public:
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
        item_reference* m_Ref;
        
    public:
        /// \brief Dereferences the content of this container
        inline item* operator->() {
            return m_Ref->m_Item;
        }

        /// \brief Dereferences the content of this container
        inline const item* operator->() const {
            return m_Ref->m_Item;
        }
        
        /// \brief Dereferences the content of this container
        inline item& operator*() {
            return *m_Ref->m_Item;
        }
        
        /// \brief Dereferences the content of this container
        inline const item& operator*() const {
            return *m_Ref->m_Item;
        }
        
        /// \brief Dereferences the content of this container
        inline operator item*() {
            return m_Ref->m_Item;
        }
        
        /// \brief Dereferences the content of this container
        inline operator const item*() const {
            return m_Ref->m_Item;
        }
        
        /// \brief Ordering operator
        inline bool operator<(const item_container& compareTo) const {
            return item::compare(m_Ref->m_Item, compareTo.m_Ref->m_Item);
        }
        
        /// \brief Comparison operator
        inline bool operator==(const item_container& compareTo) const {
            if (m_Ref->m_Item == compareTo.m_Ref->m_Item)                   return true;
            if (m_Ref->m_Item == NULL || compareTo.m_Ref->m_Item == NULL)   return false;
            
            return (*m_Ref->m_Item) == *compareTo;
        }

    public:
        /// \brief Creates a new container (clones the item)
        inline item_container(const item& it) {
            m_Ref = new item_reference(it.clone(), true);
        }

        /// \brief Creates a new container (direct reference to an existing item)
        inline item_container(item* it) {
            if (it) {
                m_Ref = new item_reference(it, false);
            } else {
                m_Ref = NULL;
            }
        }

        /// \brief Creates a new container (clones the item)
        inline item_container(const item* it) {
            if (it) {
                m_Ref = new item_reference(it->clone(), true);
            } else {
                m_Ref = NULL;
            }
        }
        
        /// \brief Creates a new container
        inline item_container(const item_container& copyFrom) {
            m_Ref = copyFrom.m_Ref;
            m_Ref->retain();
        }
        
        /// \brief Assigns the content of this container
        inline item_container& operator=(const item_container& assignFrom) {
            if (&assignFrom == this) return *this;
            
            assignFrom.m_Ref->retain();
            m_Ref->release();
            m_Ref = assignFrom.m_Ref;

            return *this;
        }

        /// \brief Deletes the item in this container
        inline ~item_container() {
            m_Ref->release();
        }
    };
};

#endif

#include "ContextFree/standard_items.h"
#include "ContextFree/ebnf_items.h"
#include "ContextFree/rule.h"
