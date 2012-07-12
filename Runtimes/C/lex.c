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

#define SUPPORTED_FILE_VERSION  0x100
#define DEFAULT_LOOKAHEAD_SIZE  32

#if SUPPORTED_FILE_VERSION != TP_CURRENTFORMATVERSION
# error C lexer must be updated to support the latest binary file version
#endif

/**
 * \brief Definition of a lexer state
 */
struct tp_lexer_state {
    /** The parser for this state */
    tp_parser parser;

    /** A pointer to the symbol map for this parser */
    const uint32_t* symbolMap;

    /** A pointer to the lexer state machine */
    const uint32_t* lexerStateMachine;

    /** A pointer to the lexer accepting states */
    const uint32_t* lexerAcceptingStates;

    /** The identifier of the current state */
    int curState;

    /** The size of the lookahead buffer */
    int lookaheadSize;

    /** The current position in the lookahead buffer */
    int lookaheadPos;

    /** The last valid character in the lookahead buffer */
    int lookaheadEndPos;

    /** The lookahead buffer */
    int* lookahead;

    /** The lookahead symbol set buffer */
    int* lookaheadSets;

    /** tp_lex_nothing, or the symbol accepted at acceptPos */
    int acceptSymbol;

    /** 1 if we've reached the end of the file */
    int endOfFile;
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

    state->parser               = parser;
    state->symbolMap            = parser->data + parser->data[TPH_LEXER_SYMBOLMAP];
    state->lexerStateMachine    = parser->data + parser->data[TPH_LEXER_STATEMACHINE];
    state->lexerAcceptingStates = parser->data + parser->data[TPH_LEXER_ACCEPTINGSTATES];
    state->curState             = 0;
    state->lookaheadSize        = DEFAULT_LOOKAHEAD_SIZE;
    state->lookaheadPos         = 0;
    state->lookaheadEndPos      = 0;
    state->lookahead            = calloc(sizeof(int), state->lookaheadSize);
    state->lookaheadSets        = calloc(sizeof(int), state->lookaheadSize);
    state->acceptSymbol         = tp_lex_nothing;
    state->endOfFile            = 0;

    /* Done; ready to start reading data */
    return state;
}

/**
 * \brief Frees up a lexer state
 */
void tp_free_lexer(tp_lexer_state state) {
    /* Sanity check */
    if (!state) {
        return;
    }

    /* Free the internal structures */
    free(state->lookahead);
    free(state->lookaheadSets);

    /* Free the state itself */
    free(state);
}

/**
 * \brief Maps a character to an internal symbol ID
 */
static int tp_lex_map_symbol(int character, const uint32_t* symbolMap) {
    /*  The symbol map has the same format documented in hard_coded_symbol_table
     *  (with a character type of wchar_t, ie, 16-bit characters):
     * 
     *      The table format is an array of integers, consisting of several layers
     *      of tables. Each layer represents how successively lower bytes of the
     *      symbol should be looked up: ie, the first layer represents how the upper
     *      8 bits of the symbol are translated, the second the next 8 bits and so
     *      on until the entire character has been translated.
     *     
     *      The format for all layers except the bottom layer is as follows:
     *          * 1 int  = default set for unknown characters
     *          * 1 int  = (lowest_used | (highest_used<<8))
     *          * n ints = -1 for the default set, or the offset of the table for the next layer, relative to this layer
     *      The bottom layer is the same, except that the values are the actual symbol set
     */
    int lowHigh;
    int lowest;
    int highest;
    int pos;
    int nextOffset;
    int result;

    /* Base case: negative characters are preserved intact, so tp_lex_eof always maps to tp_lex_eof */
    if (character < 0)      return character;

    /* We only accept 16-bit characters at the moment */
    if (character >= 65536) return tp_lex_invalid;

    /* First layer: fetch the symbol range */
    lowHigh = (int)symbolMap[1];
    lowest  = lowHigh&0xff;
    highest = lowHigh>>8;

    /* Get the initial offset */
    pos = (character>>8)&0xff;

    /* Use the default character if out of range */
    if (pos < lowest || pos >= highest) return (int)symbolMap[0];

    /* Fetch the offset of the secondary table */
    nextOffset = (int)symbolMap[2 + (pos - lowest)];

    /* Default symbol if the offset is -1 */
    if (nextOffset == -1) return (int)symbolMap[0];

    /* Second layer: fetch the symbol itself */
    lowHigh = (int)symbolMap[nextOffset + 1];
    lowest  = lowHigh&0xff;
    highest = lowHigh>>8;

    /* Get the final offset */
    pos = character&0xff;

    /* Default character if out of range */
    if (pos < lowest || pos >= highest) return (int)symbolMap[nextOffset + 0];

    /* Fetch the result if not */
    result = (int)symbolMap[nextOffset + 2 + (pos - lowest)];

    /* Default character if -1 */
    if (result == -1) return (int)symbolMap[nextOffset + 0];

    /* Otherwise, this is the result */
    return result;
}

