//
//  definition_file.h
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

#ifndef _LANGUAGE_DEFINITION_FILE_H
#define _LANGUAGE_DEFINITION_FILE_H

#include <vector>

#include "TameParse/Util/container.h"
#include "TameParse/Language/block.h"
#include "TameParse/Language/toplevel_block.h"

namespace language {
    ///
    /// \brief Describes the content of a definition file
    ///
    class definition_file : public block {
    public:
        /// \brief List of top level blocks
        typedef std::vector<toplevel_block*> block_list;
        
        /// \brief Iterator for accessing the blocks
        typedef block_list::const_iterator iterator;
        
    private:
        /// \brief The blocks that make up this definition file
        block_list m_Blocks;
        
    public:
        /// \brief Creates a new definition file, containing no blocks
        definition_file();
        
        /// \brief Creates a new definition file by copying an existing one
        definition_file(const definition_file& copyFrom);
        
        /// \brief Assigns the value of this definition file
        definition_file& operator=(const definition_file& copyFrom);
        
        /// \brief Destroys this definition file object
        virtual ~definition_file();
        
        /// \brief Adds a new top-level block to this object; this object will be responsible for freeing the block
        void add(toplevel_block* newBlock);
        
        /// \brief The first block in the definition file
        inline const iterator begin() const { return m_Blocks.begin(); }
        
        /// \brief The final block in the definition file
        inline const iterator end() const   { return m_Blocks.end(); }
        
    public:
        /// \brief Creates a clone of this file
        inline definition_file* clone() const { return new definition_file(*this); }
        
        /// \brief Compares two definition files
        inline static bool compare(const definition_file* a, const definition_file* b) {
            // Just do a direct pointer compare
            return a < b;
        }
    };
    
    /// \brief Container for definition files
    typedef util::container<definition_file> definition_file_container;
}

#endif
