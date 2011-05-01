//
//  lr_item.cpp
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "lr_item.h"

using namespace contextfree;
using namespace lr;

/// \brief Creates an LR(0) item by copying a rule
lr0_item::lr0_item(const grammar* gram, const contextfree::rule& r, int offset)
: m_Rule(r)
, m_Offset(offset)
, m_Grammar(gram) {
}

/// \brief Creates an LR(0) item by referencing an existing rule
lr0_item::lr0_item(const grammar* gram, contextfree::rule* r, int offset)
: m_Rule(r)
, m_Offset(offset)
, m_Grammar(gram) {
}

/// \brief Creates a copy of an existing LR(0) item
lr0_item::lr0_item(const lr0_item& copyFrom)
: m_Rule(copyFrom.m_Rule)
, m_Offset(copyFrom.m_Offset)
, m_Grammar(copyFrom.m_Grammar) {
}

/// \brief Copies an LR(0) item into this one
lr0_item& lr0_item::operator=(const lr0_item& copyFrom) {
    if (&copyFrom == this) return *this;
    
    m_Rule      = copyFrom.m_Rule;
    m_Offset    = copyFrom.m_Offset;
    m_Grammar   = copyFrom.m_Grammar;
    
    return *this;
}

/// \brief Orders an LR(0) item
bool lr0_item::operator<(const lr0_item& compareTo) const {
    if (m_Offset < compareTo.m_Offset) return true;
    if (m_Offset > compareTo.m_Offset) return false;
    
    int ourId   = m_Grammar->identifier_for_rule(rule());
    int theirId = m_Grammar->identifier_for_rule(compareTo.rule());
    
    return ourId < theirId;
}

/// \brief Compares two LR(0) items
bool lr0_item::operator==(const lr0_item& compareTo) const {
    if (m_Offset != compareTo.m_Offset) return false;
    
    int ourId   = m_Grammar->identifier_for_rule(rule());
    int theirId = m_Grammar->identifier_for_rule(compareTo.rule());
    
    return ourId == theirId;
}

/// \brief Constructs an LR(1) item by appending lookahead to an LR(0) item
lr1_item::lr1_item(const lr0_item& core, const lookahead_set& lookahead)
: m_Lr0Item(core)
, m_LookAhead(lookahead) {
}

/// \brief Constructs an LR(1) item by copying a rule
lr1_item::lr1_item(const grammar* gram, const contextfree::rule& rule, int offset, const lookahead_set& lookahead) 
: m_Lr0Item(gram, rule, offset)
, m_LookAhead(lookahead) {
}

/// \brief Constructs an LR(1) item by creating a reference to an existing rule
lr1_item::lr1_item(const grammar* gram, contextfree::rule* rule, int offset, const lookahead_set& lookahead)
: m_Lr0Item(gram, rule, offset)
, m_LookAhead(lookahead) {
}

/// \brief Creates a copy of an LR(1) item
lr1_item::lr1_item(const lr1_item& copyFrom)
: m_Lr0Item(copyFrom)
, m_LookAhead(copyFrom.lookahead()) {
}

/// \brief Assigns the context of this LR(1) item from another item
lr1_item& lr1_item::operator=(const lr1_item& copyFrom) {
    if (&copyFrom == this) return *this;
    
    m_Lr0Item   = copyFrom;
    m_LookAhead = copyFrom.lookahead();
    
    return *this;
}

/// \brief Orders an LR(1) item
bool lr1_item::operator<(const lr1_item& compareTo) const {
    size_t lookaheadSize        = lookahead().size();
    size_t compareLookaheadSize = compareTo.lookahead().size();
    
    // Compare the lookahead set sizes
    if (lookaheadSize < compareLookaheadSize) return true;
    if (compareLookaheadSize < lookaheadSize) return false;
    
    // Compare the LR(0) item
    if (m_Lr0Item > compareTo)          return false;
    else if (m_Lr0Item != compareTo)    return true;
    
    // Compare the lookahead content itself
    lookahead_set::const_iterator ourLookahead      = lookahead().begin();
    lookahead_set::const_iterator theirLookahead    = compareTo.lookahead().begin();
    
    for (; ourLookahead != lookahead().end() && theirLookahead != compareTo.lookahead().end(); ourLookahead++, theirLookahead++) {
        if (*ourLookahead > *theirLookahead)        return false;
        else if (*ourLookahead != *theirLookahead)  return true;
    }
    
    // Items are equal
    return false;
}

/// \brief Compares two LR(1) items
bool lr1_item::operator==(const lr1_item& compareTo) const {
    size_t lookaheadSize        = lookahead().size();
    size_t compareLookaheadSize = compareTo.lookahead().size();
    
    // Compare the lookahead set sizes
    if (lookaheadSize != compareLookaheadSize) return false;
    
    // Compare the LR(0) item
    if (m_Lr0Item != compareTo)          return false;
    
    // Compare the lookahead content itself
    lookahead_set::const_iterator ourLookahead      = lookahead().begin();
    lookahead_set::const_iterator theirLookahead    = compareTo.lookahead().begin();
    
    for (; ourLookahead != lookahead().end() && theirLookahead != compareTo.lookahead().end(); ourLookahead++, theirLookahead++) {
        if (*ourLookahead != *theirLookahead)        return false;
    }
    
    // Items are equal
    return true;
    
}
