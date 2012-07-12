/*
 *  lex.c
 *  TameParse
 *
 *  Created by Andrew Hunter on 12/07/2012.
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

#include <stdlib.h>

#include "cparse.h"
#include "cparse_internal.h"
#include "binaryformat.h"

#define SUPPORTED_FILE_VERSION 0x100

#if SUPPORTED_FILE_VERSION != TP_CURRENTFORMATVERSION
# error C lexer must be updated to support the latest 
#endif

/**
 * \brief Definition of a lexer state
 */
struct tp_lexer_state {
    /** A pointer to the symbol map for this parser */
    const uint32_t* symbolMap;

    /** A pointer to the lexer state machine */
    const uint32_t* lexerStateMachine;

    /** A pointer to the lexer accepting states */
    const uint32_t* lexerAcceptingStates;
};

/**
 * \brief Creates a new lexer in the initial state
 *
 * \param parser The parser where the lexer is defined
 * \param userData The user data to pass in to the parser functions
 *
 * \return The final state of the parser
 */
tp_lexer_state tp_create_lexer(tp_parser parser, void* userData) {
    tp_lexer_state state;

    /* Sanity check */
    if (!parser) {
        return NULL;
    }

    if (!parser->data) {
        return NULL;
    }

    if (parser->data[TPH_FORMAT] != TP_FORMATINDICATOR) {
        return NULL;
    }

    if (parser->data[TPH_FILEFORMATVERSION] != SUPPORTED_FILE_VERSION) {
        return NULL;
    }

    /* Allocate space for the lexer */
    state                       = malloc(sizeof(struct tp_lexer_state));

    state->symbolMap            = parser->data + parser->data[TPH_LEXER_SYMBOLMAP];
    state->lexerStateMachine    = parser->data + parser->data[TPH_LEXER_STATEMACHINE];
    state->lexerAcceptingStates = parser->data + parser->data[TPH_LEXER_ACCEPTINGSTATES];

    /* Done; ready to start reading data */
    return state;
}

/**
 * \brief Frees up a lexer state
 */
void tp_free_lexer(tp_lexer_state state) {

}

/**
 * \brief Matches a single symbol using the lexer
 *
 * \param state A lexer state
 *
 * \return tp_lex_nothing if no symbols are matched, otherwise 
 *
 * At least one character is always removed from
 */
int tp_lex_symbol(tp_lexer_state state) {

}