/**
 * \brief Returns the next state for a particular symbol set in a particular state
 *
 * Assumes state is valid for the current state machine
 */
static int tp_lex_next_state(int symbolSet, int state, const uint32_t* stateMachine) {
    int offset;
    int nextState;
    int upper;
    int lower;

    /* Fetch the offset for this state */
    offset = (int) stateMachine[1 + state];

    /* Get the upper and lower bounds for this state */
    lower = offset;
    upper = (int) stateMachine[1 + state + 1];

    /* TODO: Do a binary search to find the symbol set */

    /* Result is -1 if we couldn't find any transition */
    return -1;
}

/**
 * \brief Matches a single symbol using the lexer
 *
 * \param state A lexer state
 *
 * \return tp_lex_nothing if no symbols are matched, otherwise the identifier of
 * the lexical symbol that was matched.
 *
 * At least one character is always removed from the source stream,
 * so it is safe to repeatedly call this even if an error results.
 */
int tp_lex_symbol(tp_lexer_state state) {
    tp_parser       parser;
    tp_read_symbol  readSymbol;
    void*           userData;
    int*            lookahead;
    int*            lookaheadSets;
    const uint32_t* symbolMap;

    /* Sanity check */
    if (!state) {
        return tp_lex_eof;
    }

    /* Fetch some data from the state */
    parser          = state->parser;
    symbolMap       = state->symbolMap;
    readSymbol      = parser->functions.readSymbol;
    userData        = parser->userData;
    lookahead       = state->lookahead;
    lookaheadSets   = state->lookaheadSets;

    /* Discard any existing lookahead */
    if (state->lookaheadPos > 0) {
        memmove(lookahead, lookahead + state->lookaheadPos, sizeof(int) * state->lookaheadPos);

        state->lookaheadEndPos -= state->lookaheadPos;
        state->lookaheadPos     = 0;
    }

    /* Iterate until we reach the end of the file */
    for (;;) {
        int nextSymbolSet;

        /* Fill in the lookahead */
        if (state->lookaheadEndPos == state->lookaheadPos) {
            /* Increase the amount of storage for the lookahead */
            if (state->lookaheadEndPos >= state->lookaheadSize) {
                state->lookaheadSize    *= 2;
                state->lookahead        = realloc(state->lookahead, sizeof(int) * state->lookaheadSize);
                state->lookaheadSets    = realloc(state->lookaheadSets, sizeof(int) * state->lookaheadSize);
                lookahead               = state->lookahead;
                lookaheadSets           = state->lookaheadSets;
            }

            /* Read the next character */
            if (!state->endOfFile) {
                int nextCharacter;

                nextCharacter                   = readSymbol(userData);
                lookahead[state->lookaheadPos]  = nextCharacter;
                if (nextCharacter == tp_lex_eof) {
                    state->endOfFile = tp_lex_eof;
                    lookaheadSets[state->lookaheadPos] = tp_lex_eof;
                } else {
                    /* Map the character we read */
                    lookaheadSets[state->lookaheadPos] = tp_lex_map_symbol(nextCharacter, symbolMap);
                }
            } else {
                /* Generate an infinite series of EOF characters here */
                lookahead[state->lookaheadPos] = tp_lex_eof;
                lookaheadSets[state->lookaheadPos] = tp_lex_eof;
            }

            /* Move the lookahead position on */
            state->lookaheadEndPos++;
        }

        /* Fetch the next symbol set */
        nextSymbolSet = lookaheadSets[state->lookaheadPos];

    }
}
