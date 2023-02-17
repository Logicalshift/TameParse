//
//  import_block.cpp
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

#include "TameParse/Language/import_block.h"
#include "TameParse/Language/process.h"

using namespace std;
using namespace dfa;
using namespace yy_language;

/// \brief Creates a new import block by processing the quoted string in a lexeme
import_block::import_block(const dfa::lexeme& string) {
    // Get the content of the lexeme
    wstring stringContent(string.content<wchar_t>());
    
    // Work out the end position
    position_tracker end(string.pos());
    end.update_position(stringContent.begin(), stringContent.end());
    
    // Set the position of this block
    set_start_pos(string.pos());
    set_end_pos(end.current_position());
    
    // Set the filename value
    m_ImportFile = process::dequote_string(stringContent);
}

/// \brief Creates a new import block with the specified filename
import_block::import_block(const std::wstring& filename, const position& start, const position& end) 
: block(start, end)
, m_ImportFile(filename) {
}
