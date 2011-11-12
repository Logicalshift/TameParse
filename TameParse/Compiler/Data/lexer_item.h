//
//  lexer_item.h
//  TameParse
//
//  Created by Andrew Hunter on 12/11/2011.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _COMPILER_LEXER_ITEM_H
#define _COMPILER_LEXER_ITEM_H

#include <string>

namespace compiler {
	/// \brief Class representing the data associated with an individual lexer item
	class lexer_item {
	public:
		/// \brief The type of a lexer item
		enum item_type {
			regex,
			literal
		};

		/// \brief The type of this item
		item_type type;

		/// \brief The definition of this item
		std::wstring definition;

		/// \brief True if this item should be case-insensitive
		bool case_insensitive;

		/// \brief Creates a new lexer item
		lexer_item(item_type type, const std::wstring& definition, bool case_insensitive);
	};
}

#endif
