//
//  console.cpp
//  Parse
//
//  Created by Andrew Hunter on 06/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/Compiler/console.h"

using namespace std;
using namespace compiler;

/// \brief Orders this console with respect to another (default operation is just a pointer compare)
bool console::operator<(const console& compareTo) const {
    // Just do a pointer comparison
    return this < &compareTo;
}
/// \brief Converts a wstring filename to whatever is the preferred format for the current system
std::string console::convert_filename(const std::wstring& filename) {
    // This is mainly here because C++ sucks and has missed out on about the last 20 years of development
    // This implementation is a really dumb conversion to latin-1
    // Some operating systems might like UTF-8 here, but C++ is too dumb even to do that (and there doesn't appear to be any
    // standard way to do it, either, which seems nearly unbelievable, sigh)
    string latin1Filename;
    
    for (wstring::const_iterator fileChar = filename.begin(); fileChar != filename.end(); fileChar++) {
        latin1Filename += (char) *fileChar;
    }

    return latin1Filename;
}

/// \brief Returns true if the options are valid and the parser can start
bool console::can_start() const {
    return true;
}
