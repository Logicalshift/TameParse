//
//  lalr_machine.h
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_LALR_MACHINE_H
#define _LR_LALR_MACHINE_H

#include <set>
#include <map>
#include <vector>

#include "Util/container.h"
#include "ContextFree/item.h"
#include "ContextFree/grammar.h"
#include "Lr/lalr_state.h"

namespace lr {
    ///
    /// \brief Class representing the state machine for a LALR parser
    ///
    /// The state machine for LR(1) and SLR parsers has a very similar structure to this. A future revision of this library 
    /// may supply a state machine template so that these kinds of parser can be easily produced.
    ///
    /// This class is primarily concerned with representing the data associated with the state machine for a LALR parser.
    ///
    class lalr_machine {
    public:
        /// \brief State container
        typedef util::container<lalr_state> container;
        
        /// \brief Maps unique states to identifiers
        ///
        /// Updating lookahead or identifiers in state doesn't change them, so it's safe to do this
        typedef std::map<container, int> state_to_identifier;
        
        /// \brief List of states
        typedef std::vector<container> state_list;
        
        /// \brief Representation of a state transition
        typedef std::pair<contextfree::item_container, int> transition;

        /// \brief Set of transitions
        typedef std::map<contextfree::item_container, int> transition_set;
        
        /// \brief Transition for each state
        typedef std::vector<transition_set> transition_for_state;
        
        /// \brief Set of LR(0) items that represent a closure of a LALR state
        typedef std::set<lr0_item_container> closure_set;
        
    private:
        /// \brief The grammar for this state machine
        contextfree::grammar* m_Grammar;
        
        /// \brief Maps states to identifiers
        state_to_identifier m_StateIds;
        
        /// \brief List of states. The index in this list corresponds to the identifier in m_StateIds
        state_list m_States;
        
        /// \brief Transition for each state in this machine
        transition_for_state m_Transitions;
        
    public:
        /// \brief Creates an empty LALR machine, which will reference the specified gramamr
        lalr_machine(contextfree::grammar& gram);
        
    public:
        /// \brief Adds a new state to this machine, or retrieves the identifier for the existing state
        ///
        /// This returns the identifier for the state, and the state is updated so that its identifier 
        /// reflects this. The state should not have any more lr1_items added to it, but it is permissable
        /// to alter the lookahead set after calling this.
        ///
        /// Typically, a state will be added with no lookahead set, and the lookahead will be added later.
        /// (An algorithm that builds the LALR parser from an LR(1) set might do this differently, though)
        ///
        /// A closure will be calculated for the state as it is being added.
        ///
        int add_state(container& newState);
        
        /// \brief Adds a transition to this state machine
        ///
        /// Transitions involving terminals create shift actions in the final parser. Nonterminals and EBNF
        /// items go into the goto table for the final parser. The empty item should be ignored. Guard items
        /// are a little weird: they act like shift actions if they are matched.
        void add_transition(int stateId, const contextfree::item_container& item, int newStateId);
        
        /// \brief Adds the specified set of lookahead items to the state with the supplied ID and returns true if the lookahead changed
        bool add_lookahead(int stateId, const lr0_item& item, const contextfree::item_set& newLookahead);
        
        /// \brief Adds the specified set of lookahead items to the state with the supplied ID and returns true if the lookahead changed
        bool add_lookahead(int stateId, int itemId, const contextfree::item_set& newLookahead);
        
    private:
        /// \brief Creates the closure for a particular lalr state
        static void create_closure(closure_set& target, const lalr_state& state, const contextfree::grammar* gram);
        
    public:
        /// \brief Class used to iterate through a set of states (or a container with NULL in it if this doesn't exist)
        typedef state_list::const_iterator state_iterator;
        
        /// \brief Returns the state with the specified identifier
        inline const container& state_with_id(int identifier) const {
            static const container empty;
            if (identifier < 0 || identifier >= m_States.size()) return empty;
            
            return m_States[identifier];
        }
        
        /// \brief The set of transitions for a state with the specified identifier
        inline const transition_set& transitions_for_state(int stateId) const {
            static const transition_set empty;
            if (stateId < 0 || stateId >= m_Transitions.size()) return empty;
            
            return m_Transitions[stateId];
        }
        
        /// \brief Counts the number of states in this machine
        inline int count_states() const { return (int)m_States.size(); }
        
        /// \brief The first state in this machine
        inline state_iterator first_state() const { return m_States.begin(); }
        
        /// \brief The position after the final state in this machine
        inline state_iterator last_state() const { return m_States.end(); }
        
        /// \brief The grammar used for this state machine
        inline contextfree::grammar& gram() const { return *m_Grammar; }
    };
}

#endif
