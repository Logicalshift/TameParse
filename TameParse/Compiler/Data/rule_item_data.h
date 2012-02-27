//
//  rule_item_data.h
//  Parse
//
//  Created by Andrew Hunter on 08/01/2012.
//  Copyright 2012 Andrew Hunter. All rights reserved.
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
        typedef language::ebnf_item_attributes rule_attributes;

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
