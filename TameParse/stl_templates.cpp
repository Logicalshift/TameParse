//
//  stl_templates.cpp
//  Parse
//
//  Created by Andrew Hunter on 19/11/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/common.h"
#include "TameParse/stl_templates.h"

// Strings
template class std::basic_string<int>;
template class std::basic_string<wchar_t>;
template class std::basic_string<char>;

// Maps
template class std::map<std::string, int>;
