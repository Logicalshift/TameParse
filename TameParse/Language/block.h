//
//  block.h
//  Parse
//
//  Created by Andrew Hunter on 23/07/2011.
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

#ifndef _LANGUAGE_BLOCK_H
#define _LANGUAGE_BLOCK_H

#include "TameParse/Dfa/position.h"

namespace language {
    ///
    /// \brief Common superclass for a block in a language file
    ///
    class block {
    public:
        /// \brief Position type
        typedef dfa::position position;
        
    private:
        /// \brief The position in the source file where this block starts
        position m_Start;
        
        /// \brief The position in the source file where this block ends
        position m_End;
        
    public:
        /// \brief Creates a new block with unset start and end points
        block();
        
        /// \brief Creates a new block with the given start and end points
        block(const position& start, const position& end);
        
        /// \brief Destructor
        virtual ~block();
        
        /// \brief The start position
        inline const position& start_pos() const { return m_Start; }
        
        /// \brief The end position
        inline const position& end_pos() const { return m_End; }
        
    protected:
        /// \brief Sets the start of this block
        void set_start_pos(const position& newStart)    { m_Start = newStart; }
        
        /// \brief Sets the end position of this block
        void set_end_pos(const position& newEnd)        { m_End = newEnd; }
    };
}

#endif
