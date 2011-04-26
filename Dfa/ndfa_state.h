//
//  ndfa_state.h
//  Parse
//
//  Created by Andrew Hunter on 26/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_NDFA_STATE_H
#define _DFA_NDFA_STATE_H

#include <set>

namespace dfa {
    /// \brief Type representing the state of an NDFA parse
    typedef std::set<int> ndfa_state;
}

#endif
