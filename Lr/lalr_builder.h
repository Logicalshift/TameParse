//
//  lalr_builder.h
//  Parse
//
//  Created by Andrew Hunter on 01/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_LALR_BUILDER_H
#define _LR_LALR_BUILDER_H

#include "ContextFree/grammar.h"
#include "Lr/lalr_machine.h"

namespace lr {
    ///
    /// \brief Class that builds up a LALR state machine from a grammar
    ///
    class lalr_builder {
    private:
        /// \brief The grammar that this builder will use
        contextfree::grammar* m_Grammar;
        
        /// \brief The LALR machine that this is building up
        ///
        /// We store only the kernel states here.
        lalr_machine m_Machine;
        
    public:
        /// \brief Creates a new builder for the specified grammar
        lalr_builder(contextfree::grammar& gram);
        
        /// \brief Adds an initial state to this builder that will recognise the language specified by the supplied symbol
        ///
        /// To build a valid parser, you need to add at least one symbol. The builder will add a new state that recognises
        /// this language
        int add_initial_state(const contextfree::item_container& language);
        
        /// \brief Finishes building the parser (the LALR machine will contain a LALR parser after this call completes)
        void complete_parser();
        
        /// \brief Generates the lookaheads for the parser (when the machine has been built up as a LR(0) grammar)
        void complete_lookaheads();
        
        /// \brief The LALR state machine being built up by this object
        lalr_machine& machine() { return m_Machine; }
        
        /// \brief The LALR state machine being built up by this object
        const lalr_machine& machine() const { return m_Machine; }
        
        /// \brief The grammar used for this builder
        const contextfree::grammar& gram() const { return *m_Grammar; }
    };
}

#endif
