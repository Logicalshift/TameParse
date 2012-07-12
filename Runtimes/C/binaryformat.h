/*
 *  binaryformat.h
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

#ifndef _TAME_BINARYFORMAT_H
#define _TAME_BINARYFORMAT_H

/*
 * See binary.h for a full description of the file format
 */

/** \brief The format indicator word value (in the system endianness) */
#define TP_FORMATINDICATOR          0x54506172u

/** \brief The file format version ID described by this header */
#define TP_CURRENTFORMATVERSION     0x100

/** \brief Offset to the format/endian indicator, in 32-bit words */
#define TPH_FORMAT                  0

/** \brief Offset to the file format version number */
#define TPH_FILEFORMATVERSION       1

/** \brief Offset to the language name string ID */
#define TPH_LANGUAGENAME            2

/** \brief Offset to the TameParse version number */
#define TPH_TAMEPARSEVERSION        3

/** \brief Offset to the descripted version string ID */
#define TPH_TAMEPARSEVERSIONSTRING  4

/** \brief Offset of the first offset*/
#define TPH_TABLEOFFSETS            16

/** \brief Offset of the strings table */
#define TPH_STRINGOFFSET            TPH_TABLEOFFSETS + 0

/** \brief Offset of the lexer symbol map */
#define TPH_LEXER_SYMBOLMAP         TPH_TABLEOFFSETS + 1

/** \brief Offset of the lexer state machine */
#define TPH_LEXER_STATEMACHINE      TPH_TABLEOFFSETS + 2

/** \brief Offset of the lexer accepting states table */
#define TPH_LEXER_ACCEPTINGSTATES   TPH_TABLEOFFSETS + 3

/** \brief Offset to the LR terminal actions table */
#define TPH_LR_TERMINALACTIONS      TPH_TABLEOFFSETS + 4

/** \brief Offset to the LR nonterminal actions table */
#define TPH_LR_NONTERMINALACTIONS   TPH_TABLEOFFSETS + 5

/** \brief Offset to the LR guard ending state table */
#define TPH_LR_GUARDENDSTATE        TPH_TABLEOFFSETS + 6

/** \brief Offset to the rule symbol count table */
#define TPH_LR_RULESYMBOLCOUNT      TPH_TABLEOFFSETS + 7

/** \brief Offset to the weak-to-strong symbol table */
#define TPH_LR_WEAKTOSTRONG         TPH_TABLEOFFSETS + 8

/** \brief Offset to the rule definitions table */
#define TPH_INFO_RULEDEFINITIONS    TPH_TABLEOFFSETS + 9

/** \brief Offset to the terminal names table */
#define TPH_INFO_TERMINALNAMES      TPH_TABLEOFFSETS + 10

/** \brioef Offset to the nonterminal names table */
#define TPH_INFO_NONTERMINALNAMES   TPH_TABLEOFFSETS + 11

#endif
