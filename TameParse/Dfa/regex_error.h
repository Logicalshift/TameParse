//
//  regex_error.h
//  Parse
//
//  Created by Andrew Hunter on 22/11/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
//

#ifndef _DFA_REGEX_ERROR_H
#define _DFA_REGEX_ERROR_H

#include <string>

#include "TameParse/Dfa/position.h"

namespace dfa {
	/// \brief Class representing an error in a regular expression
	class regex_error {
	public:
		/// \brief The types of error that can be detected within a regular expression
		enum error_type {
			/// \brief Unknown type of error
			unknown,

			/// \brief Missing ')'
			missing_round_bracket,

			/// \brief Missing ']'
			missing_square_bracket,

			/// \brief Missing '}'
			missing_curly_bracket,

			/// \brief An expression that doesn't resolve to anything
			missing_expression
		};

	private:
		/// \brief The type of error
		error_type m_Type;

		/// \brief Position (relative to the regular expression)
		position m_Position;

		/// \brief Name of the symbol that the error applies to (where applicable)
		std::basic_string<int> m_Symbol;

	public:
		/// \brief Constructor
		regex_error(error_type type, const position& pos);

		/// \brief Constructor
		regex_error(error_type type, const position& pos, const std::basic_string<int>& symbol);

		/// \brief The type of error
		inline error_type type() const { return m_Type; }

		/// \brief The position of this error within the regular expression
		inline const position& pos() const { return m_Position; }

		/// \brief Name of the symbol that the error applies to (where applicable)
		const std::basic_string<int>& symbol() const { return m_Symbol; }
	};
}

#endif
