//
//  output_stage_data.h
//  TameParse
//
//  Created by Andrew Hunter on 10/12/2011.
//  
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

#ifndef _COMPILER_OUTPUT_STAGE_DATA_H
#define _COMPILER_OUTPUT_STAGE_DATA_H

#include <vector>
#include <map>

#include "TameParse/ContextFree/item.h"

namespace compiler {
    /// \brief Structures used to supply output to an output stage
    namespace data {
        ///
        /// \brief Represents a terminal symbol
        ///
        struct terminal_symbol {
            /// \brief Constructs a new terminal symbol
            inline terminal_symbol(const std::wstring& newName, int newIdentifier, const contextfree::item_container& newItem)
            : name(newName)
            , identifier(newIdentifier)
            , item(newItem) { }

            /// \brief The name of this symbol
            std::wstring name;

            /// \brief The identifier of this symbol
            int identifier;

            /// \brief The context free item representing this terminal symbol
            contextfree::item_container item;
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
        /// \brief Represents an item in a rule
        ///
        struct ast_rule_item {
            inline ast_rule_item(bool newIsTerminal, int newSymbolId, const contextfree::item_container& newItem, const std::wstring& newUniqueName, bool newIsRepetition)
            : isTerminal(newIsTerminal)
            , symbolId(newSymbolId)
            , item(newItem)
            , uniqueName(newUniqueName)
            , isEbnfRepetition(newIsRepetition) {
            }

            /// \brief True if this is a terminal item
            bool isTerminal;

            /// \brief True if this item is the item representing a repetition in an EBNF closure
            /// 
            /// This can be true for rules for nonterminals of kind item::repeat or item::repeat_zero_or_one.
            bool isEbnfRepetition;

            /// \brief The identifier for this symbol
            int symbolId;

            /// \brief A unique name within the rule for this item (based on the name of the item itself)
            ///
            /// Item names are re-used within a nonterminal but not within a rule, so
            /// if a rule contains the same item multiple times, each usage can be uniquely
            /// addressed.
            std::wstring uniqueName;

            /// \brief The item defined at this position in the rule
            contextfree::item_container item;
        };

        /// \brief List of rule items
        typedef std::vector<ast_rule_item> ast_rule_item_list;

        /// \brief The rules within an nonterminal (maps rule IDs to items)
        typedef std::map<int, ast_rule_item_list> ast_nonterminal_rules;

        ///
        /// \brief Definition of a nonterminal within the AST
        ///
        struct ast_nonterminal {
            /// \brief Identifier for this nonterminal
            int nonterminalId;

            /// \brief The rules for this nonterminal
            ast_nonterminal_rules rules;
        };
    }
}

#endif
