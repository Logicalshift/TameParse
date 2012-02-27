//
//  unicode.h
//  TameParse
//
//  Created by Andrew Hunter on 11/09/2011.
//  
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

#ifndef _UTIL_UNICODE_H
#define _UTIL_UNICODE_H

#include "TameParse/Dfa/symbol_set.h"

namespace util {
    ///
    /// \brief Class used to help out with unicode sequences
    ///
    class unicode {
    public:
        /// \brief Range in a unicode block
        struct range {
            int lower;
            int upper;
        };
        
        /// \brief Defines a block of unicode characters
        struct block {
            /// \brief The type of this block (as in the UnicodeData.txt file)
            const char* type;
            
            /// \brief The ranges in this block
            const range* ranges;
        };
        
        /// \brief Iterator that goes through all of the blocks
        typedef const block* iterator;

        /// \brief Entry in a character map table
        struct map_entry {
            int lower;
            int upper;
            int map_to;
        };
    
    private:
        /// \brief The blocks in this unicode definition
        static const block s_Blocks[];
        
        /// \brief The block after the last block in the list
        static const block* s_EndBlock;

        /// \brief Number of entries in the upper case map
        static const int s_UppercaseMapSize;

        /// \brief Number of entries in the lower case map
        static const int s_LowercaseMapSize;

        /// \brief Sorted array mapping lowercase characters to their uppercase equivalent
        static const map_entry s_UppercaseMap[];

        /// \brief Sorted array mapping uppercase characters to their lowercase equivalent
        static const map_entry s_LowercaseMap[];
        
    public:
        /// \brief The first character range
        inline iterator begin() const { return s_Blocks; }
        
        /// \brief The final character range
        inline iterator end() const { return s_EndBlock; }

        /// \brief Returns the uppercase equivalent of the specified symbol set
        dfa::symbol_set to_upper(const dfa::symbol_set& source);

        /// \brief Returns the lowercase equivalent of the specified symbol set
        dfa::symbol_set to_lower(const dfa::symbol_set& source);
    };
}

#endif
