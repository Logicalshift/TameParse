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
            
            /// \brief Placeholder item indicating the follow set or lookahead for the current rule
            follow,
            
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
        static inline bool compare(const item& a, const item& b) {
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
            
            // For well-known kinds, we can just compare the symbols
            int aSym = a.symbol();
            int bSym = b.symbol();
            
            return aSym < bSym;
        }
    };
    
    /// \brief Class used to contain a reference to an item
    class item_container {
    private:
        item* m_Item;
        
    public:
        /// \brief Dereferences the content of this container
        inline item* operator->() {
            return m_Item;
        }

        /// \brief Dereferences the content of this container
        inline const item* operator->() const {
            return m_Item;
        }
        
        /// \brief Dereferences the content of this container
        inline item& operator*() {
            return *m_Item;
        }
        
        /// \brief Dereferences the content of this container
        inline const item& operator*() const {
            return *m_Item;
        }
        
        /// \brief Dereferences the content of this container
        inline operator item*() {
            return m_Item;
        }
        
        /// \brief Dereferences the content of this container
        inline operator const item*() const {
            return m_Item;
        }
        
        /// \brief Ordering operator
        inline bool operator<(const item_container& compareTo) const {
            return item::compare(m_Item, compareTo.m_Item);
        }
        
        /// \brief Comparison operator
        inline bool operator==(const item_container& compareTo) const {
            if (m_Item == compareTo.m_Item)                 return true;
            if (m_Item == NULL || compareTo.m_Item == NULL) return false;
            
            return (*m_Item) == *compareTo;
        }

    public:
        /// \brief Creates a new container
        inline item_container(const item& it) {
            m_Item = it.clone();
        }

        /// \brief Creates a new container
        inline item_container(const item* it) {
            if (it) {
                m_Item = it->clone();
            } else {
                m_Item = NULL;
            }
        }
        
        /// \brief Creates a new container
        inline item_container(const item_container& copyFrom) {
            m_Item = copyFrom->clone();
        }
        
        /// \brief Assigns the content of this container
        inline item_container& operator=(const item_container& assignFrom) {
            if (&assignFrom == this) return *this;
            
            delete m_Item;
            m_Item = NULL;
            if (assignFrom.m_Item) {
                m_Item = assignFrom.m_Item->clone();
            }
            return *this;
        }

        /// \brief Deletes the item in this container
        inline ~item_container() {
            delete m_Item;
        }
    };
    
    /// \brief Set of items
    typedef std::set<item_container> item_set;
    
    /// \brief Ordered list of items
    typedef std::vector<item_container> item_list;
    
    /// \brief Maps items to a type of value (item_map<T>::type gives the map type, this structure is useless by itself)
    template<typename Value> struct item_map { typedef std::map<item_container, Value> type; };
};

#endif
