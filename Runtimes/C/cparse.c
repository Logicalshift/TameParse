/*
 *  cparse.c
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

#include <stdlib.h>

#include "cparse.h"
#include "cparse_internal.h"

/**
 * \brief Creates a new parser
 *
 * \param data The parser definition data (which must be generated for the native system endianess)
 * \param functions The parser action functions
 *
 * \return NULL if the parser could not be created, otherwise a new tp_parser structure
 */
tp_parser tp_create_parser(tp_parser_data data, tp_parser_functions* functions, void* userData) {
    tp_parser result;

    /* Sanity check */
    if (!data)      return NULL;
    if (!functions) return NULL;

    if (functions->size != sizeof(tp_parser_functions)) return NULL;
    if (!functions->readSymbol)                         return NULL;
    if (!functions->freeAstNode)                        return NULL;
    if (!functions->shift)                              return NULL;
    if (!functions->reduce)                             return NULL;

    /* Allocate the structure */
    result = malloc(sizeof(struct tp_parser));

    result->functions   = *functions;
    result->data        = (const uint32_t*) data;
    result->userData    = userData;

    return result;
}

/**
 * \brief Frees an existing parser
 */
void tp_free_parser(tp_parser oldParser) {
    /* Sanity check */
    if (!oldParser) return;

    /* Free the structure */
    free(oldParser);
}
