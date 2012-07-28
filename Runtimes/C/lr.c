/*
 *  lr.c
 *  TameParse
 *
 *  Created by Andrew Hunter on 28/07/2012.
 *  
 *  Copyright (c) 2011-2012 Andrew Hunter
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy 
 *  of this software and associated documentation files (the \"Software\"), to 
 *  deal in the Software without restriction, including without limitation the 
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 *  sell copies of the Software, and to permit persons to whom the Software is 
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 *  IN THE SOFTWARE.
 */

#include "cparse.h"
#include "cparse_internal.h"

/**
 * \brief Runs the parser and returns the final state
 *
 * \param parser The parser to run
 * \param initialStateId The initial 
 * \param userData The user data to pass in to the parser functions
 *
 * \return The final state of the parser
 */
tp_parser_state tp_parse(tp_parser parser, int initialStateId, void* userData) {

}

/**
 * \brief Creates the initial state for the parser
 *
 * \param parser A parser created by tp_create_parser
 * \param stateId The initial state ID to begin parsing at
 * \param userData The user data to pass in to the parser functions
 *
 * \return NULL if no state can be created, otherwise a parser state in the initial state for the parser
 *
 * The initial state will be 0 in many cases. However, TameParse can generate
 * parsers with numerous start symbols to support parsing different aspects of
 * a language. These parsers will have more than one start state.
 */
tp_parser_state tp_create_initial_state(tp_parser parser, int stateId, void* userData) {
    
}

/**
 * \brief Creates a new parser state by copying an existing state
 */
tp_parser_state tp_copy_state(tp_parser_state oldState) {

}

/**
 * \brief Frees a parser state
 */
void tp_free_state(tp_parser_state oldState) {

}

/**
 * \brief Returns 0 if the parser is not in an accepting state, or 1 if it is
 */
int tp_state_accepting(tp_parser_state state) {

}

/**
 * \brief Returns 0 if the parser is not in an error state, or 1 if it is
 */
int tp_state_error(tp_parser_state state) {

}

/**
 * \brief Returns the ID of the current location in the state machine
 *
 * \param state The parser state to retrieve the ID for
 */
int tp_state_get_current(tp_parser_state state) {

}

/**
 * \brief Retrieves the stack depth in the specified state
 */
int tp_state_get_stack_depth(tp_parser_state state) {

}

/**
 * \brief Creates a new state by popping entries from the stack
 */
tp_parser_state tp_state_peek(tp_parser_state state, int depth) {

}

/**
 * \brief Attempts to shift the specified symbol on to the given parser state
 */
void tp_state_shift(tp_parser_state state, int terminalId, int** symbols, int numSymbols) {

}
