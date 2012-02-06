//
//  binary.h
//  TameParse
//
//  Created by Andrew Hunter on 06/02/2012.
//  Copyright (c) 2012 Andrew Hunter. All rights reserved.
//

#ifndef _COMPILER_OUTPUT_BINARY_H
#define _COMPILER_OUTPUT_BINARY_H

#include "TameParse/Compiler/output_stage.h"

namespace compiler {
    ///
    /// \brief Produces a binary file containing the parser and lexer tables
    ///
    /// The output file is designed to be used directly as the tables for a parser 
    /// implementation. This class supports both big and little-endian output.
    ///
    /// File format is as follows:
    ///
    /// All offsets are from file beginning; an offset of 0 indicates that a
    /// table is not present.
    ///
    /// Header (16 words):
    ///    1 word: file format/endian indicator 'TPar' (0x54506172)
    ///    1 word: file format version number (0x100)
    ///	   1 word: string ID containing the language name
    ///	   1 word: TameParse version (major<<16 | minor<<8 | revision<<0)
    ///    1 word: string ID containing the TameParse version information
    ///    11 words: reserved (written as 0xffffffff)
    ///
    /// Offset tables:
    ///    1 word: offset to strings table
    ///
    ///    1 word: offset to symbol map
    ///    1 word: offset to lexer state machine
    ///
    ///    1 word: offset to terminal action table
    ///    1 word: offset to nonterminal action table
    ///    1 word: offset to guard ending state table
    ///    1 word: offset to rule symbol count table
    ///    1 word: offset to weak to strong symbol table
    ///
    ///    1 word: offset to rule definition table
    ///    1 word: offset to terminal name table
    ///    1 word: offset to nonterminal name table
    ///
    class output_binary : public output_stage {
    private:
    };
}

#endif
