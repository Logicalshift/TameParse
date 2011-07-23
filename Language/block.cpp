//
//  block.cpp
//  Parse
//
//  Created by Andrew Hunter on 23/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "Language/block.h"

using namespace language;

/// \brief Creates a new block with unset start and end points
block::block()
: m_Start()
, m_End() {
}

/// \brief Creates a new block with the given start and end points
block::block(const position& start, const position& end) 
: m_Start(start)
, m_End(end) {
}

/// \brief Destructor
block::~block() {
    // Nothing to do
}
