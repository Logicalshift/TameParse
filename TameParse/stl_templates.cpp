//
//  stl_templates.cpp
//  Parse
//
//  Created by Andrew Hunter on 19/11/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/common.h"
#include "TameParse/stl_templates.h"

// Sets
template class std::set<int>;

// Maps
template class std::map<std::string, int>;
template class std::map<int, std::string>;
template class std::map<int, std::wstring>;
template class std::map<int, int>;
template class std::map<int, std::set<int> >;
