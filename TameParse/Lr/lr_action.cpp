//
//  lr_action.cpp
//  Parse
//
//  Created by Andrew Hunter on 04/05/2011.
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

#include "TameParse/Lr/lr_action.h"

using namespace contextfree;
using namespace lr;

static empty_item an_empty_item;

/// \brief Empty rule
static rule an_empty_rule(an_empty_item);

/// \brief Creates a shift or goto action (with no rule)
lr_action::lr_action(action_type type, const contextfree::item_container& item, int nextState)
: m_Type(type)
, m_Item(item)
, m_NextState(nextState)
, m_Rule(&an_empty_rule, false) {
}

/// \brief Creates a reduce action (with a rule to reduce)
lr_action::lr_action(action_type type, const contextfree::item_container& item, int nextState, const contextfree::rule_container& rule)
: m_Type(type)
, m_Item(item)
, m_NextState(nextState)
, m_Rule(rule) {
}

/// \brief Copy constructor
lr_action::lr_action(const lr_action& copyFrom)
: m_Type(copyFrom.m_Type)
, m_Item(copyFrom.m_Item)
, m_NextState(copyFrom.m_NextState)
, m_Rule(copyFrom.m_Rule) {
}

/// \brief Copy constructor, modifies the item for an action
lr_action::lr_action(const lr_action& copyFrom, const contextfree::item_container& newItem)
: m_Type(copyFrom.m_Type)
, m_Item(newItem)
, m_NextState(copyFrom.m_NextState)
, m_Rule(copyFrom.m_Rule) {
}

