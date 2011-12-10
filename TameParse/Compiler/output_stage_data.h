//
//  output_stage_data.h
//  TameParse
//
//  Created by Andrew Hunter on 10/12/2011.
//  Copyright (c) 2011 Andrew Hunter. All rights reserved.
//

#ifndef _COMPILER_OUTPUT_STAGE_DATA_H
#define _COMPILER_OUTPUT_STAGE_DATA_H

#include "Tameparse/ContextFree/item.h"

namespace compiler {
	/// \brief Structures used to supply output to an output stage
	namespace data {
		///
		/// \brief Structure representing a terminal symbol
		///
		struct terminal_symbol {
			/// \brief Constructs a new terminal symbol
			inline terminal_symbol(const std::wstring& newName, int newIdentifier)
			: name(newName)
			, identifier(newIdentifier) { }

			/// \brief The name of this symbol
			std::wstring name;

			/// \brief The identifier of this symbol
			int identifier;
		};

		///
		/// \brief Structure representing a nonterminal symbol
		///
		struct nonterminal_symbol {
			inline nonterminal_symbol(const std::wstring& newName, int newIdentifier, const contextfree::item_container& newItem)
			: name(newName)
			, identifier(newIdentifier)
			, item(newItem) { }

			/// \brief The name of the nonterminal symbol
			std::wstring name;

			/// \brief The identifier assigned to it within the grammar and parser
			int identifier;

			/// \brief The context free item representing this nonterminal symbol
			contextfree::item_container item;
		};
	}
}

#endif
