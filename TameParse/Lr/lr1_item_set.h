//
//  lr1_item_set.h
//  Parse
//
//  Created by Andrew Hunter on 10/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_LR1_ITEM_SET_H
#define _LR_LR1_ITEM_SET_H

#include <set>

#include "TameParse/Lr/lr_item.h"
#include "TameParse/ContextFree/item_set.h"

namespace lr {
    ///
    /// \brief Class used to store sets of LR(1) items
    ///
    /// A standard set can't provide the right behaviour for this. LR(1) items consist of two parts: the LR(0)
    /// 'kernel', and its lookahead. When adding a new item, if an existing item has the same kernel, the set
    /// should just modify the lookahead of that item rather than 
    ///
    class lr1_item_set {
    public:
        ///
        /// \brief Comparison object that compares only the LR(0) kernel of a LR(1) item
        ///
        /// This is used so that items that differ only by lookahead aren't duplicated in LR(1) item sets.
        ///
        class kernel_only_comparison {
        public:
            /// \brief Compares the LR(0) items contained by two LR(1) items
            inline bool operator()(const lr1_item_container& first, const lr1_item_container& second) {
                return first->get_lr0_item() < second->get_lr0_item();
            }
            
        };
        
        /// \brief Set type used for storage by lr1_item_set
        typedef std::set<lr1_item_container, kernel_only_comparison> internal_set;
        
        /// \brief Iterates through a LR(1) item set
        typedef internal_set::iterator iterator;
        
        /// \brief Iterates through a LR(1) item set
        typedef internal_set::const_iterator const_iterator;
        
        /// \brief Type of a lookahead set
        typedef lr1_item::lookahead_set lookahead_set;
        
    private:
        /// \brief The items stored in this set
        internal_set m_Data;
        
    public:
        /// \brief Adds a new item to this set
        std::pair<iterator, bool> insert(lr1_item_container newItem) {
            // Search for an existing item that looks like the new item
            iterator existing = m_Data.lower_bound(newItem);
            
            // If the existing item has the same LR(0) item, then we just need to extend the lookahead
            if (existing != m_Data.end() && (*existing)->get_lr0_item() == newItem->get_lr0_item()) {
                // Merge the lookaheads
                // If there are any new lookahead items, then we need to merge the lookahead sets
                // TODO: is there a quicker way of detecting when the sets are similar?
                
                // If the sets are the same, then this item has already been added and there's nothing more to do
                if (newItem->lookahead().size() == (*existing)->lookahead().size()) {
                    if (newItem->lookahead() == (*existing)->lookahead()) {
                        return std::pair<iterator, bool>(existing, false);
                    }
                }
                
                // Merge the sets, and update the item if the size changes
                lookahead_set   mergedLookahead((*existing)->lookahead());
                size_t          initialSize = mergedLookahead.size();
                
                mergedLookahead.merge(newItem->lookahead());
                
                // If the merged set is the same size as before then there have been no changes (these lookahead items already existed)
                if (mergedLookahead.size() == initialSize) {
                    return std::pair<iterator, bool>(existing, false);
                }

                // Replace the existing item with the merged item
                lr1_item_container mergedItem(new lr1_item(&newItem->gram(), newItem->rule(), newItem->offset(), mergedLookahead), true);
                
                // TODO: is there a fast way of doing this (existing is already a pointer to the place that the
                // replacement needs to be placed)
                m_Data.erase(existing);
                return m_Data.insert(mergedItem);
            }
            
            // No item has the same LR(0) kernel, so just replace the existing item
            existing = m_Data.insert(existing, newItem);
            return std::pair<iterator, bool>(existing, true);
        }
        
        /// \brief The first LR(1) item in the set
        inline iterator begin() { return m_Data.begin(); }
        
        /// \brief The first LR(1) item in the set
        inline const_iterator begin() const { return m_Data.begin(); }
        
        /// \brief The item after the last LR(1) item in the set
        inline iterator end() { return m_Data.end(); }
        
        /// \brief The item after the last LR(1) item in the set
        inline const_iterator end() const { return m_Data.end(); }
        
        /// \brief True if the set is empty
        inline bool empty() const { return m_Data.empty(); }
        
        /// \brief The total number of items in the set
        inline size_t size() const { return m_Data.size(); }
        
        /// \brief Clears this set
        inline void clear() { m_Data.clear(); }
    };
}

#endif
