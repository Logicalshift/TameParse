//
//  unicode.h
//  TameParse
//
//  Created by Andrew Hunter on 11/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _UTIL_UNICODE_H
#define _UTIL_UNICODE_H

namespace util {
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
    
    private:
        /// \brief The blocks in this unicode definition
        static const block s_Blocks[];
        
        /// \brief The block after the last block in the list
        static const block* s_EndBlock;
        
    public:
        /// \brief The first character range
        inline iterator begin() const { return s_Blocks; }
        
        /// \brief The final character range
        inline iterator end() const { return s_EndBlock; }
    };
}

#endif
