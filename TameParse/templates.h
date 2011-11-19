//
//  templates.h
//  Parse
//
//  Created by Andrew Hunter on 19/11/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _TAMEPARSE_TEMPLATES_H
#define _TAMEPARSE_TEMPLATES_H

#include "TameParse/Util/container.h"

#include "TameParse/Compiler/console.h"

///
/// Extern templates defined by the tameparse language
///
/// These can be used to reduce the size of object files using the tameparse library
/// by avoiding the need to instantiate very commonly used templates.
///

// Containers (we use a lot of these)
extern template class util::container<compiler::console>;

#endif
