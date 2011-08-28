//
//  conflict.h
//  Parse
//
//  Created by Andrew Hunter on 04/06/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_CONFLICT_H
#define _LR_CONFLICT_H

#include <set>
#include <vector>

#include "ContextFree/item.h"
#include "Lr/lr_item.h"
#include "Lr/lalr_builder.h"

namespace lr {
    class conflict;
    
    /// \brief Container object for a conflict
    typedef util::container<conflict> conflict_container;
    
    /// \brief A list of conflicts
    typedef std::vector<conflict_container> conflict_list;

    ///
    /// \brief Description of a LR conflict
    ///
    /// There are two kinds of possible conflict in a LR parser: shift/reduce and reduce/reduce. These occur because
    /// the parser has to be able to determine whether or not a rule is completed based on a single token of
    /// lookahead. The state machine can deal with cases where more than one rule is reached by a shift action,
    /// so these actions cannot conflict with one another.
    ///
    /// In a LALR parser, conflicts can also arise not because the grammar is ambiguous, but because the algorithm
    /// is unable to determine that a particular nonterminal's usage is unique. These conflicts always appear as
    /// reduce/reduce problems, and have the property that in any given state only one reduction will allow the
    /// parser to continue. It's possible to resolve these conflicts by replacing one of the reduce actions with
    /// a weak reduce action.
    ///
    /// Resolving conflicts can be tricky. For reductions, knowing the rule and nonterminal that is being reduced
    /// is insufficient to determine what in the grammar is causing the problem (except in sufficiently simply
    /// grammars). For this reason, this object also stores information about the states that can be reached
    /// by a particular reduction, so it is possible to find where the token will ultimately be shifted.
    ///
    class conflict {
    public:
        /// \brief Pair indicating the state and item ID of a particular LR item
        typedef lalr_builder::lr_item_id lr_item_id;
        
        /// \brief Set of possible states and items that can be reached by a reduction
        typedef std::set<lr_item_id> possible_reduce_states;
        
        /// \brief Maps LR(0) items that can be reduced by a particular 
        typedef std::map<lr0_item_container, possible_reduce_states> reduce_conflicts;
        
        /// \brief Iterator used to go through the list of reduce conflicts
        typedef reduce_conflicts::const_iterator reduce_iterator;
        
    private:
        /// \brief The ID of the parser state where this conflict occurred
        int m_StateId;
        
        /// \brief The token that is causing the conflict
        contextfree::item_container m_Token;
        
        /// \brief The LR item(s) that can be shifted, or empty if this is a reduce/reduce conflict
        ///
        /// Note that these items can never conflict with each other (the complete set just describes the
        /// state reached by the shift operation). These items only conflict with the reduce items.
        lr0_item_set m_Shift;
        
        /// \brief The LR item(s) that can be reduced with the specified token on the lookahead, and the
        /// states that can be reached when following them.
        ///
        /// (These can be determined by finding where the appropriate lookaheads were generated spontaneously,
        /// and/or could be propagated from)
        reduce_conflicts m_Reduce;
        
        /// \brief Disabled assignment
        conflict& operator=(const conflict& assignFrom);
        
    public:
        /// \brief Creates a new conflict object (describing a non-conflict)
        conflict(int stateId, const contextfree::item_container& token);
        
        /// \brief Copy constructor
        conflict(const conflict& copyFrom);
        
        /// \brief Destroys this conflict object
        virtual ~conflict();
        
        /// \brief Clones this conflict
        conflict* clone() const;
        
    public:
        /// \brief Returns true if conflict a is less than conflict b
        static bool compare(const conflict* a, const conflict* b);
        
        /// \brief Orders this conflict relative to another
        bool operator<(const conflict& compareTo) const;
        
    public:
        /// \brief Adds an LR(0) item that will be followed if the token is shifted
        void add_shift_item(const lr0_item_container& item);
        
        /// \brief Adds an LR(0) item that will be reduced if the token is in the lookahead
        ///
        /// This returns an item that the caller can add the set of reduce states for this item
        possible_reduce_states& add_reduce_item(const lr0_item_container& item);
        
    public:
        /// \brief The state ID where this conflict occurred
        inline int state() const { return m_StateId; }
        
        /// \brief The token that causes this conflict
        inline const contextfree::item_container& token() const { return m_Token; }
        
        /// \brief First item in the set of items that can be shifted for the token
        inline lr0_item_set::const_iterator first_shift_item() const { return m_Shift.begin(); }
        
        /// \brief The item after the final item in the set of items that can be shifted for the token
        ///
        /// If this is the same as first_shift_item, then this is a reduce/reduce conflict
        inline lr0_item_set::const_iterator last_shift_item() const { return m_Shift.end(); }
        
        /// \brief The first conflicting reduce action. There is always at least one conflicting reduce action.
        inline reduce_iterator first_reduce_item() const { return m_Reduce.begin(); }
        
        /// \brief The item after the final conflicting reduce action.
        inline reduce_iterator last_reduce_item() const { return m_Reduce.end(); }
        
    public:
        /// \brief Adds the conflicts found in the specified LALR builder object to the passed in list
        static void find_conflicts(const lalr_builder& builder, conflict_list& target);
    };
}

#endif
