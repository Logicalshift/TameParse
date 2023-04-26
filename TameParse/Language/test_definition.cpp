//
//  test_definition.cpp
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

#include "TameParse/Language/test_definition.h"

using namespace yy_language;

/// \brief Creates a new test definition
test_definition::test_definition(const std::wstring& nonterminalLanguage, const std::wstring& nonterminal, test_type testType, const std::wstring& identifier, const std::wstring& testString, const position& start, const position& end, const position& testStringPos) 
: block(start, end)
, m_TestStringPosition(testStringPos)
, m_NonterminalLanguage(nonterminalLanguage) 
, m_Nonterminal(nonterminal)
, m_Type(testType)
, m_Identifier(identifier)
, m_TestString(testString) {
}

/// \brief Destructor
test_definition::~test_definition() {
    
}
