//
//  lr_action.cpp
//  Parse
//
//  Created by Andrew Hunter on 04/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/common.h"

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

