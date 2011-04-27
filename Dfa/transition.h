//
//  transition.h
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_TRANSITION_H
#define _DFA_TRANSITION_H

#include "Dfa/symbol_set.h"

namespace dfa {
    ///
    /// \brief Description of a transition in a (N)DFA
    ///
    class transition {
    private:
        /// \brief The range of symbols that this transition will occur on
        ///
        /// This identifier usually maps into a symbol set in a symbol_map object
        int m_SymbolSet;
        
        /// \brief The DFA state that's entered by this transition
        int m_NewState;
        
    public:
        /// \brief Creates a new DFA transition
        transition(int symbolSet, int newState);
        
        /// \brief Returns the ID of the symbol set that this transition will match
        inline int symbol_set() const { return m_SymbolSet; }
        
        /// \brief Returns the ID of the new state that the DFA will move into after matching this state
        inline int new_state() const { return m_NewState; }
        
    public:
        /// \brief Determines if this set represents the same as another set
        bool operator==(const transition& compareTo) const;
        
        /// \brief Orders this symbol set
        bool operator<(const transition& compareTo) const;
        
        /// \brief Orders this symbol set
        bool operator<=(const transition& compareTo) const;
        
        /// \brief Orders this symbol set
        inline bool operator!=(const transition& compareTo) const { return !operator==(compareTo); }
        
        /// \brief Orders this symbol set
        inline bool operator>(const transition& compareTo) const { return !operator<=(compareTo); }
        
        /// \brief Orders this symbol set
        inline bool operator>=(const transition& compareTo) const { return !operator<(compareTo); }
    };
}

#endif
