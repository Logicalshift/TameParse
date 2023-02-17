//
//  rule_item_data.h
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

#ifndef _COMPILER_DATA_RULE_ITEM_DATA_H
#define _COMPILER_DATA_RULE_ITEM_DATA_H

#include <map>

#include "TameParse/ContextFree/rule.h"
#include "TameParse/Language/ebnf_item_attributes.h"

namespace compiler {
    ///
    /// \brief Class that associated items in rules with corresponding data
    ///
    class rule_item_data {
    public:
        /// \brief Type of an attribute associated with a rule item key
        typedef yy_language::ebnf_item_attributes rule_attributes;

        /// \brief Maps rule item keys to the associated attributes
        typedef std::map<int, rule_attributes> rule_attribute_map;

        /// \brief The type of a rule
        typedef contextfree::rule rule;

        /// \brief Type of a rule iterator
        typedef contextfree::rule::iterator rule_iterator;

    private:
        /// \brief The next key to assign to an item in a rule
        int m_NextKey;

        /// \brief The attributes applied to each rule item
        rule_attribute_map m_RuleItemAttributes;

    public:
        /// \brief Creates a new, empty, rule item data object
        rule_item_data();

        /// \brief Sets the attributes for a particular item in a rule
        void set_attributes(rule& rule, const rule_iterator& pos, const rule_attributes& newAttributes);

        /// \brief Sets the attributes for a particular item in a rule
        void set_attributes(rule& rule, size_t index, const rule_attributes& newAttributes);

        /// \brief Returns the attributes for a particular item in a rule
        const rule_attributes& attributes_for(const rule& rule, const rule_iterator& pos) const;

        /// \brief Returns the attributes for a particular item in a rule
        const rule_attributes& attributes_for(const rule& rule, const size_t index) const;
    };
}

#endif
