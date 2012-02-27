//
//  binary.h
//  TameParse
//
//  Created by Andrew Hunter on 06/02/2012.
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

#ifndef _COMPILER_OUTPUT_BINARY_H
#define _COMPILER_OUTPUT_BINARY_H

#include <string>
#include <map>

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
    /// All offsets are in words (32-bits, 4 bytes) from file beginning; an offset of
    /// 0 indicates that a table is not present.
    ///
    /// Header (16 words):
    ///    1 word: file format/endian indicator 'TPar' (0x54506172)
    ///    1 word: file format version number (0x100)
    ///    1 word: string ID containing the language name
    ///    1 word: TameParse version (major<<16 | minor<<8 | revision<<0)
    ///    1 word: string ID containing the TameParse version information
    ///    11 words: reserved (written as 0xffffffff)
    ///
    /// Offset tables:
    ///    1 word: offset to strings table
    ///
    ///    1 word: offset to symbol map
    ///    1 word: offset to lexer state machine
    ///    1 word: offset to lexer accepting state table
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
    /// The symbol map has the same format documented in hard_coded_symbol_table
    /// (with a character type of wchar_t, ie, 16-bit characters):
    ///
    ///     The table format is an array of integers, consisting of several layers
    ///     of tables. Each layer represents how successively lower bytes of the
    ///     symbol should be looked up: ie, the first layer represents how the upper
    ///     8 bits of the symbol are translated, the second the next 8 bits and so
    ///     on until the entire character has been translated.
    ///    
    ///     The format for all layers except the bottom layer is as follows:
    ///         * 1 int  = default set for unknown characters
    ///         * 1 int  = (lowest_used | (highest_used<<8))
    ///         * n ints = -1 for the default set, or the offset of the table for the next layer, relative to this layer
    ///     The bottom layer is the same, except that the values are the actual symbol set
    ///
    /// The lexer state machine has the following format:
    ///
    ///     1 word:   number of states
    ///     n words:  table giving the offset to the transition table for each state
    ///                 offsets of 0 indicate a state with no transitions
    ///     1 word:   the offset to the word after the last transition
    ///     n words:  transition table for each state, in the following format:
    ///                 n words: words for each transition: ((symbol<<16) | state), ordered by symbol
    ///
    /// There is no support for lexers with > 65536 states or > 65536 distinct symbols
    /// in this version of the file format. The number of transitions in a given state
    /// can be established by subtracting the offset for that state from the offset
    /// for the next state (the transitions are ordered by state)
    ///
    /// The lexer accepting table consists of one entry per state, which is 0xffffffff
    /// for rejecting states, or otherwise the symbol ID of the terminal that should be
    /// generated.
    ///
    /// The terminal and nonterminal tables have the same basic format and different
    /// headers. This common format is as follows:
    ///
    ///     n words:  table giving the offset to the action table for each state
    ///     n words:  the actions for each state, 2 words per action, sorted by symbol ID:
    ///         1 word: (action_type<<24) | next_state
    ///         1 word: symbol ID for this action
    ///
    /// Action types are the same as in lr_action.h. Reduce actions supply the rule
    /// ID instead of the next state.
    ///
    /// The terminal table has the following header preceding the action table:
    ///
    ///     1 word:   number of states
    ///
    /// The nonterminal table has the following header preceding the action table:
    ///
    ///     1 word:   number of states
    ///     1 word:   ID of the 'end of input' ('$') nonterminal symbol
    ///     1 word:   ID of the 'end of guard' ('%') nonterminal symbol
    ///
    /// (Note that 'end of input' and 'end of guard' both count as nonterminal
    /// symbols; these can have actions other than goto)
    ///
    class output_binary : public output_stage {
    public:
        ///
        /// \brief Offsets and other definitions for the binary file header
        ///
        class header {
        public:
            /// \brief File format indicator
            static const uint32_t format_indicator = 0x54506172;

            /// \brief File format version number
            static const uint32_t format_version = 0x100;

            /// \brief Total number of words in the header
            static const uint32_t length = 16;

            /// \brief Offset to the file indicator word
            static const uint32_t indicator = 0;

            /// \brief Header word containin the format_version constant
            static const uint32_t version = 1;

            /// \brief Header word containing the string ID of the name of the language
            static const uint32_t name = 2;

            /// \brief Header word containing the version number of TameParse that this was generated by
            static const uint32_t generator_version = 3;

            /// \brief Header word containing a string ID identifying the generator for this file
            static const uint32_t generator_version_string = 4;
        };

        ///
        /// \brief Locations for the table offsets (after the header)
        ///
        class table {
        public:
            /// \brief Offset to the strings table
            static const uint32_t strings = header::length + 0;

            /// \brief Offset to the symbol map table
            static const uint32_t symbol_map = header::length + 1;

            /// \brief Offset to the lexer state machine definition
            static const uint32_t lexer = header::length + 2;

            /// \brief Offset to the lexer state machine definition
            static const uint32_t lexer_accepting = header::length + 3;

            /// \brief Offset to the terminal actions table
            static const uint32_t terminal_actions = header::length + 4;

            /// \brief Offset to the nonterminal actions table
            static const uint32_t nonterminal_actions = header::length + 5;

            /// \brief Offset to the guard ending state table
            static const uint32_t guard_endings = header::length + 6;

            /// \brief Offset to the rule symbol counts table
            static const uint32_t rule_symbol_counts = header::length + 7;

            /// \brief Offset to the weak-to-strong symbol mapping table
            static const uint32_t weak_to_strong = header::length + 8;

            /// \brief Offset to the rule definitions table
            static const uint32_t rule_definitions = header::length + 9;

            /// \brief Offset to the terminal names table
            static const uint32_t terminal_names = header::length + 10;

            /// \brief Offset to the nonterminal names table
            static const uint32_t nonterminal_names = header::length + 11;

            /// \brief Offset to the end of the table offsets (first place we can write 'real' data to)
            static const uint32_t table_offset_end = header::length + 16;
        };

    private:
        /// \brief True if the binary file should be written out as big-endian,
        /// false if it should be little endian
        bool m_BigEndian;

        /// \brief True if there has been an error and the file should not be written out
        bool m_Errored;

        /// \brief The binary file being generated by this object
        uint8_t* m_File;

        /// \brief Current write position in the file
        size_t m_WritePos;

        /// \brief The total number of bytes allocated in m_File
        size_t m_FileLength;

        /// \brief Maps strings to their identifiers
        std::map<std::wstring, int32_t> m_StringIdentifiers;

        /// \brief The name of the file to write
        std::wstring m_TargetFilename;

    private:
        output_binary(const output_binary& copyFrom);
        output_binary& operator=(const output_binary& copyFrom);

    private:
        /// \brief Retrieves a string associated with this binary file
        int32_t get_string(const std::wstring& value);

        /// \brief Retrieves a string associated with this binary file
        int32_t get_string(const std::string& value);

        /// \brief Indicates that we're about to start writing out the table whose header offset is at the specified position
        void start_table(uint32_t offset);

        /// \brief Writes out an unsigned integer value
        void write_int(uint32_t value);

        /// \brief Writes out a signed intger value
        void write_int(int32_t value);

        /// \brief Writes out a string value
        void write_string(const std::wstring& value);

    private:
        /// \brief Writes out the symbol map table
        void write_symbol_map();

        /// \brief Writes out the lexer DFA
        void write_lexer_dfa();

        /// \brief Writes out the lexer 'accepting state' table
        void write_lexer_accept();

    private:
        /// \brief Writes the terminal and nonterminal tables
        void write_action_tables();

        /// \brief Writes the guard ending state table
        void write_guard_endings();

        /// \brief Writes the rule count table
        void write_rule_counts();

        /// \brief Writes the weak-to-strong symbol mapping table
        void write_weak_to_strong();

    private:
        /// \brief Writes out the strings
        ///
        /// Needs to be called after all the strings are generated
        void write_string_table();

    public:
        /// \brief Creates a new output stage
        output_binary(console_container& console, const std::wstring& sourceFilename, lexer_stage* lexer, language_stage* language, lr_parser_stage* parser, const std::wstring& targetFilename, bool bigEndian = false);

        /// \brief Destructor
        virtual ~output_binary();

        /// \brief Compiles the parser specified by this stage
        virtual void compile();
    };
}

#endif
