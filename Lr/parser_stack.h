//
//  parser_stack.h
//  Parse
//
//  Created by Andrew Hunter on 08/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_PARSER_STACK_H
#define _LR_PARSER_STACK_H

namespace lr {
    ///
    /// \brief Representation of an entry on a parser stack
    ///
    template<typename ItemData> class parser_stack_entry {
    private:
        /// \brief The previous entry on this stack
        parser_stack_entry<ItemData>* m_PreviousEntry;
    };
}

#endif
