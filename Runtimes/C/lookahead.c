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

#include <stdlib.h>

#include "cparse.h"
#include "cparse_internal.h"

/**
 * \brief Inserts a lookahead item into the specified state (before any other lookahead)
 */
void tp_lookahead_push(tp_parser_state state, int terminalId, const int* symbols, int numSymbols) {
    struct tp_lookahead_entry*  newLookahead;
    int                         pos;

    /* Sanity check */
    if (!state)     return;
    if (!symbols)   numSymbols = 0;

    /* Allocate space */
    newLookahead = malloc(sizeof(struct tp_lookahead_entry));

    /* Set up this lookahead */
    newLookahead->usageCount    = 1;                                    /* The current state will be using this lookahead */
    newLookahead->terminalId    = terminalId;                           /* Terminal ID */
    newLookahead->numSymbols    = numSymbols;                           /* Number of symbols in this lookahead */
    newLookahead->symbols       = malloc(sizeof(int)*(numSymbols+1));   /* Allocate space for the symbols */
    newLookahead->next          = state->lookahead;                     /* Following lookaehad is whatever is current for this state */
    newLookahead->prev          = NULL;                                 /* No previous yet */
    newLookahead->numPrev       = 0;                                    /* No previous yet */

    /* Copy the symbols */
    for (pos=0; pos<numSymbols; ++pos) 
        newLookahead->symbols[pos] = symbols[pos];
    newLookahead->symbols[pos] = 0;

    /* Set up the previous of the old 'current' lookahead */
    if (state->lookahead) {                                             /* Should always be true (lexer will get out of sync if this is ever false) */
        /* Allocate space */
        ++state->lookahead->numPrev;
        state->lookahead->prev = realloc(state->lookahead->prev, sizeof(struct tp_lookahead_entry*) * state->lookahead->numPrev);

        state->lookahead->prev[state->lookahead->numPrev-1] = newLookahead;

        /* No longer current */
        --state->lookahead->usageCount;
    }

    /* This becomes the current lookahead */
    state->lookahead = newLookahead;
}

/**
 * \brief Creates a new lookahead entry by reading from the lexer
 */
struct tp_lookahead_entry* tp_lookahead_load_from_lexer(tp_parser_state state) {

}

/**
 * \brief Throws away the lookahead pointed to by the specified state
 */
void tp_lookahead_pop(tp_parser_state state) {
    struct tp_lookahead_entry* oldLookahead;

    /* Sanity check */
    if (!state)             return;
    if (!state->lookahead)  return;

    /* We do nothing if we've reached the end of file */
    if (state->lookahead->terminalId == tp_lex_eof) return;

    /* Current lookahead is no longer 'used' */
    oldLookahead = state->lookahead;
    --oldLookahead->usageCount;

    /* Read from the lexer if this the last lookahead item */
    if (!state->lookahead->next) {
        struct tp_lookahead_entry* newEntry;

        /* Read the next entry */
        newEntry = tp_lookahead_load_from_lexer(state);

        if (oldLookahead->numPrev > 0 || oldLookahead->usageCount > 0) {
            /* Make the current state lookahead the 'previous' of the next entry */
            ++newEntry->numPrev;
            newEntry->prev = realloc(newEntry->prev, sizeof(struct tp_lookahead_entry*) * newEntry->numPrev);

            newEntry->prev[newEntry->numPrev-1] = oldLookahead;
            oldLookahead->next                  = newEntry;
        }
    }

    /* Next lookahead becomes the current lookahead for this state */
    state->lookahead = oldLookahead->next;
    if (state->lookahead) {
        ++state->lookahead->usageCount;
    }

    /* Free the old lookahead if nothing can use it any more */
    if (oldLookahead->usageCount <= 0 && oldLookahead->numPrev > 0) {
        /* Nothing is using the old lookahead, and it's about to become inaccessible */
        int pos;

        /* Remove from the previous list of the next lookahead */
        if (oldLookahead->next) {
            struct tp_lookahead_entry* next;

            /* Get the next item */
            next = oldLookahead->next;

            /* Search for items referring to the old lookahead */
            for (pos = 0; pos < next->numPrev; ++pos) {
                if (next->prev[pos] == oldLookahead) {
                    /* Remove this entry */
                    --next->numPrev;
                    if (pos < next->numPrev) {
                        memmove(next->prev + pos, next->prev + pos + 1, sizeof(struct tp_lookahead_entry*) * next->numPrev);
                    }
                    --pos;
                }
            }
        }

        /* Free the contents of this item */
        if (oldLookahead->symbols)  free(oldLookahead->symbols);
        if (oldLookahead->prev)     free(oldLookahead->prev);
        free(oldLookahead);
    }
}

/**
 * \brief Retrieves the terminal at the current lookahead position
 */
int tp_lookahead_terminal(tp_parser_state state) {
    /* Sanity check */
    if (!state)             return tp_lex_nothing;
    if (!state->lookahead)  return tp_lex_nothing;

    /* Return lookahead terminal */
    return state->lookahead->terminalId;
}

/**
 * \brief Retrieves the symbols at the current lookahead position
 *
 * This returns the number of symbols and updates result so that it points
 * at the symbols themselves.
 */
int tp_lookahead_symbols(tp_parser_state state, const int** result) {
    /* Sanity check */
    if (!result)            return 0;
    *result = NULL;

    if (!state)             return 0;
    if (!state->lookahead)  return 0;

    /* Return the symbols in the current lookahead */
    *result = state->lookahead->symbols;
    return state->lookahead->numSymbols;
}
