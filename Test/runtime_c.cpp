//
//  runtime_c.cpp
//  Parse
//
//  Created by Andrew Hunter on 13/07/2012.
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

#include <iostream>
#include <fstream>

#include "runtime_c.h"

#include "Runtimes/C/cparse.h"

using namespace std;

/// Simple lexer test: two identifiers
static const char* identifierTest = "identifier1 identifier2";

/// Runtime data structure
struct runtime_data {
    runtime_data(const char* string)
    : inputStream((const unsigned char*) string)
    , length(strlen(string))
    , pos(0) { }

    const unsigned char*    inputStream;
    int                     length;
    int                     pos;
};

static int read_symbol(void* data) {
    runtime_data* rtData = (runtime_data*) data;

    // EOF if we're over the end
    if (rtData->pos >= rtData->length) return tp_lex_eof;

    return (int) rtData->inputStream[rtData->pos++];
}

static void free_ast(void* astData, void* userData) {

}

static void* shift(int terminalId, int* symbols, int numSymbols, void* userData) {
    return NULL;
}

static void* reduce(int nonterminalId, int numSymbols, void** astData, int ruleId, void* userData) {
    return NULL;
}

void test_runtime_c::run_tests() {
    // Read in the ansic.tpdat file (which is expected in whichever directory we're running in)
    ifstream ansiStream;

    ansiStream.open("ansic.tpdat", ios::binary);
    ansiStream.seekg(0, ios::end);
    int length = ansiStream.tellg();
    ansiStream.seekg(0, ios::beg);

    char* ansiBinary = new char[length];
    ansiStream.read(ansiBinary, length);
    ansiStream.close();

    // Create the functions structure
    tp_parser_functions functions;

    functions.size          = sizeof(tp_parser_functions);
    functions.readSymbol    = read_symbol;
    functions.freeAstNode   = free_ast;
    functions.shift         = shift;
    functions.reduce        = reduce;

    // Try to create a parser from the stream
    tp_parser cParser = tp_create_parser((tp_parser_data*) ansiBinary, &functions, (void*) new runtime_data(identifierTest));

    report("CreateParser", cParser != NULL);
    if (!cParser) return;

    // Finish up
    delete[] ansiBinary;
}
