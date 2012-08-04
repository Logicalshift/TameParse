/*
 *  cparse.h
 *  TameParse
 *
 *  Created by Andrew Hunter on 20/05/2012.
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

#ifndef _TAMEPARSE_CPARSE_INTERNAL_H
#define _TAMEPARSE_CPARSE_INTERNAL_H

#include "cparse.h"

/**
 * \brief The internal representation of a parser
 *
 * User programs should not rely on this structure, as it's subject to change
 * even between minor updates of the runtime.
 */
struct tp_parser_def {
    /** The user-defined functions used by this parser */
    tp_parser_functions functions;

    /** The data that defines this parser */
    const uint32_t* data;
};

/**
 * \brief An entry in the lookahead queue
 */
struct tp_lookahead_entry {
    /** The number of states that are pointing at this entry */
    int usageCount;

    /** The terminal ID of the token this entry represents */
    int terminalId;

    /** The number of symbols that were read from the input stream for this item */
    int numSymbols;

    /** A copy of the input symbols read in for this entry */
    int* symbols;

    /** \brief The next item in the lookahead queue */
    struct tp_lookahead_entry* next;

    /** 
     * \brief The previous item(s) in the lookahead queue 
     *
     * A state can free a lookahead entry if usageCount is 0 and there is no
     * previous item (if there is a previous item or the usage count is
     * non zero, then another state must have an eventual reference to this
     * entry)
     */
    struct tp_lookahead_entry** prev;

    /**
     * \brief The number of previous items
     */
    int numPrev;
};

/**
 * \brief The internal represention of a parser stack entry
 */
struct tp_parser_entry {
    /** The number of states that point at this entry, or 0 or -1 if this entry is marked (referenced from another entry) */
    int usageCount;

    /** -1 if this item is on top of the stack, or the index of the entry 'above' it */
    int previousEntry;

    /** The ID of the state represented by this entry */
    int state;

    /** The user-supplied AST data for this entry */
    void* ast;
};

/** 
 * \brief The internal representation of a parser stack
 *
 * The stack uses a garbage-collection algorithm to remove states rather than
 * being a simple push/pop stack. This makes it possible to have multiple
 * states for the same input simultaneously (this is initially useful for
 * evaluating guards, but can also be used for performing error recovery or 
 * implementing generalised parsing) 
 */
struct tp_parser_stack {
    /** The parser object that this stack is for */
    tp_parser parser;

    /** The number of parser states that are using this stack */
    int numStates;

    /** The number of entries in this stack */
    int numEntries;

    /** An array of stack entries */
    struct tp_parser_entry* entries;

    /** The next free entry */
    int nextFree;

    /** The value of usageCount for a marked, in-use, entry (0 or -1) */
    int markValue;
};

/**
 * \brief The internal representation of a parser state
 */
struct tp_parser_state_def {
    /** The parser object that this state is for */
    tp_parser parser;

    /** The tokenizer for this parser */
    tp_read_token tokenizer;

    /** The user data for the tokenizer */
    void* tokenizerData;

    /** The stack that this state is a part of */
    struct tp_parser_stack* stack;

    /** The current lookahead in this state */
    struct tp_lookahead_entry* lookahead;

    /** The ID of the stack entry that this state refers to */
    int entry;
};

/**
 * \brief Creates a new lookahead entry by reading from the lexer
 */
struct tp_lookahead_entry* tp_lookahead_load_from_lexer(tp_parser_state state);

#endif
