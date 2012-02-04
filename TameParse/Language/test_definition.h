//
//  test_definition.h
//  TameParse
//
//  Created by Andrew Hunter on 18/10/2011.
//  Copyright (c) 2011 Andrew Hunter. All rights reserved.
//

#ifndef _LANGUAGE_TEST_DEFINITION_H
#define _LANGUAGE_TEST_DEFINITION_H

#include <string>

#include "TameParse/Language/block.h"

namespace language {
	/// \brief Class that represents a test definition in the parser language
	class test_definition : public block {
	public:
		/// \brief The possible types of test definition
		enum test_type {
			/// \brief The string should match the specified nonterminal
			match,

			/// \brief The string should not match the specified nonterminal
			no_match,

			/// \brief The string specifies the name of the file whose content should match the nonterminal
			match_from_file
		};

	private:
		/// \brief The empty string, or the name of the language that the nonterminal is defined in
		std::wstring m_NonterminalLanguage;

		/// \brief The name of the nonterminal that this test should match
		std::wstring m_Nonterminal;

		/// \brief The type of this test
		test_type m_Type;

		/// \brief The empty string, or the identifier for this test
		std::wstring m_Identifier;

		/// \brief The string that should match the nonterminal, or the name of the file to load
		std::wstring m_TestString;

		/// \brief The position of the test string in the file (can be used to compute where parse errors occurred)
		position m_TestStringPosition;

	public:
		/// \brief Creates a new test definition
		test_definition(const std::wstring& nonterminalLanguage, const std::wstring& nonterminal, test_type testType, const std::wstring& identifier, const std::wstring& testString, const position& start, const position& end, const position& testStringPos);

		/// \brief Destructor
		virtual ~test_definition();

		/// \brief The empty string, or the name of the language that the nonterminal is defined in
		inline const std::wstring& nonterminal_language() { return m_NonterminalLanguage; }

		/// \brief The name of the nonterminal that this test should match
		inline const std::wstring& nonterminal() { return m_Nonterminal; }

		/// \brief The type of this test
		inline test_type type() { return m_Type; }

		/// \brief The empty string, or the identifier for this test
		inline const std::wstring& identifier() { return m_Identifier; }

		/// \brief The string that should match the nonterminal, or the name of the file to load
		inline const std::wstring& test_string() { return m_TestString; }

		/// \brief The position of the first character in the test string
		inline const position& test_string_position() { return m_TestStringPosition; }
	};
}

#endif
