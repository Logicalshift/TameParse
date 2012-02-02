//
//  import_block.cpp
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#include "TameParse/Language/import_block.h"
#include "TameParse/Language/process.h"

using namespace std;
using namespace dfa;
using namespace language;

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
