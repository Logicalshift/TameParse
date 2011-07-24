//
//  lr_action.h
//  Parse
//
//  Created by Andrew Hunter on 04/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_LR_ACTION_H
#define _LR_LR_ACTION_H

#include <set>

#include "ContextFree/item.h"
#include "ContextFree/rule.h"
#include "Util/container.h"

namespace lr {
    ///
    /// \brief Description of an action in a LR parser
    ///
    /// There are three basic actions a LR parser can perform:
    ///
    ///  * SHIFT: move the lookahead symbol and current state onto the stack
    ///  * REDUCE: pop a fixed number of symbols, then look up a goto action for the state on top of the stack
    ///  * IGNORE: ignore a symbol that has no meaning (eg, whitespace or comments)
    ///  * GOTO: push a nonterminal symbol to the stack and go to a particular state
    ///
    /// We extend this in a couple of ways. The most important of these is the idea of a 'weak' reduce, which can
    /// be used to resolve LALR conflicts. This tries to perform one or more reduction actions until it reaches
    /// a point where the lookahead symbol is shifted or rejected. If the lookahead is rejected in the final
    /// state, then the parser restores its original state and tries another action.
    ///
    /// The second idea is that of a 'guard' symbol. An action referring to a guard evaluates a condition, and
    /// treats the guard symbol as the lookahead if the condition is matched (following a different path). This
    /// makes it possible for an ambiguous grammar to be parsed by being specific about which productions should
    /// be matched for a given lookahead.
    ///
    class lr_action {
    public:
        /// \brief Types of LR action
        enum action_type {
            /// \brief If the terminal is seen, then it is placed on the stack and the next terminal is read
            act_shift,
            
            /// \brief If the terminal is seen, discard it and look at the next one
            act_ignore,
            
            /// \brief If the terminal is seen, then the parser reduces by the specified rule
            /// 
            /// That is, pops the items in the rule, then pushes the nonterminal that the rule reduces to, and finally looks up the goto action
            /// for the resulting nonterminal in the state on top of the stack.
            act_reduce,
            
            /// \brief Works as for reduce, except that the parser does not perform this action if the symbol won't be shifted after the reduce
            ///
            /// If there is a weak reduce and a reduce action for a given symbol, then the weak reduce action is tried first. The parser should
            /// look at the state that will be reached by popping the stack and see where the goto leads to. If it would produce another reduction,
            /// it should continue looking there. If it would produce a shift action, then it should perform this reduction. If it would produce an
            /// error, then it should try other actions.
            ///
            /// This can be used to resolve reduce/reduce conflicts and hence allow a LALR parser to parse full LR(1) grammars. It is also useful
            /// if you want to support the concept of 'weak' lexical symbols (whose meaning depends on context), as a weak reduction is only possible
            /// if the lookahead symbol is a valid part of the language.
            act_weakreduce,
            
            /// \brief Identical to 'reduce', except the target symbol is the root of the language
            act_accept,
            
            /// \brief If a phrase has been reduced to this nonterminal symbol, then goto to the specified state
            act_goto,
            
            /// \brief If the terminal is seen, then the parser moves directly to the specified state (and it is left as lookahead)
            ///
            /// This is used when generating actions for guard symbols, specifically when the parser detects there is no conflict
            /// and so can always assume that the guard is successful based on a single symbol of lookahead.
            act_divert,
            
            /// \brief If the terminal is seen, then the specified guard rule should be evaluated.
            ///
            /// If the guard rule is accepted, then the guard symbol is set as the lookahead (this is retrieved from the rule
            /// that is reduced)
            act_guard,
        };
        
    private:
        /// \brief The type of action that this represents
        action_type m_Type;
        
        /// \brief The item that this refers to. For shift or reduce actions this will be a terminal. For goto actions, this will be a nonterminal
        contextfree::item_container m_Item;
        
        /// \brief The state to enter if this item is seen
        int m_NextState;
        
        /// \brief The rule that this refers to.
        contextfree::rule_container m_Rule;
        
        /// \brief Disabled assignment
        lr_action& operator=(const lr_action& assignFrom);
        
    public:
        /// \brief Creates a shift or goto action (with no rule)
        lr_action(action_type type, const contextfree::item_container& item, int nextState);
        
        /// \brief Creates a reduce action (with a rule to reduce)
        lr_action(action_type type, const contextfree::item_container& item, int nextState, const contextfree::rule_container& rule);
        
        /// \brief Copy constructor
        lr_action(const lr_action& copyFrom);
        
        /// \brief Copy constructor, modifies the item for an action
        lr_action(const lr_action& copyFrom, const contextfree::item_container& newItem);
        
        /// \brief Orders this action
        inline bool operator<(const lr_action& compareTo) const {
            if (m_Item < compareTo.m_Item)              return true;
            if (compareTo.m_Item < m_Item)              return false;
            if (m_Type < compareTo.m_Type)              return true;
            if (m_Type > compareTo.m_Type)              return false;
            if (m_NextState < compareTo.m_NextState)    return true;
            if (m_NextState > compareTo.m_NextState)    return false;
            if (m_Rule < compareTo.m_Rule)              return true;
            
            return false;
        }
        
        /// \brief Determines whether or not this action is the same as another
        inline bool operator==(const lr_action& compareTo) const {
            return m_Type == compareTo.m_Type && m_NextState == compareTo.m_NextState && m_Item == compareTo.m_Item && m_Rule == compareTo.m_Rule;
        }
        
        /// \brief The type of action that this represents
        inline action_type type() const { return m_Type; }
        
        /// \brief The item that this refers to. For shift or reduce actions this will be a terminal. For goto actions, this will be a nonterminal
        inline const contextfree::item_container& item() const { return m_Item; }
        
        /// \brief The state to enter if this item is seen
        inline int next_state() const { return m_NextState; }
        
        /// \brief The rule that this refers to.
        inline const contextfree::rule_container& rule() const { return m_Rule; }
        
        /// \brief Clones an existing action
        inline lr_action* clone() const {
            return new lr_action(*this);
        }
        
        /// \brief Compares two actions
        inline static bool compare(const lr_action* a, const lr_action* b) {
            if (a == b) return false;
            if (!a) return true;
            if (!b) return false;
            
            return *a < *b;
        }
    };
    
    /// \brief LR action container
    typedef util::container<lr_action> lr_action_container;
    
    /// \brief Set of LR actions
    typedef std::set<lr_action_container> lr_action_set;
}

#endif
