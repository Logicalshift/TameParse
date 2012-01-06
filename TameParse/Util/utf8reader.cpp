//
//  utf8reader.cpp
//  TameParse
//
//  Created by Andrew Hunter on 24/09/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#include "TameParse/Util/utf8reader.h"

using namespace std;
using namespace util;

/// \brief Creates a new UTF-8 reader
///
/// Set ownsReader to true to specify that this object owns its stream and should
/// dispose of it when it is freed.
utf8reader::utf8reader(std::istream* inputStream, bool ownsReader) 
: m_InputStream(inputStream)
, m_OwnsStream(ownsReader)
, m_BadUTF8(false)
, m_PairChar(0) {
	
}

/// \brief Destructor for this object
utf8reader::~utf8reader() {
	if (m_OwnsStream && m_InputStream) {
		// Finished with the input stream
		delete m_InputStream;
	}
}

/// \brief Places the next unicode character in the target
///
/// This will read multiple characters from the source stream until an entire
/// unicode character has been constructed. In the case where there is a problem,
/// the target will be set to 0 and good() will return false;
utf8reader& utf8reader::get(wchar_t& target) {
	// Deal with the case where there's a surrogate pair
	if (m_PairChar) {
		target 		= m_PairChar;
		m_PairChar 	= 0;
		return *this;
	}

	// Pathological case
	if (!m_InputStream) {
		target = 0;
		return *this;
	}

	// Give up immediately if the input stream is bad or we've hit a bad character
	if (!m_InputStream->good() || m_BadUTF8) {
		target = 0;
		return *this;
	}

	// A byte type
	typedef unsigned char byte;

	// Read the first character
	byte firstChar = m_InputStream->get();

	// Give up if the stream goes bad
	if (!m_InputStream->good()) {
		m_BadUTF8 	= true;
		target 		= 0;
		return *this;
	}

	// Characters less than 0x80 are passed through intact
	if (firstChar < 0x80) {
		target = firstChar;
		return *this;
	}

	// Work out how many bytes are in the complete character
	else if ((target & 0xe0) == 0xc0) {
		// Begins 110xxxx (0x80 - 0x7ff)

		// Read the remaining characters
		byte secondChar = m_InputStream->get();

		// Stop if the stream is no longer good, or the character is bad
		if (!m_InputStream->good() || (secondChar & 0xc0) != 0x80) {
			m_BadUTF8 	= true;
			target 		= 0;
			return *this;
		}

		// Create the result
		target = ((firstChar&0x1f)<<6) | (secondChar&0x3f);
		return *this;
	} else if ((target & 0xf0) == 0xe0) {
		// Begins 1110xxxx (0x800 - 0xffff)

		// Read the remaining characters
		byte secondChar	= m_InputStream->get();
		byte thirdChar	= m_InputStream->get();

		// Stop if the stream is no longer good, or the character is bad
		if (!m_InputStream->good() || (secondChar & 0xc0) != 0x80 || (thirdChar & 0xc0) != 0x80) {
			m_BadUTF8	= true;
			target		= 0;
			return *this;
		}

		// Create the result
		target = ((firstChar&0xf)<<12) | ((secondChar&0x3f)<<6) | (thirdChar&0x3f);
		return *this;
	} else if ((target & 0xf8) == 0xf0) {
		// Begins 11110000 (0x10000 - 0x1fffff)

		// Read the remaining characters
		byte secondChar	= m_InputStream->get();
		byte thirdChar	= m_InputStream->get();
		byte fourthChar	= m_InputStream->get();

		// Stop if the stream is no longer good, or the character is bad
		if (!m_InputStream->good() || (secondChar & 0xc0) != 0x80 || (thirdChar & 0xc0) != 0x80 || (fourthChar & 0xc0) != 0x80) {
			m_BadUTF8	= true;
			target		= 0;
			return *this;
		}

		// Construct the UCS-4 character
		unsigned int ucs4 = ((firstChar&0x7)<<18) | ((secondChar&0x3f)<<12) | ((thirdChar&0x3f)<<6) | (secondChar&0x3f);

		// Must be less than 0x110000 to be a valid surrogate pair
		if (ucs4 >= 0x110000) {
			m_BadUTF8	= true;
			target		= 0;
			return *this;
		}

		// Convert to a surrogate pair
		ucs4 -= 0x10000;

		// Set the low surrogate (will be returned second)
		m_PairChar = 0xdc00 + (ucs4&0x3ff);

		// Result is the high surrogate
		target = 0xd800 + ((ucs4>>10)&0x3ff);
		return *this;
	} else {
		// Other characters cannot be represented as UTF-16
		m_BadUTF8 	= true;
		target 		= 0;
		return *this;
	}
}

/// \brief True if the stream is good
bool utf8reader::good() const {
	return !m_BadUTF8 && m_InputStream->good();
}
