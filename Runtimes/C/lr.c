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
 *  \brief Finds the first action for a symbol in a state
 *
 *  \param parseData    A pointer to the start of the parse data (the point to which offsets are relative to)
 *  \param actionTable  A pointer to the action table to search (the first word after the header)
 *  \param stateId      The ID of the state to search
 *  \param symbolId     The symbol to search for
 *
 *  \return A pointer to the action, or NULL if the action does not exist
 */
static const int* tp_find_first_action(const int* parseData, const int* actionTable, int stateId, int symbolId) {
    int stateStartOffset;
    int stateEndOffset;
    int numEntries;

    int lower, upper;
    int pos;

    /* Fetch the offset for this state */
    stateStartOffset    = actionTable[stateId];
    stateEndOffset      = actionTable[stateId + 1];
    numEntries          = (stateEndOffset - stateStartOffset) / 2; /* Two words per entry */

    /* Result if NULL if there are no actions in this state */
    if (numEntries <= 0) return NULL;

    /* Perform a binary search for this symbol */
    lower = 0;
    upper = numEntries - 1;

    while (upper >= lower) {
        int middle;
        int middleSymbol;

        /* Fetch the symbol to compare (it's the second word in the entry, see binary.h for details) */
        middle          = (lower + upper) >> 1;
        middleSymbol    = parseData[stateStartOffset + (middle*2) + 1];

        if (middleSymbol < symbolId)    lower = middle + 1;
        else                            upper = middle - 1;
    }

    /* Now upper will be first with a value < symbol */
    pos = upper + 1;

    /* Result is NULL if there is no action for this symbol */
    if (parseData[stateStartOffset + (pos*2) + 1] != symbolId) return NULL;

    /* Got the offset */
    return parseData + stateStartOffset + (pos*2);
}

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
 * \brief Attempts to shift the current lookahead symbol on to the given parser state
 *
 * \param state The state to update
 *
 * \return 0 if the symbol could not be shifted (the state is left unchanged if this occurs)
 */
int tp_state_shift(tp_parser_state state) {

}

/**
 * \brief Updates a state by performing a particular action
 *
 * \param state         The state to update
 * \param actionType    The type of action to perform
 * \param nextState     The new state ID after the action (or the rule ID for reduction/guard actions)
 * 
 * \return              1 if the lookahead should be moved on to the next symbol
 */
int tp_perform_action(tp_parser_state state, int actionType, int nextState) {

}

/**
 * \brief Determines if a particular action is possible in the specified state
 *
 * This can be used with guard actions or weak reductions to check if the action
 * should be taken or if the next action should be performed. The action is
 * checked in the context of the current parser lookahead. This can be used to
 * aid error recovery (as it can check if a new state is valid).
 *
 * \param state         The state to update
 * \param actionType    The type of action to perform
 * \param nextState     The new state ID after the action (or the rule ID for reduction/guard actions)
 */
int tp_can_perform(tp_parser_state state, int actionType, int nextState) {
    
}
