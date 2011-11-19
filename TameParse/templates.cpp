//
//  templates.cpp
//  Parse
//
//  Created by Andrew Hunter on 19/11/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/common.h"
#include "TameParse/templates.h"

template class std::set<contextfree::item_container>;
template class contextfree::item_map<contextfree::item_set>;
template class contextfree::item_map<int>;
