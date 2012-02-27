//
//  accept_action.h
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the \"Software\"), to 
//  deal in the Software without restriction, including without limitation the 
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
//  sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
//  IN THE SOFTWARE.
//

#ifndef _DFA_ACCEPT_ACTION_H
#define _DFA_ACCEPT_ACTION_H

namespace dfa {
    ///
    /// \brief Class describing an accepting action in a DFA
    ///
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
        
        /// \brief Determines if this action is less important than another
        ///
        /// By default, actions with lower symbol IDs are more important than those with higher symbol IDs
        ///
        /// TODO: work out a way of ensuring that this is properly commutative when this class is subclassed
        virtual bool operator<(const accept_action& compareTo) const;
        inline bool operator>(const accept_action& compareTo) const { return compareTo.operator<(*this); }
        
        /// \brief Determines if this action is equivalent to another
        virtual bool operator==(const accept_action* compareTo) const;
        
        inline bool operator!=(const accept_action* compareTo) const { return !operator==(compareTo); }
        inline bool operator==(const accept_action& compareTo) const { return operator==(&compareTo); }
        inline bool operator!=(const accept_action& compareTo) const { return !operator==(compareTo); }

        inline bool operator<=(const accept_action& compareTo) const { return !operator>(compareTo); }
        inline bool operator>=(const accept_action& compareTo) const { return !operator<(compareTo); }

        /// \brief Destructor
        virtual ~accept_action();
        
        /// \brief The symbol that is associated with this accept action
        inline int symbol() const { return m_Symbol; }
        
        /// \brief True if this accept action is eager (stops the lexer from being greedy)
        inline bool eager() const { return m_Eager; }
    };
}

#endif
