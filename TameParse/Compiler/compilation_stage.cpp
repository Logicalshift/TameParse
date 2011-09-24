//
//  compilation_stage.cpp
//  Parse
//
//  Created by Andrew Hunter on 06/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/Compiler/compilation_stage.h"

using namespace compiler;

/// \brief Creates a new compilation stage which will use the specified console object
compilation_stage::compilation_stage(console_container& console, const std::wstring& filename)
: m_Console(console)
, m_Filename(filename) {
}
