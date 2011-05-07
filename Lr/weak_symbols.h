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

#include "Dfa/ndfa.h"
#include "ContextFree/item.h"
#include "ContextFree/terminal_dictionary.h"
#include "Lr/action_rewriter.h"

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
        
    private:
        /// \brief Item map that describes the set of weak symbols that should be considered equivalent to a particular strong
        /// symbol
        item_map m_StrongToWeak;
        
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
        
        /// \brief Given a set of weak symbols and a DFA (note: NOT an NDFA), determines the appropriate strong symbols and adds them
        ///
        /// The terminal dictionary may be modified if any symbols need to be split
        void add_symbols(dfa::ndfa& dfa, const contextfree::item_set& weak, contextfree::terminal_dictionary& terminals);

        /// \brief Modifies the specified set of actions according to the rules in this rewriter
        virtual void rewrite_actions(int state, lr_action_set& actions, const lalr_builder& builder) const;
        
        /// \brief Creates a clone of this rewriter
        virtual action_rewriter* clone() const;
    };
}

#endif
