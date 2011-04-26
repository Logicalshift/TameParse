//
//  accept_action.h
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_ACCEPT_ACTION_H
#define _DFA_ACCEPT_ACTION_H

namespace dfa {
    /// \brief Class describing an accepting action in a DFA
    class accept_action {
    private:
        /// \brief The symbol that a lexer should return for accepting this item
        int m_Symbol;
        
        /// \brief True if this should be considered an 'eager' state (no further states should be considered after this)
        bool m_Eager;
        
    public:
        /// \brief Creates a standard accept action for the specified symbol
        accept_action(int symbol);
        
        /// \brief Creates an accept action with the specified 'eagerness'
        accept_action(int symbol, bool isEager);
       
        /// \brief For subclasses, allows the NDFA to clone this accept action for storage purposes
        virtual accept_action* clone() const;
        
        /// \brief Destructor
        virtual ~accept_action();
        
        /// \brief The symbol that is associated with this accept action
        inline int symbol() const { return m_Symbol; }
        
        /// \brief True if this accept action is eager (stops the lexer from being greedy)
        inline bool eager() const { return m_Eager; }
    };
}

#endif