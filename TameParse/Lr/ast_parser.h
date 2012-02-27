//
//  ast_parser.h
//  Parse
//
//  Created by Andrew Hunter on 21/05/2011.
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

#ifndef _LR_AST_PARSER_H
#define _LR_AST_PARSER_H

#include "TameParse/Util/astnode.h"
#include "TameParse/Dfa/lexer.h"
#include "TameParse/Lr/parser.h"

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
        typedef parser<astnode_container, ast_parser_actions> ast_parser;
        
        /// \brief Type of a list of reduced symbols
        typedef ast_parser::reduce_list reduce_list;
        
    private:
        /// \brief The stream of lexemes that this actions object will read from
        lexeme_stream* m_Stream;
        
        ast_parser_actions(const ast_parser_actions& copyFrom);
        ast_parser_actions& operator=(ast_parser_actions& copyFrom);
        
    public:
        /// \brief Creates a new actions object that will read from the specified stream.
        ///
        /// The stream will be deleted when this object is deleted
        ast_parser_actions(dfa::lexeme_stream* stream)
        : m_Stream(stream) {
        }
        
        /// \brief Destroys an existing actions object
        ~ast_parser_actions() { delete m_Stream; }
        
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
        inline astnode_container reduce(int nonterminal, int rule, const reduce_list& reduce, const dfa::position& lookaheadPosition) {
            // Create a new nonterminal node
            astnode* newNode = new astnode(nonterminal, rule);
            
            // Add the contents of the reduce list to this node
            newNode->add_children(reduce.rbegin(), reduce.rend());
            
            // Create the container for this node
            return astnode_container(newNode, true);
        }
    };
    
    /// \brief A parser that produces an AST from the input source file
    typedef ast_parser_actions::ast_parser ast_parser;
}

#endif
