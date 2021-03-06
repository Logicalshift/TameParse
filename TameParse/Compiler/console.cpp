//
//  console.cpp
//  Parse
//
//  Created by Andrew Hunter on 06/08/2011.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the \"Software\"), to 
//  deal in the Software without restriction, including without limitation the 
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
//  sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
//  IN THE SOFTWARE.
//

#include "TameParse/Compiler/console.h"

using namespace std;
using namespace compiler;

/// \brief Destructor
console::~console() {
}

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
    
    for (wstring::const_iterator fileChar = filename.begin(); fileChar != filename.end(); ++fileChar) {
        latin1Filename += (char) *fileChar;
    }

    return latin1Filename;
}

/// \brief Returns true if the options are valid and the parser can start
bool console::can_start() const {
    return true;
}

/// \brief Returns a list of values for a particular option
///
/// For some options it is possible to specify more than one value: in this
/// case, this will return all of the possible values. This can also be used
/// to retrieve the values of options that can have an empty value.
///
/// The default implementation just calls get_option() and creates a list of
/// size one if it contains a non-empty string. Subclasses should generally
/// override this behaviour if they truly support empty options or options
/// with multiple values
std::vector<std::wstring> console::get_option_list(const std::wstring& name) {
    // Retrieve the option
    wstring value = get_option(name);

    // Build the result
    vector<wstring> res;

    // Add the option if it's present
    if (!value.empty()) {
        res.push_back(value);
    }

    return res;
}

/// \brief Given a pathname, returns the 'real', absolute pathname
///
/// The default implementation just returns the current path
std::wstring console::real_path(const std::wstring& pathname) {
    // By default, we don't know how to do this
    return pathname;
}

/// \brief Splits a path into its components
///
/// The default implementation assumes UNIX-style paths.
std::vector<std::wstring> console::split_path(const std::wstring& pathname) {
    // Separator character
    const wchar_t separator = L'/';

    // Split up this string
    vector<wstring> res;
    wstring current;
    for (size_t x=0; x<pathname.size(); ++x) {
        // Get the current character
        wchar_t c = pathname[x];

        if (c == separator) {
            // Add this to the result
            if (!current.empty()) {
                res.push_back(current);
                current.clear();
            }
        } else {
            // Keep building the currentpath
            current += c;
        }
    }

    // Push the final component
    if (!current.empty()) {
        res.push_back(current);
    }

    // This is the result
    return res;
}
