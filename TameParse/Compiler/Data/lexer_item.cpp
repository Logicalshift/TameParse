//
//  lexer_item.cpp
//  TameParse
//
//  Created by Andrew Hunter on 12/11/2011.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/Compiler/Data/lexer_item.h"

using namespace compiler;

/// \brief Creates a new lexer item
lexer_item::lexer_item(item_type ty, const std::wstring& def, bool insensitive, dfa::accept_action* aa)
: type(ty)
, definition(def)
, case_insensitive(insensitive)
, accept(aa) {
}

/// \brief Copies a lexer item
lexer_item::lexer_item(const lexer_item& copyFrom)
: type(copyFrom.type)
, definition(copyFrom.definition)
, case_insensitive(copyFrom.case_insensitive)
, accept(copyFrom.accept?copyFrom.accept->clone():NULL) {
}

/// \brief Assigns a lexer item
lexer_item& lexer_item::operator=(const lexer_item& assignFrom) {
    // Can't self-assign
    if (&assignFrom == this) return *this;
    
    // Destroy the contents of this object
    if (accept) {
        delete accept;
        accept = NULL;
    }
    
    // Copy new contents into it
    type                = assignFrom.type;
    definition          = assignFrom.definition;
    case_insensitive    = assignFrom.case_insensitive;
    accept              = assignFrom.accept?assignFrom.accept->clone():NULL;
    
    return *this;
}

/// \brief Disposes a lexer item
lexer_item::~lexer_item() {
	// Delete the accept action if it exists
	if (accept) {
		delete accept;
	}
}
