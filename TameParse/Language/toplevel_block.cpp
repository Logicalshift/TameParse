//
//  toplevel_block.cpp
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
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

#include "toplevel_block.h"

using namespace language;

/// \brief Creates a new top level block containing a language block
toplevel_block::toplevel_block(language_block* language)
: m_ImportBlock(NULL)
, m_LanguageBlock(language)
, m_ParserBlock(NULL)
, m_TestBlock(NULL) {
    if (language) {
        set_start_pos(language->start_pos());
        set_end_pos(language->end_pos());
    }
}

/// \brief Creates a new top level block containing an import block
toplevel_block::toplevel_block(import_block* import) 
: m_ImportBlock(import)
, m_LanguageBlock(NULL)
, m_ParserBlock(NULL)
, m_TestBlock(NULL) {
    if (import) {
        set_start_pos(import->start_pos());
        set_end_pos(import->end_pos());
    }
}

/// \brief Creates a new top level block containing a parser block
toplevel_block::toplevel_block(parser_block* parser)
: m_ImportBlock(NULL)
, m_LanguageBlock(NULL)
, m_ParserBlock(parser)
, m_TestBlock(NULL) {
}

/// \brief Creates a new top level block containing a test block
toplevel_block::toplevel_block(test_block* test)
: m_ImportBlock(NULL)
, m_LanguageBlock(NULL)
, m_ParserBlock(NULL)
, m_TestBlock(test) {
}

/// \brief Copies an existing toplevel block
toplevel_block::toplevel_block(const toplevel_block& copyFrom)
: m_ImportBlock(NULL)
, m_LanguageBlock(NULL)
, m_ParserBlock(NULL)
, m_TestBlock(NULL) {
    (*this) = copyFrom;
}

/// \brief Destructor
toplevel_block::~toplevel_block() {
    if (m_ImportBlock)      delete m_ImportBlock;
    if (m_LanguageBlock)    delete m_LanguageBlock;
    if (m_ParserBlock)      delete m_ParserBlock;
    if (m_TestBlock)        delete m_TestBlock;
}

/// \brief Assigns this block
toplevel_block& toplevel_block::operator=(const toplevel_block& copyFrom) {
    // Won't copy from self
    if (&copyFrom == this) return *this;
    
    // Self-destruct
    if (m_ImportBlock)      delete m_ImportBlock;
    if (m_LanguageBlock)    delete m_LanguageBlock;
    if (m_ParserBlock)      delete m_ParserBlock;
    if (m_TestBlock)        delete m_TestBlock;
    
    // Copy
    set_start_pos(copyFrom.start_pos());
    set_end_pos(copyFrom.end_pos());
    
    if (copyFrom.language())    m_LanguageBlock = new language_block(*copyFrom.language());
    if (copyFrom.import())      m_ImportBlock   = new import_block(*copyFrom.import());
    if (copyFrom.parser())      m_ParserBlock   = new parser_block(*copyFrom.parser());
    if (copyFrom.test())        m_TestBlock     = new test_block(*copyFrom.test());
    
    // That was tedious
    return *this;
}
