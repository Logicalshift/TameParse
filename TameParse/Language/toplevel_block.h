//
//  toplevel_block.h
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LANGUAGE_TOPLEVEL_BLOCK_H
#define _LANGUAGE_TOPLEVEL_BLOCK_H

#include "TameParse/Util/container.h"

#include "TameParse/Language/block.h"
#include "TameParse/Language/language_block.h"
#include "TameParse/Language/import_block.h"
#include "TameParse/Language/parser_block.h"
#include "TameParse/Language/test_block.h"

namespace language {
    /// \brief Class representing a toplevel block
    class toplevel_block : public block {
    private:
        /// \brief NULL if this is not a language block, otherwise the language block represented by this item
        language_block* m_LanguageBlock;
        
        /// \brief NULL if this is not an import block, otherwise the import block represented by this item
        import_block* m_ImportBlock;
        
        /// \brief NULL if this is not a parser block, otherwise the parser block represented by this item
        parser_block* m_ParserBlock;

        /// \brief NULL if this is not a test block, otherwise the test block represetned by this item
        test_block* m_TestBlock;
        
    public:
        /// \brief Creates a new top level block containing a language block
        toplevel_block(language_block* language);
        
        /// \brief Creates a new top level block containing an import block
        toplevel_block(import_block* import);
        
        /// \brief Creates a new top level block containing a parser block
        toplevel_block(parser_block* parser);

        /// \brief Creates a new top level block containing a test block
        toplevel_block(test_block* test);
        
        /// \brief Copies an existing toplevel block
        toplevel_block(const toplevel_block& copyFrom);
        
        /// \brief Destructor
        virtual ~toplevel_block();
        
        /// \brief Assigns this block
        toplevel_block& operator=(const toplevel_block& copyFrom);
        
        /// \brief NULL, or the language block represented by this toplevel block
        inline const language_block* language() const { return m_LanguageBlock; }
        
        /// \brief NULL, or the import block represented by this toplevel block
        inline const import_block* import() const { return m_ImportBlock; }
        
        /// \brief NULL, or the parser block represented by this toplevel block
        inline const parser_block* parser() const { return m_ParserBlock; }

        /// \brief NULL, or the test block represeted by this toplevel block
        inline const test_block* test() const { return m_TestBlock; }
    };
}

#endif
