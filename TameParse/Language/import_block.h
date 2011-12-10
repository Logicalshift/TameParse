//
//  import_block.h
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#ifndef _LANGUAGE_IMPORT_BLOCK_H
#define _LANGUAGE_IMPORT_BLOCK_H

#include <string>

#include "TameParse/Dfa/lexeme.h"
#include "TameParse/Language/block.h"

namespace language {
    ///
    /// \brief Represents an import block in a language file
    ///
    class import_block : public block {
    private:
        /// \brief The name of the file that should be imported
        std::wstring m_ImportFile;
        
    public:
        /// \brief Creates a new import block by processing the quoted string in a lexeme
        import_block(const dfa::lexeme& string);
        
        /// \brief Creates a new import block with the specified filename
        import_block(const std::wstring& filename, const position& start, const position& end);
        
        /// \brief The name of the file that should be imported
        const std::wstring& import_filename() const { return m_ImportFile; }
    };
}

#endif
