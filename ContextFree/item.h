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

#include "Util/container.h"

namespace lr {
    /// \brief Forward declaration of an LR(0) item
    class lr1_item;
    
    /// \brief Class that can contain an reference to a LR(0) item
    typedef util::container<lr1_item> lr1_item_container;
    
    /// \brief Set of LR(0) items
    typedef std::set<lr1_item_container> lr1_item_set;    
}

namespace contextfree {
    /// \brief Forward declaration of a container for items
    class item;
    typedef util::container<item> item_container;
    
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
            
            /// \brief The 'end of input' item
            eoi,
            
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
        
        /// \brief Computes the closure of this rule in the specified grammar
        ///
        /// This is the set of spontaneously generated LR(1) items for this item, and is used to generate the closure when
        /// producing a parser. This call is supplied the item for which the closure is being generated, and a set of states
        /// to which new items can be added (and the grammar so rules can be looked up).
        ///
        /// A spontaneously generated rule is one that is implied by this item. For example, if parser is trying to match the
        /// nonterminal 'X', then the rules for that nonterminal are spontaneously generated.
        virtual void closure(const lr::lr1_item& item, lr::lr1_item_set& state, const grammar& gram) const;
        
        /// \brief True if a transition (new state) should be generated for this item
        ///
        /// Should return false for any item that acts like the empty item
        virtual bool generate_transition();
        
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
};

#include "ContextFree/standard_items.h"
#include "ContextFree/ebnf_items.h"
#include "ContextFree/rule.h"

#endif
