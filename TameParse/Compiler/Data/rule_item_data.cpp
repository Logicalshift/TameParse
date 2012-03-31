//
//  rule_item_data.cpp
//  Parse
//
//  Created by Andrew Hunter on 08/01/2012.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
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

#include "TameParse/Compiler/Data/rule_item_data.h"

using namespace std;
using namespace contextfree;
using namespace compiler;

/// \brief Creates a new, empty, rule item data object
rule_item_data::rule_item_data() 
: m_NextKey(1) {
}

/// \brief Sets the attributes for a particular item in a rule
void rule_item_data::set_attributes(rule& rule, const rule_iterator& pos, const rule_attributes& newAttributes) {
    // Fetch the key for this rule item
    int key = rule.get_key(pos);

    // If the key is unknown, then assign a new one
    if (key <= 0) {
        key = m_NextKey++;
        rule.set_key(pos, key);
    }

    // Store the attributes in the rule map
    m_RuleItemAttributes[key] = newAttributes;
}

/// \brief Sets the attributes for a particular item in a rule
void rule_item_data::set_attributes(rule& rule, size_t index, const rule_attributes& newAttributes) {
        // Fetch the key for this rule item
    int key = rule.get_key(index);

    // If the key is unknown, then assign a new one
    if (key <= 0) {
        key = m_NextKey++;
        rule.set_key(index, key);
    }

    // Store the attributes in the rule map
    m_RuleItemAttributes[key] = newAttributes;
}

/// \brief The default attributes for cases where there is nothing defined for a rule
static rule_item_data::rule_attributes s_DefaultAttributes;

/// \brief Returns the attributes for a particular item in a rule
const rule_item_data::rule_attributes& rule_item_data::attributes_for(const rule& rule, const rule_iterator& pos) const {
    // Get the key for this item
    int key = rule.get_key(pos);

    // Return the default attributes if this item has no key
    if (key <= 0) return s_DefaultAttributes;

    // Try to return the item attributes for this key
    rule_attribute_map::const_iterator found = m_RuleItemAttributes.find(key);
    if (found != m_RuleItemAttributes.end()) {
        return found->second;
    } else {
        return s_DefaultAttributes;
    }
}

/// \brief Returns the attributes for a particular item in a rule
const rule_item_data::rule_attributes& rule_item_data::attributes_for(const rule& rule, const size_t index) const {
    // Get the key for this item
    int key = rule.get_key(index);

    // Return the default attributes if this item has no key
    if (key <= 0) return s_DefaultAttributes;

    // Try to return the item attributes for this key
    rule_attribute_map::const_iterator found = m_RuleItemAttributes.find(key);
    if (found != m_RuleItemAttributes.end()) {
        return found->second;
    } else {
        return s_DefaultAttributes;
    }
}
