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

/// \brief Disposes a lexer item
lexer_item::~lexer_item() {
	// Delete the accept action if it exists
	if (accept) {
		delete accept;
	}
}
