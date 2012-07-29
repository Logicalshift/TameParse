/*
 *  lookahead.c
 *  TameParse
 *
 *  Created by Andrew Hunter on 29/07/2012.
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
 * \brief Inserts a lookahead item into the specified state (before any other lookahead)
 */
void tp_lookahead_push(tp_parser_state state, int terminalId, const int* symbols, int numSymbols) {

}

/**
 * \brief Throws away the lookahead pointed to by the specified state
 */
void tp_lookahead_pop(tp_parser_state state) {

}

/**
 * \brief Retrieves the terminal at the current lookahead position
 */
int tp_lookahead_terminal(tp_parser_state state) {

}

/**
 * \brief Retrieves the symbols at the current lookahead position
 *
 * This returns the number of symbols and updates result so that it points
 * at the symbols themselves.
 */
int tp_lookahead_symbols(tp_parser_state state, const int** result) {
    
}
