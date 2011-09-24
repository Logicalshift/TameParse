//
//  block.h
//  Parse
//
//  Created by Andrew Hunter on 23/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
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
