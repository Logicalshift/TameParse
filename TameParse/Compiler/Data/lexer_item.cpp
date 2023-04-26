//
//  lexer_item.cpp
//  TameParse
//
//  Created by Andrew Hunter on 12/11/2011.
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

#include "TameParse/Compiler/Data/lexer_item.h"

using namespace yy_language;
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
