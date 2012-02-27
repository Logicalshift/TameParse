//
//  parser_tables.h
//  Parse
//
//  Created by Andrew Hunter on 07/05/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#ifndef _LR_PARSER_TABLES_H
#define _LR_PARSER_TABLES_H

#include <algorithm>

#include "TameParse/Lr/lalr_builder.h"
#include "TameParse/Lr/weak_symbols.h"
#include "TameParse/Dfa/lexer.h"

namespace lr {
    ///
    /// \brief The tables for a LR(1) parser
    ///
    class parser_tables {
    public:
        ///
        /// \brief Description of a parser action
        ///
        struct action {
            /// \brief The type of action (same values as specified by lr_action)
            unsigned int m_Type : 8;
            
            /// \brief The state to enter when this symbol is matched (or the rule to reduce for reduce actions)
            unsigned int m_NextState : 24;
            
            /// \brief The symbol ID. 
            ///
            /// For shift or reduce actions, this is the terminal identifier. For all other actions,
            /// this is the item ID as supplied by the source grammar.
            int m_SymbolId;
        };
        
        ///
        /// \brief Description of a reduce rule
        ///
        struct reduce_rule {
            /// \brief The identifier for the nonterminal that this rule reduces to (within the grammar)
            int m_Identifier;
            
            /// \brief The identifier for the rule that was reduced (within the grammar)
            int m_RuleId;
            
            /// \brief Number of items in this rule
            int m_Length;
        };
        
        /// \brief Iterator for actions
        typedef action* action_iterator;
        
        /// \brief Structure that counts the number of terminal and nonterminal actions a state
        struct action_count {
            int m_NumTerms;
            int m_NumNonterms;
        };
        
        /// \brief Structure that maps a weak symbol to its strong equivalent
        struct symbol_equivalent {
            int m_OriginalSymbol;
            int m_MappedTo;
            
            /// \brief Compares this to another symbol_equivalent object (only the original symbol is actually compared so we can use find())
            inline bool operator==(const symbol_equivalent& compareTo) const {
                return m_OriginalSymbol == compareTo.m_OriginalSymbol;
            }
            
            /// \brief Orders this structure
            inline bool operator<(const symbol_equivalent& compareTo) const { 
                return m_OriginalSymbol < compareTo.m_OriginalSymbol;
            }
        };
        
    private:
        /// \brief The number of states in this parser
        int m_NumStates;
        
        /// \brief The nonterminal symbol representing the end of input
        int m_EndOfInput;
        
        /// \brief The nonterminal symbol representing the 'end of guard' symbol
        int m_EndOfGuard;
        
        /// \brief The terminal action table (sorted in order of terminal ID)
        action** m_TerminalActions;
        
        /// \brief The nonterminal action table (goto actions, guard actions)
        action** m_NonterminalActions;
        
        /// \brief Counts the number of actions in each state
        action_count* m_Counts;
        
        /// \brief Sorted list of states that have an action on the 'end of guard' symbol (%)
        int* m_EndGuardStates;
        
        /// \brief Number of items in the EndGuardStates array
        int m_NumEndOfGuards;
        
        /// \brief The number of rules in the rules list
        int m_NumRules;
        
        /// \brief The reduce rules
        reduce_rule* m_Rules;
        
        /// \brief The number of items in the weak to strong map
        int m_NumWeakToStrong;
        
        /// \brief Ordered list of weak symbols and their strong equivalent
        symbol_equivalent* m_WeakToStrong;

        /// \brief True if this object owns the tables
        bool m_DeleteTables;
        
    public:
        /// \brief Creates a parser from the result of the specified builder class
        parser_tables(const lalr_builder& builder, const weak_symbols* weakSyms);

        /// \brief Creates a parser from a set of tables. Tables passed into this constructor will not be deleted by the destructor
        parser_tables(int numStates, int endOfInputSymbol, int endOfGuardSymbol, action** terminalActions, action** nonterminalActions, action_count* actionCounts, int* endGuardStates, int numEndGuards, int numRules, reduce_rule* reduceRules, int numWeakToStrong, symbol_equivalent* weakToStrong);

        /// \brief Copy constructor
        parser_tables(const parser_tables& copyFrom);
        
        /// \brief Assignment
        parser_tables& operator=(const parser_tables& copyFrom);

        /// \brief Destructor
        virtual ~parser_tables();
        
        /// \brief Calculates the size in bytes of these parser tables
        virtual size_t size() const;
        
    private:
        /// \brief Compares a symbol to an action
        inline static bool compare_symbols(const action& a, const action& compareTo) {
            return a.m_SymbolId < compareTo.m_SymbolId;
        }
        
