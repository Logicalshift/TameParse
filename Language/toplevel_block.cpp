//
//  toplevel_block.cpp
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "toplevel_block.h"

using namespace language;

/// \brief Creates a new top level block containing a language block
toplevel_block::toplevel_block(language_block* language)
: m_ImportBlock(NULL)
, m_LanguageBlock(language) {
    if (language) {
        set_start_pos(language->start_pos());
        set_end_pos(language->end_pos());
    }
}

/// \brief Creates a new top level block containing an import block
toplevel_block::toplevel_block(import_block* import) 
: m_ImportBlock(import)
, m_LanguageBlock(NULL) {
    if (import) {
        set_start_pos(import->start_pos());
        set_end_pos(import->end_pos());
    }
}

/// \brief Copies an existing toplevel block
toplevel_block::toplevel_block(const toplevel_block& copyFrom)
: m_ImportBlock(NULL)
, m_LanguageBlock(NULL) {
    (*this) = copyFrom;
}

/// \brief Destructor
toplevel_block::~toplevel_block() {
    if (m_ImportBlock)      delete m_ImportBlock;
    if (m_LanguageBlock)    delete m_LanguageBlock;
}

/// \brief Assigns this block
toplevel_block& toplevel_block::operator=(const toplevel_block& copyFrom) {
    // Won't copy from self
    if (&copyFrom == this) return *this;
    
    // Self-destruct
    if (m_ImportBlock)      delete m_ImportBlock;
    if (m_LanguageBlock)    delete m_LanguageBlock;
    
    // Copy
    set_start_pos(copyFrom.start_pos());
    set_end_pos(copyFrom.end_pos());
    
    if (copyFrom.language())    m_LanguageBlock = new language_block(*copyFrom.language());
    if (copyFrom.import())      m_ImportBlock   = new import_block(*copyFrom.import());
    
    // That was tedious
    return *this;
}
