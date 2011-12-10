//
//  parser_block.cpp
//  Parse
//
//  Created by Andrew Hunter on 25/08/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#include "TameParse/Language/parser_block.h"

using namespace std;
using namespace language;

/// \brief Creates a new parser block
parser_block::parser_block(const std::wstring& name, const std::wstring& languageName, const std::vector<std::wstring>& startSymbols, position startPos, position endPos) 
: m_LanguageName(languageName)
, m_ParserName(name)
, m_StartSymbols(startSymbols)
, block(startPos, endPos) {
}
