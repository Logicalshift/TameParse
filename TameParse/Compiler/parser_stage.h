//
//  parser_stage.h
//  TameParse
//
//  Created by Andrew Hunter on 24/09/2011.
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

#ifndef _COMPILER_PARSER_STAGE_H
#define _COMPILER_PARSER_STAGE_H

#include "TameParse/Compiler/compilation_stage.h"
#include "TameParse/Language/language_parser.h"

namespace compiler {
    ///
    /// \brief Compiler stage that reads in and translates a file
    ///
    class parser_stage : public compilation_stage {
    private:
        /// \brief The language parser object
        language::language_parser m_Parser;
        
    public:
        /// \brief Creates a new compilation stage which will use the specified console object
        parser_stage(console_container& console, const std::wstring& filename);
        
        /// \brief Performs the actions associated with this compilation stage
        virtual void compile();
        
        /// \brief The definition produced by this stage
        inline language::definition_file_container definition_file() { return m_Parser.file_definition(); }
    };
}

#endif
