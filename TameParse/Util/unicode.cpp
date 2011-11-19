//
//  unicode.cpp
//  TameParse
//
//  Created by Andrew Hunter on 11/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/common.h"

#include <cstdlib>
#include <algorithm>
#include "unicode.h"

using namespace std;
using namespace dfa;
using namespace util;

// The unicode_data.h file contains the static variable declarations
#include "TameParse/Unicode/unicode_data.h"

/// \brief Type of a symbol range
typedef symbol_set::symbol_range symbol_range;

/// \brief Entry in a character map table
typedef unicode::map_entry map_entry;

/// \brief Compares two entries in a character map
static bool compare_map_entries(const map_entry& first, const map_entry& second) {
	// Compare the second entry in the item (the upper bound of the character range)
	return first.upper < second.upper;
}

/// \brief Converts a range using the specified map
static symbol_set convert_range(const symbol_range& range, const map_entry* map, int count) {
	// Start building the result
	symbol_set result;

	// Current lower and upper bounds remaining to be processed
	int lowerChar = range.lower();
	int upperChar = range.upper();

	// Search for the first item that is >= lower
	map_entry searchItem		= { lowerChar, lowerChar, 0 };
	const map_entry* curItem 	= upper_bound(map, map + count, searchItem, compare_map_entries);
	const map_entry* lastItem	= map + count;

	// Iterate until we have processed all of the items in this range
	while (upperChar > lowerChar) {
		// Create a new range using the current item
		if (curItem == lastItem) {
			// Reached the end of the array - (lower, upper) should be copied in directly
			result |= symbol_range(lowerChar, upperChar);
			lowerChar = upperChar;
			break;
		}

		// If the current item's lower bound is > lowerChar, then this range is does not have a mapped equivalent
		if (curItem->lower > lowerChar) {
			// Get the final character of this range
			int endChar = curItem->lower;
			if (endChar > upperChar) {
				endChar = upperChar;
			}

			// Merge in this range
			result |= symbol_range(lowerChar, endChar);

			// Move on
			lowerChar = endChar;
		} else {
			// Map in the equivalent character range from the map

			// The mapped character that's equivalent to lowerChar
			int initialChar = curItem->map_to + (lowerChar - curItem->lower);

			// The number of characters to map
			int numChars = curItem->upper - curItem->lower;
			if (numChars > upperChar - lowerChar) {
				numChars = upperChar - lowerChar;
			}

			// Merge in the mapped range
			result |= symbol_range(initialChar, initialChar + numChars);

			// Move on
			curItem++;
			lowerChar += numChars;
		}
	}

	return result;
}

/// \brief Returns the uppercase equivalent of the specified symbol set
symbol_set unicode::to_upper(const symbol_set& source) {
	symbol_set result;

	// Iterate through the symbols in the source
	for (symbol_set::iterator toMap = source.begin(); toMap != source.end(); toMap++) {
		// Map this range and merge it into the result
		result |= convert_range(*toMap, s_UppercaseMap, s_UppercaseMapSize);
	}

	return result;
}

/// \brief Returns the lowercase equivalent of the specified symbol set
symbol_set unicode::to_lower(const symbol_set& source) {
	symbol_set result;

	// Iterate through the symbols in the source
	for (symbol_set::iterator toMap = source.begin(); toMap != source.end(); toMap++) {
		// Map this range and merge it into the result
		result |= convert_range(*toMap, s_LowercaseMap, s_LowercaseMapSize);
	}

	return result;
}
