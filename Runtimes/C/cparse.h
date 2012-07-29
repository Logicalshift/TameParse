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

#ifndef _TAMEPARSE_CPARSE_H
#define _TAMEPARSE_CPARSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

/** \file cparse.h
 *
 * Definitions for the C implementation of a TameParse parser.
 */

/* = Type definitions = */

/**
 * \brief Lexical symbol returned when no match is made
 */
static const int tp_lex_nothing = -1;

/**
 * \brief Lexical symbol returned when the end of file is reached
 */
static const int tp_lex_eof = -2;

/**
 * \brief Lexical symbol representing an invalid character
 */
static const int tp_lex_invalid = -3;

/**
 * \brief User-defined function that reads the next symbol from the stream
 *
 * The return value is an integer indicating the symbol ID (usually a UTF-16
 * unicode character)
 *
 * Symbols less than 0 indicate various error conditions. In particular,
 * tp_lex_eof is used to symbolise the end of the input stream.
 *
 * \param userData User data associated with the parser that this is being called for
 */
typedef int (*tp_read_symbol)(void* userData);

/**
 * \brief User-defined function that frees the data associated with an AST node
 *
 * \param astData Data for the specified AST node, as allocated by tp_shift or tp_reduce
 * \param userData User data associated with the parser that this is being called for
 *
 * AST nodes are allocated by tp_shift or tp_reduce functions, and are freed
 * when the parser that generated them is destroyed.
 */
typedef void (*tp_free)(void* astData, void* userData);

/**
 * \brief User-defined function that returns the AST node resulting from shifting a lexical symbol
 *
 * \param terminalId The ID of the terminal that is being shifted
 * \param symbols An array containing the symbols read using the tp_read_symbol function
 * \param numSymbols The number of symbols in the symbols array
 * \param userData User data associated with the parser that this is being called for
 *
 * \return A new AST node object, or NULL
 */
typedef void* (*tp_shift)(int terminalId, int* symbols, int numSymbols, void* userData);

/**
 * \brief User-defined function that returns the result of reducing a series of AST nodes
 *
 * \param nonterminalId The ID of the nonterminal that is being reduced
 * \param numSymbols The number of symbols that are being reduced
 * \param astData The AST nodes being reduced by this action (an array containing at least numSymbols entries)
 * \param ruleId The ID of the rule that is being reduced
 * \param userData User data associated with the parser that this is being called for
 *
 * \return A new AST node object, or NULL.
 *
 * After this call has completed, any nodes in astData that are not NULL are
 * to the tp_free function. This call can modify the contents of astData to
 * preserve these nodes, for example if they are preserved as part of the new
 * node.
 */
typedef void* (*tp_reduce)(int nonterminalId, int numSymbols, void** astData, int ruleId, void* userData);

/**
 * \brief Functions used to run a parser
 */
typedef struct tp_parser_functions {
    /**
     * \brief Size of this structure
     *
     * This must be equal to sizeof(tp_parser_functions). This is used to
     * maintain binary compatibility if the library version is changed and
     * the code for this file is not recompiled.
     */
    size_t size;

    /**
     * \brief Function to read the next symbol from a stream
     */
    tp_read_symbol readSymbol;

    /**
     * \brief Function that frees an AST node
     */
    tp_free freeAstNode;

    /**
     * \brief Generates an AST node resulting from a shift action
     */
    tp_shift shift;

    /**
     * \brief Generates an AST node resulting from a reduce action
     */
    tp_reduce reduce;
} tp_parser_functions;

/**
 * \brief Parser definition
 */
typedef struct tp_parser_def* tp_parser;

/**
 * \brief The state of a runner lexer
 */
typedef struct tp_lexer_state_def* tp_lexer_state;

/**
 * \brief The state of a running parser
 */
typedef struct tp_parser_state_def* tp_parser_state;

/**
 * \brief Binary data defining a parser
 *
 * This is a binary file generated by the TameParse parser generator, in the
 * system endianness.
 */
typedef const void* tp_parser_data;

/* = Function definitions = */

/**
 * \brief Creates a new parser
 *
 * \param data The parser definition data (which must be generated for the native system endianess)
 * \param functions The parser action functions
 * \param userData The user data to pass to the action functions
 *
 * \return NULL if the parser could not be created, otherwise a new tp_parser structure
 */
tp_parser tp_create_parser(tp_parser_data data, tp_parser_functions* functions);

/**
 * \brief Frees an existing parser
 */
void tp_free_parser(tp_parser oldParser);

/* = The lexer = */

/**
 * \brief Creates a new lexer in the initial state
 *
 * \param parser The parser where the lexer is defined
 * \param userData The user data to pass in to the parser functions
 *
 * \return The final state of the parser
 */
tp_lexer_state tp_create_lexer(tp_parser parser, void* userData);

/**
 * \brief Frees up a lexer state
 */
void tp_free_lexer(tp_lexer_state state);

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
int tp_lex_symbol(tp_lexer_state state);

/* = The parser = */

/**
 * \brief Runs the parser and returns the final state
 *
 * \param parser The parser to run
 * \param initialStateId The initial 
 * \param userData The user data to pass in to the parser functions
 *
 * \return The final state of the parser
 */
tp_parser_state tp_parse(tp_parser parser, int initialStateId, void* userData);

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
tp_parser_state tp_create_initial_state(tp_parser parser, int stateId, void* userData);

/**
 * \brief Creates a new parser state by copying an existing state
 */
tp_parser_state tp_copy_state(tp_parser_state oldState);

/**
 * \brief Frees a parser state
 */
void tp_free_state(tp_parser_state oldState);

/**
 * \brief Returns 0 if the parser is not in an accepting state, or 1 if it is
 */
int tp_state_accepting(tp_parser_state state);

/**
 * \brief Returns 0 if the parser is not in an error state, or 1 if it is
 */
int tp_state_error(tp_parser_state state);

/**
 * \brief Returns the ID of the current location in the state machine
 *
 * \param state The parser state to retrieve the ID for
 */
int tp_state_get_current(tp_parser_state state);

/**
 * \brief Retrieves the stack depth in the specified state
 */
int tp_state_get_stack_depth(tp_parser_state state);

/**
 * \brief Creates a new state by popping entries from the stack
 */
tp_parser_state tp_state_peek(tp_parser_state state, int depth);

/**
 * \brief Attempts to shift the specified symbol on to the given parser state
 */
void tp_state_shift(tp_parser_state state, int terminalId, const int* symbols, int numSymbols);

/* = The lookahead queue = */

/**
 * \brief Inserts a lookahead item into the specified state (before any other lookahead)
 */
void tp_lookahead_push(tp_parser_state state, int terminalId, const int* symbols, int numSymbols);

/**
 * \brief Throws away the lookahead pointed to by the specified state
 */
void tp_lookahead_pop(tp_parser_state state);

/**
 * \brief Retrieves the terminal at the current lookahead position
 */
int tp_lookahead_terminal(tp_parser_state state);

/**
 * \brief Retrieves the symbols at the current lookahead position
 *
 * This returns the number of symbols and updates result so that it points
 * at the symbols themselves.
 */
int tp_lookahead_symbols(tp_parser_state state, const int** result);

#ifdef __cplusplus
};
#endif

#endif
