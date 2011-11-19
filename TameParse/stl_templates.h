//
//  stl_templates.h
//  Parse
//
//  Created by Andrew Hunter on 19/11/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _TAMEPARSE_STL_TEMPLATES_H
#define _TAMEPARSE_STL_TEMPLATES_H

#include <string>
#include <map>
#include <set>

///
/// Extern templates used by TameParse that are in the STL
///
/// In general, these definitions are not exposed to user programs, but are used by
/// the C++ files in the tameparse library itself
///

// Sets
extern template class std::set<int>;

// Maps
extern template class std::map<std::string, int>;
extern template class std::map<int, std::string>;
extern template class std::map<int, std::wstring>;
extern template class std::map<int, int>;
extern template class std::map<int, std::set<int> >;

#endif
