//
//  process.cpp
//  Parse
//
//  Created by Andrew Hunter on 23/07/2011.
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

#include "TameParse/Language/process.h"

using namespace std;
using namespace yy_language;

/// \brief Given a string with quotation marks and '\' quoting, returns the literal string
wstring process::dequote_string(const wstring& string) {
    // Start building the result
    wstring result;
    
    // Assume the first an last character are '"' characters
    for (size_t pos=1; pos < string.size()-1; ++pos) {
        // Deal with quoting
        if (string[pos] == '\\') {
            // Move on a character
            ++pos;
            
            // Quoted character values
            switch (string[pos]) {
                    // Single character quotations
                case 'a':   result += L'\a'; break;
                case 'e':   result += 0x1b; break;
                case 'n':   result += L'\n'; break;
                case 'r':   result += L'\r'; break;
                case 'f':   result += L'\f'; break;
                case 't':   result += L'\t'; break;
                    
                    // TODO: \x and similar quotations
                    
                default:
                    // Just add the same character by default
                    result += string[pos];
                    break;
            }
        } else {
            // Just append this character
            result += string[pos];
        }
    }
    
    // Return the result
    return result;
}
