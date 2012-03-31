//
//  State.h
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
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

#ifndef _DFA_STATE_H
#define _DFA_STATE_H

#include <set>

#include "TameParse/Dfa/transition.h"

namespace dfa {
    ///
    /// \brief Describes a state in an NDFA.
    ///
    class state {
    private:
        /// \brief Storage class for the transitions
        typedef std::set<transition> transition_set;
        
        /// \brief The identifier for this state
        int m_Identifier;
        
        /// \brief The transitions for this state
        transition_set m_Transitions;

        /// \brief Disabled assignment
        state& operator=(const state& assignFrom);

    public:
        /// \brief Iterator for the transitions from this state
        typedef transition_set::const_iterator iterator;
        
    public:
        /// \brief Copies a state
        state(const state& copyFrom);
        
        /// \brief Creates a new state with the specified identifier
        explicit state(int identifier);
        
        /// \brief Destructor
        virtual ~state();
        
    public:
        /// \brief Adds a new transition to this state
        void add(const transition& newTransition);
        
    public:
        /// \brief The identifier for this state
        inline int identifier() const { return m_Identifier; }
        
        /// \brief The number of transitions in this state
        inline int count_transitions() const { return (int)m_Transitions.size(); }
        
        /// \brief The first transition of this state
        inline iterator begin() const { return m_Transitions.begin(); }
        
        /// \brief The final transition of this state
        inline iterator end() const { return m_Transitions.end(); }
    };
}

#endif
