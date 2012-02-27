//
//  transition.h
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
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

#ifndef _DFA_TRANSITION_H
#define _DFA_TRANSITION_H

#include "TameParse/Dfa/symbol_set.h"

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
