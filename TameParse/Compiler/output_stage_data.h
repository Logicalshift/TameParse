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
		/// \brief Represents a terminal symbol
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
		/// \brief Represents a nonterminal symbol
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

		///
		/// \brief Represents an entry in a symbol map
		///
		struct symbol_map {
			inline symbol_map(const dfa::range<int>& newSymbolRange, int newIdentifier)
			: symbolRange(newSymbolRange)
			, identifier(newIdentifier) {
			}

			/// \brief The range of input symbols corresponding to the identifier
			dfa::range<int> symbolRange;

			/// \brief The symbol identifier that should be generated for this range of symbols
			int identifier;
		};

		///
		/// \brief Structure representing a lexer state transition
		///
		struct lexer_state_transition {
			inline lexer_state_transition(int newStateId, int newSymbolSet, int newNewState)
			: stateIdentifier(newStateId)
			, symbolSet(newSymbolSet)
			, newState(newNewState) {
			}
			
			/// \brief The lexer state that this transition is for
			int stateIdentifier;

			/// \brief The symbol set that should generate this transition
			int symbolSet;

			/// \brief The state that the lexer should move to if this symbol set is matched
			int newState;
		};
	
		///
		/// \brief Represents the action to perform when the lexer enters a particular state
		///
		struct lexer_state_action {
			inline lexer_state_action(int newStateId, bool newAccepting, int newAcceptSymbolId)
			: stateId(newStateId)
			, accepting(newAccepting)
			, acceptSymbolId(newAcceptSymbolId) {
			}

			/// \brief The identifier of the state that this action is for
			int stateId;

			/// \brief True if this state is an accepting state (a point at which the lexer has matched a symbol)
			bool accepting;

			/// \brief The ID of the symbol that has been matched by the lexer (if this is an accepting state)
			int acceptSymbolId;
		};

		///
		/// \brief Represents the definition of a symbol in the AST
		///
		struct ast_symbol {
			inline ast_symbol(int newSymbolId, const contextfree::item_container& newItem)
			: symbolId(newSymbolId)
			, item(newItem) {
			}

			/// \brief For a terminal symbol, this is the ID of the lexer symbol that matches this symbol. For a nonterminal symbol, this is the ID of the nonterminal within the grammar.
			int symbolId;

			/// \brief The item definition for this terminal
			contextfree::item_container item;
		};

		///
		/// \brief Represents an item in a rule
		///
		struct ast_rule_item {
			inline ast_rule_item(int newNonterminalId, bool newIsTerminal, int newSymbolId, const contextfree::item_container& newItem)
			: nonterminalId(newNonterminalId)
			, isTerminal(newIsTerminal)
			, symbolId(newSymbolId)
			, item(newItem) {
			}

			/// \brief The ID of the nonterminal that this rule belongs to
			int nonterminalId;

			/// \brief True if this is a terminal item
			bool isTerminal;

			/// \brief The identifier for this symbol
			int symbolId;

			/// \brief The item defined at this position in the rule
			contextfree::item_container item;
		};
	}
}

#endif
