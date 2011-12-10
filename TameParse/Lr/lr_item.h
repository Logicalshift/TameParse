//
//  lr_item.h
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
//

#ifndef _LR_LR_ITEM_H
#define _LR_LR_ITEM_H

#include "TameParse/Util/container.h"
#include "TameParse/ContextFree/grammar.h"
#include "TameParse/ContextFree/item_set.h"

namespace lr {
    /// \brief Forward declaration of an LR(0) item
    class lr0_item;

    /// \brief Forward declaration of an LR(1) item
    class lr1_item;
    
    /// \brief Container for an LR(0) item
    typedef util::container<lr0_item> lr0_item_container;
    
    /// \brief Container for an LR(1) item
    typedef util::container<lr1_item> lr1_item_container;
    
    /// \brief Set of LR(0) items
    typedef std::set<lr0_item_container> lr0_item_set;
    
    ///
    /// \brief Representation of an LR(0) item
    ///
    /// An LR(0) item stores no lookahead. These items are suitable for producing SLR(1) parsing tables,
    /// when combined with an algorithm for generating the follow set for a production.
    ///
    /// An LR(0) item consists of a rule and an offset in that rule (which represents the 'position' in
    /// the input stream. They are typically written in a manner similar to 'X -> a * B c'. Here the
    /// rule is 'X -> a B c', and the position indicates that the parser is in a state where it has
    /// just matched the 'a' symbol and is attempting to match the 'B' symbol.
    ///
    class lr0_item {
    private:
        /// \brief The grammar that the rule was defined in
        const contextfree::grammar* m_Grammar;
        
        /// \brief The rule for this item
        contextfree::rule_container m_Rule;
        
        /// \brief The offset into the rule
        int m_Offset;
        
    public:
        /// \brief Creates an LR(0) item by copying a rule
        lr0_item(const contextfree::grammar* gram, const contextfree::rule& rule, int offset);
        
        /// \brief Creates an LR(0) item by referencing an existing rule
        lr0_item(const contextfree::grammar* gram, contextfree::rule* rule, int offset);
        
        /// \brief Creates an LR(0) item by referencing an existing rule
        lr0_item(const contextfree::grammar* gram, const contextfree::rule_container& rule, int offset);
        
        /// \brief Creates a copy of an existing LR(0) item
        lr0_item(const lr0_item& copyFrom);
        
        /// \brief Creates a copy of an existing LR(0) item with a different offset
        lr0_item(const lr0_item& copyFrom, int newOffset);
        
        /// \brief Copies an LR(0) item into this one
        lr0_item& operator=(const lr0_item& copyFrom);
        
        /// \brief Orders an LR(0) item
        bool operator<(const lr0_item& compareTo) const;
        
        /// \brief Compares two LR(0) items
        bool operator==(const lr0_item& compareTo) const;
        
    public:
        /// \brief The grammar that the rule is represented in
        inline const contextfree::grammar& gram() const { return *m_Grammar; }
        
        /// \brief The rule for this item
        inline const contextfree::rule_container& rule() const { return m_Rule; }
        
        /// \brief The offset for this item
        inline int offset() const { return m_Offset; }
        
        /// \brief True if this item is at the end of the rule (ie, is in a reducing state)
        inline bool at_end() const { return offset() >= (int) rule()->items().size(); }

    public:
        /// \brief Clones an LR(0) item
        inline lr0_item* clone() const { return new lr0_item(*this); }

        inline bool operator!=(const lr0_item& compareTo) const { return !operator==(compareTo); }
        inline bool operator>(const lr0_item& compareTo) const  { return compareTo.operator<(*this); }
        inline bool operator<=(const lr0_item& compareTo) const { return !operator>(compareTo); }
        inline bool operator>=(const lr0_item& compareTo) const { return !operator<(compareTo); }
        
        /// \brief Static ordering operator
        static inline bool compare(const lr0_item* a, const lr0_item* b) {
            if (a == b) return false;
            if (!a) return true;
            if (!b) return false;
            
            return *a < *b;
        }
    };
    
    ///
    /// \brief Representation of an LR(1) item
    ///
    /// An LR(1) item is a LR(0) item with the addition of a set of lookahead symbols, which represent
    /// the set of symbols that may follow the rule in the parser.
    ///
    /// These items are typically written in the form 'X -> a * B c, d/e', which indicates that the
    /// parser has matched the 'a' symbol, and that the symbol 'X' is followed by the symbols d or e
    /// in the context in which it appears (that is, there's a rule looking something like 'Y -> X d'
    /// somwhere in the parser).
    ///
    class lr1_item {
    public:
        /// \brief Type of the lookahead set for this item
        typedef contextfree::item_set lookahead_set;
        
