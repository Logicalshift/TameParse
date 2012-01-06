//
//  lexer_item.cpp
//  TameParse
//
//  Created by Andrew Hunter on 12/11/2011.
//  Copyright (c) 2011 Andrew Hunter. All rights reserved.
//

#include "TameParse/Compiler/Data/lexer_item.h"

using namespace language;
using namespace compiler;

/// \brief Creates a new lexer item
lexer_item::lexer_item(item_type ty, const std::wstring& def, bool insensitive, bool sensitive, const std::wstring* fn, const dfa::position& pos)
: type(ty)
, definition(def)
, case_insensitive(insensitive)
, case_sensitive(sensitive)
, symbol(0)
, definition_type(language_unit::unit_null)
, is_weak(false)
, filename(fn)
, position(pos) {
}

/// \brief Creates a new lexer item
lexer_item::lexer_item(item_type ty, const std::wstring& def, bool insensitive, bool sensitive, int sym, unit_type def_type, bool weak, const std::wstring* fn, const dfa::position& pos) 
: type(ty)
, definition(def)
, case_insensitive(insensitive)
, case_sensitive(sensitive)
, symbol(sym)
, definition_type(def_type)
, is_weak(weak)
, filename(fn)
, position(pos) {
}

/// \brief Copies a lexer item
lexer_item::lexer_item(const lexer_item& copyFrom)
: type(copyFrom.type)
, definition(copyFrom.definition)
, case_insensitive(copyFrom.case_insensitive)
, case_sensitive(copyFrom.case_sensitive)
, symbol(copyFrom.symbol)
, definition_type(copyFrom.definition_type)
, is_weak(copyFrom.is_weak)
, filename(copyFrom.filename)
, position(copyFrom.position) {
}

/// \brief Assigns a lexer item
lexer_item& lexer_item::operator=(const lexer_item& assignFrom) {
    // Can't self-assign
    if (&assignFrom == this) return *this;
    
    // Copy new contents into this object
    type                = assignFrom.type;
    definition          = assignFrom.definition;
    case_insensitive    = assignFrom.case_insensitive;
    case_sensitive      = assignFrom.case_sensitive;
    symbol              = assignFrom.symbol;
    definition_type     = assignFrom.definition_type;
    is_weak             = assignFrom.is_weak;
    filename            = assignFrom.filename;
    position            = assignFrom.position;
    
    return *this;
}

/// \brief Disposes a lexer item
lexer_item::~lexer_item() {
}
