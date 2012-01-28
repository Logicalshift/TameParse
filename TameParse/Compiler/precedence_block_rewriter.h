//
//  precedence_block_rewriter.h
//  TameParse
//
//  Created by Andrew Hunter on 22/01/2012.
//  Copyright (c) 2012 Andrew Hunter. All rights reserved.
//

#ifndef _COMPILER_PRECEDENCE_BLOCK_REWRITER_H
#define _COMPILER_PRECEDENCE_BLOCK_REWRITER_H

#include <map>

#include "TameParse/ContextFree/terminal_dictionary.h"
#include "TameParse/Lr/precedence_rewriter.h"
#include "TameParse/Language/precedence_block.h"

namespace compiler {
	///
	/// \brief Rewriter that uses the contents of a precedence block to resolve shift/reduce conflicts
	///
	class precedence_block_rewriter : public lr::precedence_rewriter {
	private:
		/// \brief Maps terminal symbols to their precedence
		std::map<int, int> m_TerminalPrecedence;

		/// \brief Maps terminal symbols to their associativity
		std::map<int, associativity> m_TerminalAssociativity;

	public:
		/// \brief Construcuts a new rewriter by interpreting a language block
		precedence_block_rewriter(const contextfree::terminal_dictionary& terminals, const language::precedence_block& precedence);

        /// \brief Creates a clone of this rewriter
        virtual action_rewriter* clone() const;

        /// \brief Retrieves the precedence of a particular symbol
        ///
        /// Subclasses should implement this to supply information about symbol
        /// precedence. A value of c_NoPrecedence indicates that a symbol does not
        /// supply a precedence. Symbols with higher precedence are reduced first.
       	virtual int get_precedence(const contextfree::item_container& terminal, const lr::lr0_item_set& shiftItems) const;

       	/// \brief Retrieves the associativity of a particular symbol
       	///
       	/// This is used when trying to disambiguate a shift/reduce conflict where
       	/// both sides have equal precedence. Note that only the associativity of
       	/// the rule has an effect on the result; this method is called by the default
       	/// implementation of get_rule_associativity to determine the associativity of
       	/// the operator it finds in the rule.
       	virtual associativity get_associativity(const contextfree::item_container& terminal, const lr::lr0_item_set& shiftItems) const;
	};
}

#endif
