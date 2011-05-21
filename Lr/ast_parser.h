//
//  ast_parser.h
//  Parse
//
//  Created by Andrew Hunter on 21/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_AST_PARSER_H
#define _LR_AST_PARSER_H

#include "util/astnode.h"
#include "dfa/lexer.h"
#include "lr/parser.h"

namespace lr {
    ///
    /// \brief Parser actions that describe a parser that produces an AST
    ///
    class ast_parser_actions {
    public:
        /// \brief Type of an AST node
        typedef util::astnode astnode;
        
        /// \brief Container for an AST node
        typedef util::astnode_container astnode_container;
        
        /// \brief Type of a lexeme stream
        typedef dfa::lexeme_stream lexeme_stream;
        
        /// \brief Type of a parser that uses these actions
        typedef parser<astnode_container, ast_parser_actions> parser;
        
        /// \brief Type of a list of reduced symbols
        typedef parser::reduce_list reduce_list;
        
    private:
        /// \brief The stream of lexemes that this actions object will read from
        lexeme_stream* m_Stream;
        
    public:
        /// \brief Creates a new actions object that will read from the specified stream.
        ///
        /// The stream will be deleted when this object is deleted
        ast_parser_actions(dfa::lexeme_stream* stream)
        : m_Stream(stream) {
        }
        
        /// \brief Destroys an existing actions object
        ast_parser_actions() { delete m_Stream; }
        
        /// \brief Reads the next symbol from the stream
        inline dfa::lexeme* read() {
            dfa::lexeme* result = NULL;
            (*m_Stream) >> result;
            return result;
        }
        
        /// \brief Returns the item resulting from a shift action
        inline astnode_container shift(const dfa::lexeme_container& lexeme) {
            // Create a new node from the lexeme
            astnode* newNode = new astnode(lexeme);
            
            // Create the container for this node
            return astnode_container(newNode, true);
        }
        
        /// \brief Returns the item resulting from a reduce action
        inline astnode_container reduce(int nonterminal, int rule, const reduce_list& reduce) {
            // Create a new nonterminal node
            astnode* newNode = new astnode(nonterminal, rule);
            
            // Add the contents of the reduce list to this node
            newNode->add_children(reduce.begin(), reduce.end());
            
            // Create the container for this node
            return astnode_container(newNode, true);
        }
    };
    
    /// \brief A parser that produces an AST from the input source file
    typedef ast_parser_actions::parser ast_parser;
}

#endif
