//
//  test_block.h
//  TameParse
//
//  Created by Andrew Hunter on 18/10/2011.
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

#ifndef _LANGUAGE_TEST_BLOCK_H
#define _LANGUAGE_TEST_BLOCK_H

#include <vector>
#include <string>

#include "TameParse/Language/block.h"
#include "TameParse/Language/test_definition.h"

namespace yy_language {
    /// \brief Class representing a test block in the parser language
    class test_block : public block {
    public:
        /// \brief List of test definitions
        typedef std::vector<test_definition*> test_definition_list;

        /// \brief Iterator for retrieving the definitions in this block
        typedef test_definition_list::const_iterator iterator;

    private:
        /// \brief The language that these tests are for
        std::wstring m_Language;

        /// \brief The test definitions in this block
        test_definition_list m_TestDefinitions;

    public:
        /// \brief Creates a new test block
        test_block(const std::wstring& languageIdentifier, position start, position end);

        /// \brief Creates a new test block by copying an old one
        test_block(const test_block& copyFrom);

        /// \brief Assigns the content of this block
        test_block& operator=(const test_block& assignFrom);

        /// \brief Destructor
        virtual ~test_block();

        /// \brief Adds a new test definition (the definition will become owned by this object)
        void add_test_definition(test_definition* newDefinition);

        /// \brief The name of the language that this should test
        inline const std::wstring& language() const { return m_Language; }

        /// \brief The first test definition in this block
        inline iterator begin() const { return m_TestDefinitions.begin(); }

        /// \brief The test definition after the last one in this block
        inline iterator end() const { return m_TestDefinitions.end(); }
    };
}

#endif
