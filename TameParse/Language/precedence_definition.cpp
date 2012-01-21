//
//  precedence_definition.cpp
//  TameParse
//
//  Created by Andrew Hunter on 21/01/2012.
//  Copyright (c) 2012 Andrew Hunter. All rights reserved.
//

#include "TameParse/Language/precedence_definition.h"

using namespace std;
using namespace language;

/// \brief Creates a new precedence definition
precedence_definition::precedence_definition(position& start, position& end)
: block(start, end) {
}

/// \brief Destructor
precedence_definition::~precedence_definition() {
	
}

/// \brief Adds a new item to this precedence definition
void precedence_definition::add_item(item& newItem) {
	// Add this item
	m_List.push_back(newItem);
}

/// \brief The item with the highest precedence
precedence_definition::iterator precedence_definition::begin() const {
	return m_List.begin();
}

/// \brief The item after the item with the lowest precedence
precedence_definition::iterator precedence_definition::end() const {
	return m_List.end();
}
