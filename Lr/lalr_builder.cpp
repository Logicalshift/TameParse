//
//  lalr_builder.cpp
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "lalr_builder.h"

using namespace contextfree;
using namespace lr;

/// \brief Creates a new builder for the specified grammar
lalr_builder::lalr_builder(contextfree::grammar& gram)
: m_Grammar(&gram)
, m_Machine(gram) {
    
}

/// \brief Adds an initial state to this builder that will recognise the language specified by the supplied symbol
///
/// To build a valid parser, you need to add at least one symbol. The builder will add a new state that recognises
/// this language
int lalr_builder::add_initial_state(const contextfree::item_container& language) {
    // Create a new rule for this language ('<language>' $)
    empty_item      empty;
    end_of_input    eoi;
    rule            languageRule(empty);
    
    languageRule << language << eoi;
    
    // Create the initial item with an empty lookahead (we only store kernels in the lalr machine)
    lalr_state  initialState;
    lr1_item    item(m_Grammar, languageRule, 0, lr1_item::lookahead_set());
    
    initialState.add(item);
    
    // Add this to the machine
    lalr_machine::container c(initialState);
    return m_Machine.add_state(c);
}

/// \brief Finishes building the parser (the LALR machine will contain a LALR parser after this call completes)
void lalr_builder::complete_parser() {
    
}
