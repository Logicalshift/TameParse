//
//  templates.h
//  Parse
//
//  Created by Andrew Hunter on 19/11/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _TAMEPARSE_TEMPLATES_H
#define _TAMEPARSE_TEMPLATES_H

#include <set>

#include "TameParse/ContextFree/item.h"
#include "TameParse/ContextFree/item_set.h"

///
/// Extern templates defined by the tameparse language
///
/// These can be used to reduce the size of object files using the tameparse library
/// by avoiding the need to instantiate very commonly used templates.
///

extern template class std::set<contextfree::item_container>;
extern template class contextfree::item_map<contextfree::item_set>;
extern template class contextfree::item_map<int>;

#endif
