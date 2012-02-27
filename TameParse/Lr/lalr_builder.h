//
//  lalr_builder.h
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#ifndef _LR_LALR_BUILDER_H
#define _LR_LALR_BUILDER_H

#include <map>
#include <set>

#include "TameParse/Util/container.h"
#include "TameParse/ContextFree/grammar.h"
#include "TameParse/Lr/lalr_machine.h"
#include "TameParse/Lr/lr_action.h"
#include "TameParse/ContextFree/terminal_dictionary.h"

namespace lr {
    /// \brief Forward declaration of the action rewriter class
    class action_rewriter;
    
    /// \brief Class that can contain an action rewriter
    typedef util::container<action_rewriter> action_rewriter_container;

    /// \brief List of action rewriters to apply when producing the final set of actions for a parser
    typedef std::vector<action_rewriter_container> action_rewriter_list;
    
    ///
    /// \brief Class that builds up a LALR state machine from a grammar
    ///
    class lalr_builder {
    public:
        /// \brief State ID, item ID pair (identifies an individual item within a state machine)
        struct lr_item_id {
            inline lr_item_id(int newState, int newItem)
            : state_id(newState)
            , item_id(newItem) {
            }

            int state_id;
            int item_id;

            inline bool operator<(const lr_item_id& compareTo) const {
                if (state_id < compareTo.state_id)  return true;
                if (state_id != compareTo.state_id) return false;

                return item_id < compareTo.item_id;
            }

            inline bool operator==(const lr_item_id& compareTo) const {
                return state_id == compareTo.state_id && item_id == compareTo.item_id;
            }

            inline bool operator!=(const lr_item_id& compareTo) const { return !operator==(compareTo); }
        };
        
        /// \brief Maps states to lists of propagations (target state, target item ID)
        typedef std::map<lr_item_id, std::set<lr_item_id> > propagation;
        
        /// \brief Set of LR(0) items that represent a closure of a LALR state
        typedef std::set<lr0_item_container> closure_set;

        /// \brief Pair mapping a source LR item to a target LR item
        typedef std::pair<lr_item_id, lr_item_id> source_to_target;

        /// \brief Maps 
        typedef std::map<source_to_target, contextfree::item_set> spontaneous_lookahead;

    private:
        /// \brief The grammar that this builder will use
        contextfree::grammar* m_Grammar;
        
        /// \brief The terminal dictionary for the language in this builder
        contextfree::terminal_dictionary* m_Terminals;
        
        /// \brief The LALR state machine that this is building up
        ///
        /// We store only the kernel states here.
        lalr_machine m_Machine;
        
        /// \brief List of action rewriter objects
        action_rewriter_list m_ActionRewriters;
        
        /// \brief Where lookaheads propagate for each item in the state machine
        ///
        /// Maps from the state, item where lookaheads should propagate from to the set of states and items where they
        /// should propagate to.
        mutable propagation m_Propagate;
        
        /// \brief Where lookaheads were spontaneously generated
        ///
        /// Maps from the state and item whose closure generated a spontaneous lookahead to the state and item where the
        /// lookahead ended up. (This isn't quite enough to see what the lookahead generated was)
        mutable propagation m_Spontaneous;

        /// \brief Maps from pairs of items (representing source and target) to the lookahead that was spontaneously generated for them
        mutable spontaneous_lookahead m_SpontaneousLookahead;
        
        /// \brief Maps state IDs to sets of LR actions
        mutable std::map<int, lr_action_set> m_ActionsForState;
        
        /// \brief Maps the ID of guard rules to their initial state (if they generate an accepting action, then the guard is matched)
        std::map<int, int> m_StatesForGuard;
        
        lalr_builder(const lalr_builder& copyFrom);
        lalr_builder& operator=(const lalr_builder& copyFrom);
        
    public:
        /// \brief Creates a new builder for the specified grammar
        ///
        /// The grammar and terminal dictionary are stored as references, not copied into this object. This object will
        /// become invalid if they are freed before it is.
        lalr_builder(contextfree::grammar& gram, contextfree::terminal_dictionary& terminals);
        
        /// \brief Adds an initial state to this builder that will recognise the language specified by the supplied symbol
        ///
        /// To build a valid parser, you need to add at least one symbol. The builder will add a new state that recognises
        /// this language
        int add_initial_state(const contextfree::item_container& language);
        
        /// \brief Finishes building the parser (the LALR machine will contain a LALR parser after this call completes)
        void complete_parser();
        
        /// \brief Generates the lookaheads for the parser (when the machine has been built up as a LR(0) grammar)
        void complete_lookaheads();
        
        /// \brief The LALR state machine being built up by this object
        lalr_machine& machine() { return m_Machine; }
        
        /// \brief The LALR state machine being built up by this object
        const lalr_machine& machine() const { return m_Machine; }
        
        /// \brief The grammar used for this builder
        const contextfree::grammar& gram() const { return *m_Grammar; }
        
        /// \brief The terminal dictionary for this builder
        const contextfree::terminal_dictionary& terminals() const { return *m_Terminals; }
        
        /// \brief Adds a new action rewriter to this builder
        void add_rewriter(const action_rewriter_container& rewriter);
        
        /// \brief Replaces the rewriters that this builder will use
        void set_rewriters(const action_rewriter_list& list);
        
        /// \brief Adds the closure of the specified LALR state to the specified set
        ///
        /// In order to generate actions for a state, the closure needs to be made in order to discover actions
        /// caused by empty productions. This is also required to display (or resolve) shift/reduce conflicts
        static void generate_closure(const lalr_state& state, lr1_item_set& closure, const contextfree::grammar* gram);
        
    public:
        /// \brief Returns the number of states in the state machine
        inline int count_states() const { return m_Machine.count_states(); }
        
        /// \brief After the state machine has been completely built, returns the actions for the specified state
        ///
        /// If there are conflicts, this will return multiple actions for a single symbol.
        const lr_action_set& actions_for_state(int state) const;
        
        /// \brief Returns the items that the lookaheads are propagated to for a particular item in this state machine
        const std::set<lr_item_id>& propagations_for_item(int state, int item) const;

        /// \brief Returns the items that the item in the specified state generates spontaneous lookaheads for
        const std::set<lr_item_id>& spontaneous_for_item(int state, int item) const;

        /// \brief Returns the lookahead generated spontaneously from a particular item to a particular item
        const contextfree::item_set& lookahead_for_spontaneous(int sourceState, int sourceItem, int destState, int destItem);

        /// \brief Finds the set of items that were used in the generation of the lookahead for the specified item
        ///
        /// This is used to help with resolving reduction conflicts: if you know where a particular terminal symbol comes from,
        /// it is possible to see why a conflict exists. Pass in the state and item ID that the lookahead was generated for, and
        /// the lookahead symbol that generated the conflict, and this will add the items where the lookahead was generated to
        /// the set. This is the set of states that are reached by a reduction on the specified symbol.
        void find_lookahead_source(int state, int item, contextfree::item_container lookaheadItem, std::set<lr_item_id>& sourceItems) const;

        /// \brief Computes the closure of a LALR state
        static void create_closure(closure_set& target, const lalr_state& state, const contextfree::grammar* gram);
        
    private:
        /// \brief Adds guard actions appropriate for the specified guard item
        void add_guard(const contextfree::item_container& item, lr_action_set& newSet) const;
    };
}

#include "action_rewriter.h"

#endif