        /// \brief Finds an action
        inline const action_iterator find_action(int symbol, action* actionList, int count) const {
            action compareAction;
            compareAction.m_SymbolId = symbol;

            return std::lower_bound(actionList, actionList + count, compareAction, compare_symbols);
        }
        
    public:
        /// \brief Returns the reduce rule with the specified ID
        inline const reduce_rule& rule(int ruleId) const { return m_Rules[ruleId]; }
        
        /// \brief An iterator pointing to the last action referring to a terminal symbol in the specified state
        inline action_iterator last_terminal_action(int stateId) const { 
            return m_TerminalActions[stateId] + m_Counts[stateId].m_NumTerms;
        }

        /// \brief An iterator pointing to the last action referring to a non-terminal symbol in the specified state
        inline action_iterator last_nonterminal_action(int stateId) const { 
            return m_NonterminalActions[stateId] + m_Counts[stateId].m_NumNonterms;
        }
        
        /// \brief Finds the first action that refers to a terminal with an ID equal or greater to that supplied 
        /// to this function
        inline action_iterator find_terminal(int stateId, int terminal) const {
            return find_action(terminal, m_TerminalActions[stateId], m_Counts[stateId].m_NumTerms);
        }
        
        /// \brief Finds the first action that refers to a nonterminal with an ID equal or greater to that supplied
        /// to this function
        inline action_iterator find_nonterminal(int stateId, int nonterminal) const {
            return find_action(nonterminal, m_NonterminalActions[stateId], m_Counts[stateId].m_NumNonterms);
        }
        
        /// \brief Returns the nonterminal identifier representing the end of input symbol
        inline int end_of_input() const { return m_EndOfInput; }
        
        /// \brief Returns the nonterminal identifier representing the end of guard symbol
        inline int end_of_guard() const { return m_EndOfGuard; }
        
        /// \brief Returns true if the specified state has an end of guard symbol
        inline bool has_end_of_guard(int stateId) const {
            return std::binary_search(m_EndGuardStates, m_EndGuardStates + m_NumEndOfGuards, stateId);
        }
        
        /// \brief Finds the strong symbol that is equivalent to a given weak terminal symbol
        inline int strong_for_weak(int weakTerminal) const {
            // If there are no symbols in the map, then just return the terminal symbol
            if (m_NumWeakToStrong == 0) return weakTerminal;
            
            // Look up the symbol in the map
            const symbol_equivalent     search  = { weakTerminal, 0 };
            const symbol_equivalent*    found   = std::find(m_WeakToStrong, m_WeakToStrong + m_NumWeakToStrong, search);
            if (found != m_WeakToStrong + m_NumWeakToStrong) {
                return found->m_MappedTo;
            }
            
            // Nothing was found
            return weakTerminal;
        }

    public:
        /// \brief Returns the total number of action items for the specified state
        inline int count_actions_for_state(int stateId) const {
            return m_Counts[stateId].m_NumTerms + m_Counts[stateId].m_NumNonterms;
        }

        /// \brief Returns the number of states in these tables
        inline int count_states() const { 
            return m_NumStates;
        }

    public:
        /// \brief The terminal actions table.
        ///
        /// There is one table per state, and the action_counts table specifies the number of items for any
        /// given state.
        inline const action* const* terminal_actions() const { return m_TerminalActions; }

        /// \brief The non-terminal actions table.
        ///
        /// There is one table per state, and the action_counts table specifies the number of items for any
        /// given state.
        inline const action* const* nonterminal_actions() const { return m_NonterminalActions; }

        /// \brief The action count table
        ///
        /// There is one entry per state, each entry supplies the size of the terminal and nonterminal actions for that state.
        inline const action_count* action_counts() const { return m_Counts; }

        /// \brief A sorted list of the state IDs that have an 'end of guard' symbol action
        inline const int* end_of_guard_states() const { return m_EndGuardStates; }

        /// \brief The size of the end_of_guard_states table
        inline int count_end_of_guards() const { return m_NumEndOfGuards; }

        /// \brief The number of reduce rules
        inline int count_reduce_rules() const { return m_NumRules; }

        /// \brief The reduce rules for this parser (count_reduce_rules items)
        inline const reduce_rule* reduce_rules() const { return m_Rules; }

        /// \brief Counts the number of weak to strong entries
        inline int count_weak_to_strong() const { return m_NumWeakToStrong; }

        /// \brief The weak-to-strong equivalence table (ordered, count_weak_to_strong entries)
        inline const symbol_equivalent* weak_to_strong() const { return m_WeakToStrong; }
    };
}

#endif
