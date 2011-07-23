//
//  process.cpp
//  Parse
//
//  Created by Andrew Hunter on 23/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "process.h"

using namespace std;
using namespace language;

/// \brief Given a string with quotation marks and '\' quoting, returns the literal string
wstring process::dequote_string(const wstring& string) {
    // Start building the result
    wstring result;
    
    // Assume the first an last character are '"' characters
    for (int pos=1; pos < string.size()-1; pos++) {
        // Deal with quoting
        if (string[pos] == '\\') {
            // Move on a character
            pos++;
            
            // Quoted character values
            switch (string[pos]) {
                    // Single character quotations
                case 'a':   result += L'\a'; break;
                case 'e':   result += L'\e'; break;
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
