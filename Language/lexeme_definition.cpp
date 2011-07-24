//
//  lexeme_definition.cpp
//  Parse
//
//  Created by Andrew Hunter on 24/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "lexeme_definition.h"

using namespace language;

/// \brief Creates a new lexeme definition
lexeme_definition::lexeme_definition(type typ, std::wstring identifier, std::wstring definition, position start, position end)
: m_Type(typ)
, m_Identifier(identifier)
, m_Definition(definition)
, block(start, end) {
}
