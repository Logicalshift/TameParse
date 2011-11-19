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

///
/// Extern templates used by TameParse that are in the STL
///
/// In general, these definitions are not exposed to user programs, but are used by
/// the C++ files in the tameparse library itself
///

// Strings
extern template class std::basic_string<int>;
extern template class std::basic_string<wchar_t>;
extern template class std::basic_string<char>;

// Maps
extern template class std::map<std::string, int>;

#endif
