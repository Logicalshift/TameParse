//
//  regex_error.cpp
//  Parse
//
//  Created by Andrew Hunter on 22/11/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
//

#include "TameParse/Dfa/regex_error.h"

using namespace dfa;

/// \brief Constructor
regex_error::regex_error(error_type type, const position& pos) 
: m_Type(type)
, m_Position(pos) {
}

/// \brief Constructor
regex_error::regex_error(error_type type, const position& pos, const std::basic_string<int>& symbol) 
: m_Type(type)
, m_Position(pos)
, m_Symbol(symbol) {
}
