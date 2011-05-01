//
//  lalr_state.h
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_LALR_STATE_H
#define _LR_LALR_STATE_H

#include "Lr/lr_state.h"
#include "Lr/lr_item.h"

namespace lr {
    ///
    /// \brief Representation of a state in a LALR parser
    ///
    /// LALR states are LR(1) states where the lookahead isn't used to distinguish them in the state machine.
    ///
    class lalr_state {
    public:
        ///
        /// \brief Comparator that compares two LR(1) states by only looking at their LR(0) equivalent
        ///
        class compare_lr0 {
        public:
            inline bool operator()(const lr1_item& a, const lr1_item& b) {
                return static_cast<const lr0_item&>(a) < static_cast<const lr0_item&>(b);
            }
        };
        
        ///
        /// \brief Merging class that merges the lookaheads for two LR(1) states
        ///
        class merge_lalr {
        public:
            inline bool operator()(lr1_item& target, const lr1_item& mergeWith) {
                // Merge the lookahead sets for these two items
                bool changed = false;
                
                for (lr1_item::lookahead_set::const_iterator it = mergeWith.lookahead().begin(); it != mergeWith.lookahead().end(); it++) {
                    if (target.add_lookahead(*it)) changed = true;
                }
                
                return changed;
            }
        };
        
    public:
        /// \brief Instantiation of lr_state required to represent a LALR state
        typedef lr_state<lr1_item, compare_lr0, merge_lalr> state;
        
        /// \brief Container class
        typedef state::container container;
        
        /// \brief Iterator for accessing the items in this state
        typedef state::iterator iterator;
        
    private:
        /// \brief The underlying state of this item
        state m_State;
        
    public:
        /// \brief Constructs an empty state
        lalr_state();
        
        /// \brief Copies this state
        lalr_state(lalr_state& copyFrom);
        
        bool operator<(const lalr_state& compareTo) const;
        bool operator==(const lalr_state& compareTo) const;
        
        inline bool operator!=(const lalr_state& compareTo) const { return !operator==(compareTo); }
        inline bool operator>(const lalr_state& compareTo) const { return !operator<(compareTo) && !operator==(compareTo); }
        inline bool operator<=(const lalr_state& compareTo) const { return operator<(compareTo) || operator==(compareTo); }
        inline bool operator>=(const lalr_state& compareTo) const { return !operator<(compareTo); }
        
    public:
        /// \brief Clones this state
        inline lalr_state* clone() { return new lalr_state(*this); }
        
        /// \brief Comparison function
        static inline bool compare(const lalr_state& a, const lalr_state& b) { return a < b; }
        
        /// \brief Comparison function
        static inline bool compare(const lalr_state* a, const lalr_state* b) { 
            if (a == b) return false;
            if (!a) return true;
            if (!b) return false;
            
            return a < b;
        }
        
    public:
        /// \brief Adds a new item to this object. Returns true if the operation modified this container
        bool add(const container& newItem);
        
        /// \brief The first item in this state
        iterator begin() const;
        
        /// \brief The final item in this state
        iterator end() const;
    };
}

#endif
