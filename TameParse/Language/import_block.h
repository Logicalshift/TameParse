//
//  import_block.h
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
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
