//
//  test_definition.cpp
//  TameParse
//
//  Created by Andrew Hunter on 18/10/2011.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/Language/test_definition.h"

using namespace language;

/// \brief Creates a new test definition
test_definition::test_definition(const std::wstring& nonterminalLanguage, const std::wstring& nonterminal, test_type testType, const std::wstring& identifier, const std::wstring& testString) 
: m_NonterminalLanguage(nonterminalLanguage) 
, m_Nonterminal(nonterminal)
, m_Type(testType)
, m_Identifier(identifier)
, m_TestString(testString) {
}

/// \brief Destructor
test_definition::~test_definition() {
	
}