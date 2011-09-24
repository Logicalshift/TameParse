//
//  epsilon.h
//  Parse
//
//  Created by Andrew Hunter on 27/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_EPSILON_H
#define _DFA_EPSILON_H

#include "TameParse/Dfa/symbol_set.h"

namespace dfa {
    ///
    /// \brief Placeholder class representing the empty symbol set
    ///
    class epsilon {
    private:
        /// \brief The empty symbol set
        static const symbol_set s_Empty;
        
    public:
        inline operator const symbol_set&() const   { return s_Empty; }
        inline operator symbol_set() const          { return s_Empty; }
    };
}

#endif