    private:
        /// \brief An LR(0) item that forms the core of this item
        lr0_item_container m_Lr0Item;
        
        /// \brief The set of lookahead items for this item
        lookahead_set m_LookAhead;
        
    public:
        /// \brief Constructs an LR(1) item by appending lookahead to an LR(0) item
        lr1_item(const lr0_item_container& core, const lookahead_set& lookahead);

        /// \brief Constructs an LR(1) item by appending lookahead to an LR(0) item
        lr1_item(const lr0_item& core, const lookahead_set& lookahead);
        
        /// \brief Constructs an LR(1) item by copying a rule
        lr1_item(const contextfree::grammar* gram, const contextfree::rule& rule, int offset, const lookahead_set& lookahead);
        
        /// \brief Constructs an LR(1) item by creating a reference to an existing rule
        lr1_item(const contextfree::grammar* gram, contextfree::rule* rule, int offset, const lookahead_set& lookahead);
        
        /// \brief Constructs an LR(1) item by creating a reference to an existing rule
        lr1_item(const contextfree::grammar* gram, const contextfree::rule_container& rule, int offset, const lookahead_set& lookahead);
        
        /// \brief Creates a copy of an LR(1) item
        lr1_item(const lr1_item& copyFrom);
        
        /// \brief Assigns the context of this LR(1) item from another item
        lr1_item& operator=(const lr1_item& copyFrom);
        
        /// \brief Orders an LR(1) item
        ///
        /// Items are ordered first by the LR(0) item, and then by the content of the lookahead (this makes it
        /// possible to find items in a set that differ only by their lookahead)
        bool operator<(const lr1_item& compareTo) const;
        
        /// \brief Compares two LR(1) items
        bool operator==(const lr1_item& compareTo) const;
        
    public:
        /// \brief Adds a new lookahead item (and returns true if it was successfully added)
        inline bool add_lookahead(const contextfree::item_container& lookahead) {
            return m_LookAhead.insert(lookahead);
        }

    public:
        /// \brief The grammar that the rule is represented in
        inline const contextfree::grammar& gram() const { return m_Lr0Item->gram(); }

        /// \brief The rule for this item
        inline const contextfree::rule_container& rule() const { return m_Lr0Item->rule(); }
        
        /// \brief The offset for this item
        inline int offset() const { return m_Lr0Item->offset(); }
        
        /// \brief The lookahead set for this item
        inline const lookahead_set& lookahead() const { return m_LookAhead; }
        
        /// \brief True if this item is at the end of the rule (ie, is in a reducing state)
        inline bool at_end() const { return m_Lr0Item->at_end(); }
        
    public:
        /// \brief Converts to a LR(0) item (loses lookahead information)
        inline const lr0_item& get_lr0_item() const { return *m_Lr0Item; }

        /// \brief Converts to a LR(0) item (loses lookahead information)
        inline lr0_item& get_lr0_item() { return *m_Lr0Item; }
        
        /// \brief Converts to a LR(0) item (loses lookahead information)
        inline operator lr0_item&() { return *m_Lr0Item; }

        /// \brief Converts to a LR(0) item (loses lookahead information)
        inline operator const lr0_item&() const { return *m_Lr0Item; }

        /// \brief Converts to a LR(0) item (loses lookahead information)
        inline operator lr0_item_container&() { return m_Lr0Item; }
        
        /// \brief Converts to a LR(0) item (loses lookahead information)
        inline operator const lr0_item_container&() const { return m_Lr0Item; }
        
        /// \brief Clones an LR(1) item
        inline lr1_item* clone() const { return new lr1_item(*this); }
        
        inline bool operator!=(const lr1_item& compareTo) const { return !operator==(compareTo); }
        inline bool operator>(const lr1_item& compareTo) const { return !operator<(compareTo); }
        inline bool operator<=(const lr1_item& compareTo) const { return operator<(compareTo) || operator==(compareTo); }
        inline bool operator>=(const lr1_item& compareTo) const { return !operator<(compareTo) || operator==(compareTo); }
        
        /// \brief Static ordering operator
        static inline bool compare(const lr1_item* a, const lr1_item* b) {
            if (a == b) return false;
            if (!a) return true;
            if (!b) return false;
            
            return *a < *b;
        }
    };
}

#include "TameParse/Lr/lr1_item_set.h"

#endif
