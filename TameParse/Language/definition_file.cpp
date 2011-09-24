//
//  definition_file.cpp
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/Language/definition_file.h"

using namespace std;
using namespace language;

/// \brief Creates a new definition file, containing no blocks
definition_file::definition_file() {
}

/// \brief Creates a new definition file by copying an existing one
definition_file::definition_file(const definition_file& copyFrom) {
    (*this) = copyFrom;
}

/// \brief Assigns the value of this definition file
definition_file& definition_file::operator=(const definition_file& copyFrom) {
    // Nothing to do if the object passed in already exists
    if (&copyFrom == this) return *this;
    
    // Iterate through the blocks in this file
    for (block_list::iterator deleteBlock = m_Blocks.begin(); deleteBlock != m_Blocks.end(); deleteBlock++) {
        delete *deleteBlock;
    }
    m_Blocks.clear();
    
    // Reset this file
    set_start_pos(position());
    set_end_pos(position());
    
    // Copy the blocks from the source
    for (iterator copyBlock = copyFrom.begin(); copyBlock != copyFrom.end(); copyBlock++) {
        add(new toplevel_block(**copyBlock));
    }
    
    return *this;
}

/// \brief Destroys this definition file object
definition_file::~definition_file() {
    // Iterate through the blocks in this file and delete them
    for (block_list::iterator deleteBlock = m_Blocks.begin(); deleteBlock != m_Blocks.end(); deleteBlock++) {
        delete *deleteBlock;
    }
    m_Blocks.clear();
}

/// \brief Adds a new top-level block to this object; this object will be responsible for freeing the block
void definition_file::add(toplevel_block* newBlock) {
    // Don't add NULL blocks
    if (!newBlock) return;
    
    // Update the start and end position
    if (m_Blocks.empty() || newBlock->start_pos() < start_pos())    set_start_pos(newBlock->start_pos());
    if (m_Blocks.empty() || end_pos() < newBlock->end_pos())        set_end_pos(newBlock->end_pos());

    // Add to the m_Blocks array
    m_Blocks.push_back(newBlock);
}
