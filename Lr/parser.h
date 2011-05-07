//
//  parser.h
//  Parse
//
//  Created by Andrew Hunter on 07/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_PARSER_H
#define _LR_PARSER_H

#include "Lr/lalr_builder.h"
#include "Dfa/lexer.h"

namespace lr {
    ///
    /// \brief A LR(1) parser
    ///
    class parser {
    public:
        ///
        /// \brief Description of a parser action
        ///
        struct action {
            /// \brief The type of action (same values as specified by lr_action)
            unsigned int m_Type : 8;
            
            /// \brief The state to enter when this symbol is matched (or the rule to reduce for reduce actions)
            unsigned int m_NextState : 24;
            
            /// \brief The symbol ID. 
            ///
            /// For shift or reduce actions, this is the terminal identifier. For all other actions,
            /// this is the item ID as supplied by the source grammar.
            int m_SymbolId;
        };
        
        ///
        /// \brief Description of a reduce rule
        ///
        struct reduce_rule {
            /// \brief The identifier for this rule (within the grammar)
            int m_Identifier;
            
            /// \brief Number of items in this rule
            int m_Length;
        };
        
    private:
        /// \brief The number of states in this parser
        int m_NumStates;
        
        /// \brief The terminal action table (sorted in order of terminal ID)
        action** m_TerminalActions;
        
        /// \brief The nonterminal action table (goto actions, guard actions)
        action** m_NonterminalActions;
        
        /// \brief The reduce rules
        reduce_rule* m_Rules;
        
    public:
        /// \brief Creates a parser from the result of the specified builder class
        parser(const lalr_builder& builder);

        /// \brief Copy constructor
        parser(const parser& copyFrom);
        
        /// \brief Assignment
        parser& operator=(const parser& copyFrom);

        /// \brief Destructor
        virtual ~parser();
        
        /// \brief Parses the specified lexeme string
        void parse(dfa::lexeme_stream& input);
    };
}

#endif
