//
//  weak_symbols.h
//  Parse
//
//  Created by Andrew Hunter on 06/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_WEAK_SYMBOLS_H
#define _LR_WEAK_SYMBOLS_H

#include <set>
#include <map>

#include "TameParse/Dfa/ndfa.h"
#include "TameParse/ContextFree/item.h"
#include "TameParse/ContextFree/terminal_dictionary.h"
#include "TameParse/Lr/action_rewriter.h"

namespace lr {
    ///
    /// \brief Action rewriter that is used to deal with 'weak' lexical symbols
    ///
    /// A 'weak' lexical symbol is one that is only matched in the context in which it is present. If the symbol is not
    /// present, it has a 'strong' equivalent that is matched against instead.
    ///
    /// This rewriter can be used with a list of weak symbols and their equivalent. In any state with an action referring
    /// to a strong symbol, it will add equivalent actions for the weak symbols that it maps to (provided there are no
    /// actions for those symbols).
    ///
    /// If a state contains a reduce action for a weak symbol and an action for a strong symbol (shift or reduce), it will
    /// be substituted with a weak reduce action and another action mirroring the action for the strong symbol.
    ///
    /// Actions which do not refer to strong symbols will not be changed.
    ///
    class weak_symbols : public action_rewriter {
    public:
        /// \brief Maps symbols to sets of symbols
        typedef std::map<contextfree::item_container, contextfree::item_set> item_map;
        
        /// \brief Iterator that retrieves the map between strong and weak symbols
        typedef item_map::const_iterator strong_iterator;
        
    private:
        /// \brief Item map that describes the set of weak symbols that should be considered equivalent to a
        /// particular strong symbol
        item_map m_StrongToWeak;
        
        /// \brief The items that are considered 'weak'
        ///
        /// Note that m_StrongToWeak only contains a symbol if it has weak equivalents. Symbols are strong by default,
        /// so it's necessary to maintain a separate list of the weak symbols.
        contextfree::item_set m_WeakSymbols;
        
    public:
        /// \brief Constructs a translator with no weak symbols
        weak_symbols();
        
        /// \brief Constructs a rewriter with the specified map of strong to weak symbols
        explicit weak_symbols(const item_map& map);
        
        /// \brief Copy constructor
        weak_symbols(const weak_symbols& copyFrom);
        
        /// \brief Destructor
        virtual ~weak_symbols();
        
        /// \brief Maps the specified strong symbol to the specified set of weak symbols
        void add_symbols(const contextfree::item_container& strong, const contextfree::item_set& weak);
        
        /// \brief Given a set of weak symbols and a DFA (note: NOT an NDFA), determines the appropriate strong
        /// symbols and adds them
        ///
        /// The terminal dictionary may be modified if any symbols need to be split in order to generate a set
        /// of unique symbols.
        void add_symbols(dfa::ndfa& dfa, const contextfree::item_set& weak, contextfree::terminal_dictionary& terminals);

        /// \brief Modifies the specified set of actions according to the rules in this rewriter
        virtual void rewrite_actions(int state, lr_action_set& actions, const lalr_builder& builder) const;
        
        /// \brief Creates a clone of this rewriter
        virtual action_rewriter* clone() const;
        
        /// \brief Returns the weak symbols that are equivalent to the specified strong symbol
        ///
        /// This is primarily useful for testing this object: it can reveal if the form of add_symbols referring to a DFA
        /// has operated correctly.
        inline const contextfree::item_set& weak_for_strong(const contextfree::item_container& strongSymbol) const {
            // Need a set of empty items
            static contextfree::item_set emptySet;
            
            // Try to find the set of weak symbols for the specified strong symbol
            item_map::const_iterator found = m_StrongToWeak.find(strongSymbol);
            
            // Return the item that was found if there was one
            if (found != m_StrongToWeak.end()) return found->second;
            
            // Return an empty set if there wasn't
            return emptySet;
        }
        
        /// \brief An iterator referencing the first symbol in the strong-to-weak map
        inline strong_iterator begin_strong() const { return m_StrongToWeak.begin(); }
        
        /// \brief An iterator referencing symbol after the last symbol in the strong-to-weak map
        inline strong_iterator end_strong() const { return m_StrongToWeak.end(); }
    };
}

#endif
