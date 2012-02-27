//
//  lalr_state.h
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
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

#ifndef _LR_LALR_STATE_H
#define _LR_LALR_STATE_H

#include <vector>

#include "TameParse/Util/container.h"
#include "TameParse/ContextFree/grammar.h"
#include "TameParse/Lr/lr_state.h"
#include "TameParse/Lr/lr_item.h"

namespace lr {
    ///
    /// \brief Representation of a state in a LALR parser
    ///
    /// LALR states are LR(1) states where the lookahead isn't used to distinguish them in the state machine.
    ///
    class lalr_state {
    public:
        /// \brief Instantiation of lr_state required to represent a LALR state
        typedef lr_state<lr0_item> state;
        
        /// \brief Container class
        typedef state::container container;
        
        /// \brief Iterator for accessing the items in this state
        typedef state::iterator iterator;
        
        /// \brief Maps item IDs to LR(1) lookahead sets
        typedef std::vector<lr1_item::lookahead_set*> lookahead_for_item;
        
    private:
        /// \brief The underlying state of this item
        state m_State;
        
        /// \brief Contains the lookahead sets for this item
        lookahead_for_item m_Lookahead;
        
        /// \brief Disabled assignment
        lalr_state& operator=(const lalr_state& assignFrom);
        
    public:
        /// \brief Constructs an empty state
        lalr_state();
        
        /// \brief Copies this state
        lalr_state(const lalr_state& copyFrom);
        
        /// \brief Destroys this state
        virtual ~lalr_state();
        
        bool operator<(const lalr_state& compareTo) const;
        bool operator==(const lalr_state& compareTo) const;
        
        inline bool operator!=(const lalr_state& compareTo) const { return !operator==(compareTo); }
        inline bool operator>(const lalr_state& compareTo) const  { return compareTo.operator<(*this); }
        inline bool operator<=(const lalr_state& compareTo) const { return !operator>(compareTo); }
        inline bool operator>=(const lalr_state& compareTo) const { return !operator<(compareTo); }
        
    public:
        /// \brief Clones this state
        inline lalr_state* clone() const { return new lalr_state(*this); }
        
        /// \brief Comparison function
        static inline bool compare(const lalr_state& a, const lalr_state& b) { return a < b; }
        
        /// \brief Comparison function
        static inline bool compare(const lalr_state* a, const lalr_state* b) { 
            if (a == b) return false;
            if (!a) return true;
            if (!b) return false;
            
            return *a < *b;
        }
        
    public:
        /// \brief Adds a new LR(0) item to this object. Returns true if the operation modified this container
        int add(const container& newItem, const contextfree::grammar* gram);
        
        /// \brief Finds the identifier for the specified LR(0) item
        int find_identifier(const container& item) const;
        
        /// \brief Returns the lookahead set for the item with the specified ID
        lr1_item::lookahead_set& lookahead_for(int identifier);
        
        /// \brief Returns the lookahead set for the item with the specified ID
        const lr1_item::lookahead_set& lookahead_for(int identifier) const;
        
        /// \brief Returns the lookahead set for the specified item, or NULL if the item is not part of this state
        inline lr1_item::lookahead_set* lookahead_for(const container& item) {
            int ident = find_identifier(item);
            if (ident == -1) return NULL;
            return &lookahead_for(ident);
        }

        /// \brief Returns the lookahead set for the specified item, or NULL if the item is not part of this state
        inline const lr1_item::lookahead_set* lookahead_for(const container& item) const {
            int ident = find_identifier(item);
            if (ident == -1) return NULL;
            return &lookahead_for(ident);
        }

        /// \brief The identifier for this state, or -1 if it's not set
        inline int identifier() const { return m_State.identifier(); }
        
        /// \brief Changes the identifier for this state
        inline void set_identifier(int newId) { m_State.set_identifier(newId); }
        
        /// \brief The first item in this state
        iterator begin() const;
        
        /// \brief The final item in this state
        iterator end() const;
        
        /// \brief Number of items in this state
        inline int count_items() const { return m_State.count_items(); }
        
        /// \brief Item with the specified identifier
        inline const container& operator[](int identifier) const { return m_State[identifier]; }
    };
    
    /// \brief Container for LALR states
    typedef util::container<lalr_state> lalr_state_container;
}

#endif
