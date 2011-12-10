//
//  error.cpp
//  Parse
//
//  Created by Andrew Hunter on 06/08/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
//

#include "TameParse/Compiler/error.h"

using namespace compiler;

/// \brief Creates a new error
error::error(severity severity, const std::wstring& filename, const std::wstring& identifier, const std::wstring& description, const position& pos)
: m_Severity(severity)
, m_Filename(filename)
, m_Identifier(identifier)
, m_Description(description)
, m_Position(pos) {
}
