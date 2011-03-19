//
//  transition.h
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_TRANISTION_H
#define _DFA_TRANSITION_H

#include "symbol_set.h"

namespace dfa {
    /// \brief Description of a transition in a DFA
    class transition {
    private:
        /// \brief The range of symbols that this transition will occur on
        ///
        /// This identifier usually maps into a symbol set in a symbol_map object
        int m_SymbolSet;
    };
}

#endif
