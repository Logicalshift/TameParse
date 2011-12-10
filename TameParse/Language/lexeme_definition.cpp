//
//  lexeme_definition.cpp
//  Parse
//
//  Created by Andrew Hunter on 24/07/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
//

#include "TameParse/Language/lexeme_definition.h"

using namespace language;

/// \brief Creates a new lexeme definition
lexeme_definition::lexeme_definition(type typ, std::wstring identifier, std::wstring definition, bool addToDefinition, bool replaceDefinition, position start, position end, position defnPos)
: m_Type(typ)
, m_Identifier(identifier)
, m_Definition(definition)
, m_AddToDefinition(addToDefinition)
, m_ReplaceDefinition(replaceDefinition)
, block(start, end)
, m_DefinitionPos(defnPos) {
}
