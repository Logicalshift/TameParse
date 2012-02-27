//
//  language_block.h
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

#ifndef _LANGUAGE_LANGUAGE_BLOCK_H
#define _LANGUAGE_LANGUAGE_BLOCK_H

#include <string>
#include <vector>

#include "TameParse/Language/block.h"
#include "TameParse/Language/language_unit.h"

namespace language {
    ///
    /// \brief Class representing the content of a language block from a parser definition file
    ///
    class language_block : public block {
    public:
        /// \brief List of identifiers
        typedef std::vector<std::wstring> identifier_list;
        
        /// \brief List of language units
        typedef std::vector<language_unit*> unit_list;
        
        /// \brief Iterator that retrieves the contents of this language block
        typedef unit_list::const_iterator iterator;
        
    private:
        /// \brief The identifier for this language block
        std::wstring m_Identifier;
        
        /// \brief A list of the identifiers of the languages that this language should inherit from
        identifier_list m_Inherits;
        
        /// \brief A list of the language units that define this language
        unit_list m_Units;
        
    public:
        /// \brief Creates a new empty language block with the specified identifier
        language_block(const std::wstring& identifier, position start, position end);
        
        /// \brief Creates a copy of this language block
        language_block(const language_block& copyFrom);
        
        /// \brief Copies a language block into this object
        language_block& operator=(const language_block& copyFrom);
        
        /// \brief Destructor
        virtual ~language_block();
        
        /// \brief Adds the identifier of a language that this language inherits from
        void add_inherits(const std::wstring& identifier);
        
        /// \brief Adds a language unit to this object (the unit becomes owned by this object, which will destroy it when it is itself destroyed)
        void add_unit(language_unit* unit);
        
        /// \brief The identifiers that this language inherits from
        const identifier_list& inherits() const { return m_Inherits; }
        
        /// \brief The identifier for this language
        const std::wstring& identifier() const { return m_Identifier; }
        
        /// \brief The first language unit in this block
        iterator begin() const { return m_Units.begin(); }
        
        /// \brief The item after the final language unit in this block
        iterator end() const { return m_Units.end(); }
    };
}

#endif
