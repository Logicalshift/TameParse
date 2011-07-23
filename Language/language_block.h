//
//  language_block.h
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LANGUAGE_LANGUAGE_BLOCK_H
#define _LANGUAGE_LANGUAGE_BLOCK_H

#include <string>
#include <vector>

#include "Language/block.h"
#include "Language/language_unit.h"

namespace language {
    ///
    /// \brief Class representing the content of a language block from a parser definition file
    ///
    class language_block : public block {
    public:
        /// \brief List of identifiers
        typedef std::vector<std::wstring> identifier_list;
        
        /// \brief List of language units
        typedef std::vector<language_unit*> unit_list;
        
    private:
        /// \brief The identifier for this language block
        std::wstring m_Identifier;
        
        /// \brief A list of the identifiers of the languages that this language should inherit from
        identifier_list m_Inherits;
        
        /// \brief A list of the language units that define this language
        unit_list m_Units;
        
    public:
    };
}

#endif
