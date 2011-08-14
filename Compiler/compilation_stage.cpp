//
//  compilation_stage.cpp
//  Parse
//
//  Created by Andrew Hunter on 06/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "compilation_stage.h"

using namespace compiler;

/// \brief Creates a new compilation stage which will use the specified console object
compilation_stage::compilation_stage(console_container& console)
: m_Console(console) {
}
