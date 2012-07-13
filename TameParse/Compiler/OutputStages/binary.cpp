//
//  binary.cpp
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

#include <cstdlib>
#include "TameParse/Compiler/OutputStages/binary.h"
#include "TameParse/Language/Formatter.h"
#include "TameParse/version.h"

using namespace std;
using namespace dfa;
using namespace contextfree;
using namespace tameparse;
using namespace language;
using namespace compiler;

/// \brief Creates a new output stage
output_binary::output_binary(console_container& console, const std::wstring& sourceFilename, lexer_stage* lexer, language_stage* language, lr_parser_stage* parser, const std::wstring& targetFile, bool bigEndian)
: output_stage(console, sourceFilename, lexer, language, parser)
, m_File(NULL)
, m_WritePos(0)
, m_FileLength(0)
, m_BigEndian(bigEndian) 
, m_TargetFilename(targetFile) {
}

/// \brief Destructor
output_binary::~output_binary() {
    // Free up the file if it exists
    if (m_File) {
        free(m_File);
        m_File = NULL;
    }
}

/// ==================
///  Raw file writing
/// ==================

/// \brief Retrieves a string associated with this binary file
int32_t output_binary::get_string(const std::wstring& value) {
    // Try to find the existing string
    map<wstring, int32_t>::iterator found = m_StringIdentifiers.find(value);

    // Use the existing ID if there is one
    if (found != m_StringIdentifiers.end()) {
        return found->second;
    }

    // Create a new ID
    int32_t newId = (int32_t) m_StringIdentifiers.size();
    m_StringIdentifiers[value] = newId;

    return newId;
}

/// \brief Writes out an integer value at the specified position
static inline void write_int_raw(uint32_t val, uint8_t*& file, size_t writePos, size_t& fileLength, bool bigEndian) {
    // Resize the file if necessary
    while (writePos + 4 > fileLength) {
        if (fileLength <= 0) {
            // Start at 16 bytes
            fileLength = 16;
        } else {
            // Double the amount of allocated memory every time we need to increase the size
            fileLength *= 2;
        }

        // Resize the file
        file = (uint8_t*) realloc(file, fileLength);
    }

    // Write out this value
    if (bigEndian) {
        file[writePos+0]    = (val>>24)&0xff;
        file[writePos+1]    = (val>>16)&0xff;
        file[writePos+2]    = (val>>8) &0xff;
        file[writePos+3]    = (val>>0) &0xff;
    } else {
        file[writePos+0]    = (val>>0) &0xff;
        file[writePos+1]    = (val>>8) &0xff;
        file[writePos+2]    = (val>>16)&0xff;
        file[writePos+3]    = (val>>24)&0xff;       
    }
}

/// \brief Retrieves a string associated with this binary file
int32_t output_binary::get_string(const string& value) {
    // Simple conversion to wide string (basically, assumes that the input string is ISO-8859-1)
    wstring wideString;
    for (string::const_iterator chr = value.begin(); chr != value.end(); ++chr) {
        wideString += (wchar_t) *chr;
    }

    // Return the entry in the string table for this value
    return get_string(wideString);
}

/// \brief Indicates that we're about to start writing out the table whose header offset is at the specified position
void output_binary::start_table(uint32_t offset) {
    // Write the format indicator before each table (so we can tell when an offset is incorrect)
    write_int(header::format_indicator);

    // Position within the actual file (4 * the offset)
    int pos = offset * 4;

    // Write out the current write pos at this position
    write_int_raw((uint32_t) m_WritePos, m_File, pos, m_FileLength, m_BigEndian);
}

/// \brief Writes out an unsigned integer value
void output_binary::write_int(uint32_t value) {
    write_int_raw(value, m_File, m_WritePos, m_FileLength, m_BigEndian);
    m_WritePos += 4;
}

/// \brief Writes out a signed intger value
void output_binary::write_int(int32_t value) {
    write_int_raw((uint32_t) value, m_File, m_WritePos, m_FileLength, m_BigEndian);
    m_WritePos += 4;
}

/// \brief Writes out a string value
void output_binary::write_string(const std::wstring& value) {
    // Start with the string length (in characters)
    write_int((uint32_t) value.size());

    // Convert the string into words and write it out
    for (size_t pos = 0; pos < value.size(); pos += 2) {
        wchar_t first   = value[pos];
        wchar_t second  = ((pos+1) < value.size()) ? value[pos+1] : 0;

        write_int((((uint32_t) first)<<16) | (uint32_t) second);
    }
}

/// =======
///  Lexer
/// =======

/// \brief Writes out the symbol map table
void output_binary::write_symbol_map() {
    // Build up a symbol table from the symbol sets generated by the parser
    symbol_table<wchar_t> symbolLevels;

    // Add the symbol range to our new set
    for (symbol_map_iterator symbolMap = begin_symbol_map(); symbolMap != end_symbol_map(); ++symbolMap) {
        symbolLevels.add_range(symbolMap->symbolRange, symbolMap->identifier);
    }

    // Convert to a hard-coded symbol table
    size_t  size    = 0;
    int*    table   = symbolLevels.table.to_hard_coded_table(size);

    // Write out the table
    start_table(table::symbol_map);
    write_int((uint32_t) size);

    for (size_t entry = 0; entry < size; ++entry) {
        write_int((int32_t) table[entry]);
    }
}

/// \brief Writes out the lexer DFA
void output_binary::write_lexer_dfa() {
    // Start the table
    start_table(table::lexer);

    // Write out the number of states
    uint32_t numStates = count_lexer_states();
    write_int(numStates);

    // Work out the offsets for each state
    uint32_t    curOffset   = m_WritePos + numStates * 4 + 4;
    const ndfa* dfa         = get_dfa();

    for (uint32_t stateId = 0; stateId < numStates; ++stateId) {
        // Get the current state
        const state& thisState = dfa->get_state(stateId);

        // For other states, the offset is in curOffset
        write_int(curOffset);

        // 4 bytes per transition
        curOffset += 4 * thisState.count_transitions();
    }

    // Write the final offset (so for a given state, the two offsets can be used to calculate the number of transitions)
    write_int(curOffset);

    // Write out the transitions themselves
    for (uint32_t stateId = 0; stateId < numStates; ++stateId) {
        // Get the current state
        const state& thisState = dfa->get_state(stateId);

        // Nothing to write if there are no transitions for this state
        if (thisState.count_transitions() == 0) {
            continue;
        }

        // Write out the transitions themselves
        bool shownError = false;

        for (state::iterator transit = thisState.begin(); transit != thisState.end(); ++transit) {
            // Get the symbol set and target state of this transition
            int symSet      = transit->symbol_set();
            int targetState = transit->new_state();

            // We only support 65535 symbol sets/65535 states
            // (Lexers that use more than this are going to be *HUGE* and will likely have hit other system limits before we get here)
            if (symSet > 65535 && !shownError) {
                cons().report_error(error(error::sev_error, filename(), L"OUT_OF_BOUNDS_SYMBOL_SET", L"Too many symbol sets", position()));
                shownError  = true;
                m_Errored   = true;
            }
            if (targetState > 65534) {
                cons().report_error(error(error::sev_error, filename(), L"OUT_OF_BOUNDS_TARGET_STATE", L"Too many lexer states", position()));
                shownError  = true;
                m_Errored   = true;
            }

            // Generate the entry
            uint32_t entry = ((uint32_t)symSet)<<16;
            if (targetState < 0) {
                entry |= 0xffffu;
            } else {
                entry |= (uint32_t)targetState;
            }

            // Write it out
            write_int(entry);
        }
    }
}

/// \brief Writes out the lexer 'accepting state' table
void output_binary::write_lexer_accept() {
    // Start the table
    start_table(table::lexer_accepting);

    // Iterate through the lexer state action table
    bool shownError = false;

    for (lexer_state_action_iterator act = begin_lexer_state_action(); act != end_lexer_state_action(); ++act) {
        if (!act->accepting) {
            // Non-accepting state
            write_int(0xffffffffu);
        } else {
            // Accepting state
            write_int(act->acceptSymbolId);
        }
    }
}

/// ========
///  Parser
/// ========

/// \brief Writes the terminal and nonterminal tables
void output_binary::write_action_tables() {
    // Fetch the parser tables built by the generator
    const lr::parser_tables& tables = get_parser_tables();
    typedef lr::parser_tables::action action;

    // Start the terminal actions table
    start_table(table::terminal_actions);

    // Write out the number of states
    write_int((uint32_t) tables.count_states());

    // Write out the pointers to the actions for each state
    uint32_t curPos = m_WritePos + tables.count_states()*4;
    for (int stateId = 0; stateId < tables.count_states(); ++stateId) {
        // Write out the current position
        write_int(curPos);

        // 8 bytes per action
        curPos += 8 * tables.action_counts()[stateId].numTerminals;
    }

    // Write out the final position
    write_int(curPos);

    // Write out the actions themselves
    for (int stateId = 0; stateId < tables.count_states(); ++stateId) {
        for (int actionId = 0; actionId < tables.action_counts()[stateId].numTerminals; ++actionId) {
            // Get this action
            const action& act = tables.terminal_actions()[stateId][actionId];

            // Write it out
            write_int((((uint32_t)act.type) << 24) | act.nextState);
            write_int((uint32_t)act.symbolId);
        }
    }

    // Start the nonterminal actions table
    start_table(table::nonterminal_actions);

    // Write out the number of states
    write_int((uint32_t) tables.count_states());

    // Write out the end of input and end of guard symbols
    write_int((uint32_t) tables.end_of_input());
    write_int((uint32_t) tables.end_of_guard());

    // Write out the pointers to the actions for each state
    curPos = m_WritePos;
    for (int stateId = 0; stateId < tables.count_states(); ++stateId) {
        // Write out the current position
        write_int(curPos);

        // 8 bytes per action
        curPos += 8 * tables.action_counts()[stateId].numNonterminals;
    }

    // Write out the final position
    write_int(curPos);

    // Write out the actions themselves
    for (int stateId = 0; stateId < tables.count_states(); ++stateId) {
        for (int actionId = 0; actionId < tables.action_counts()[stateId].numNonterminals; ++actionId) {
            // Get this action
            const action& act = tables.nonterminal_actions()[stateId][actionId];

            // Write it out
            write_int((((uint32_t)act.type) << 24) | act.nextState);
            write_int((uint32_t)act.symbolId);
        }
    }
}

/// \brief Writes the guard ending state table
void output_binary::write_guard_endings() {
    // Fetch the parser tables built by the generator
    const lr::parser_tables& tables = get_parser_tables();

    // Start the guard endings table
    start_table(table::guard_endings);

    // Write out the counts
    write_int((uint32_t) tables.count_end_of_guards());

    for (int guardId = 0; guardId < tables.count_end_of_guards(); ++guardId) {
        write_int((uint32_t) tables.end_of_guard_states()[guardId]);
    }
}

/// \brief Writes the rule count table
void output_binary::write_rule_counts() {
    // Fetch the parser tables built by the generator
    const lr::parser_tables& tables = get_parser_tables();

    // Start the table
    start_table(table::rule_symbol_counts);

    // Write the number of rules
    write_int((uint32_t) tables.count_reduce_rules());

    // Write the rules themselves
    for (int ruleId=0; ruleId < tables.count_reduce_rules(); ++ruleId) {
        const lr::parser_tables::reduce_rule& rule = tables.reduce_rules()[ruleId];

        // Each rule has a length and a nonterminal identifier
        write_int((((uint32_t) rule.identifier)<<16) | ((uint32_t) rule.length));

        // TODO: error if there are more than 65535 nonterminal identifiers or a rule with more than 65535 symbols
    }
}

/// \brief Writes the weak-to-strong symbol mapping table
void output_binary::write_weak_to_strong() {
    // Fetch the parser tables built by the generator
    const lr::parser_tables& tables = get_parser_tables();

    // Start the table
    start_table(table::weak_to_strong);

    // Write the number of weak-to-strong symbols in the table
    write_int((uint32_t) tables.count_weak_to_strong());

    // Write the symbols themselves
    for (int symbolId = 0; symbolId < tables.count_weak_to_strong(); ++symbolId) {
        // Write this item
        const lr::parser_tables::symbol_equivalent& equiv = tables.weak_to_strong()[symbolId];
        write_int((((uint32_t) equiv.m_OriginalSymbol)<<16) | ((uint32_t) equiv.m_MappedTo));

        // TODO: error if the symbols are out of range (can't happen?)
    }
}

/// =====================
///  Rule/grammar tables
/// =====================

/// \brief Writes out the names of the terminal symbols
void output_binary::write_terminal_names() {
    // Fetch the terminal dictionary
    const terminal_dictionary& terms = terminals();

    // Start the table
    start_table(table::terminal_names);

    // Terminal ID that we want to write out
    int curId = 0;

    // Iterate through the terminals
    for (terminal_dictionary::iterator term = terms.begin(); term != terms.end(); ++term) {
        // Ignore terminals that require us to go backwards (shouldn't be possible)
        if (term->first < curId) {
            continue;
        }

        // Write out blanks until we hit curId
        for (int blank = curId+1; blank < term->first; ++blank) {
            write_int(0xffffffffu);
        }
        curId = term->first;

        // Write out the string for this terminal
        write_int(get_string(term->second));
    }
}

/// \brief Writes out the names of the nonterminal symbols
void output_binary::write_nonterminal_names() {
    // Start the table
    start_table(table::nonterminal_names);

    // Fetch the grammar
    const grammar&              gram        = output_stage::gram();
    const terminal_dictionary&  terminals   = output_stage::terminals();

    // Start building up a map of nonterminal IDs to names
    map<int, wstring> ntNames;

    // Iterate through all of the rules in the grammar
    for (int ruleId = 0; ruleId < gram.max_rule_identifier(); ++ruleId) {
        // Fetch this rule
        const rule_container& rule = gram.rule_with_identifier(ruleId);

        // Get the nonterminal (item) ID
        int itemId = gram.identifier_for_item(rule->nonterminal());

        // See if we've already recorded a name for its nonterminal
        if (ntNames.find(itemId) == ntNames.end()) {
            // Generate the name for this item using the formatter
            ntNames[itemId] = formatter::to_string(*rule->nonterminal(), gram, terminals);
        }
    }

    // Write out the list of terminals
    int curId = 0; 

    for (map<int, wstring>::iterator nt = ntNames.begin(); nt != ntNames.end(); ++nt) {
        // Ignore items with identifiers that go downwards (shouldn't happen; generally indicates some items have negative IDs)
        if (nt->first < curId) {
            continue;
        }

        // Fetch the item that this represents
        const item_container& item = gram.item_with_identifier(nt->first);

        // Write out blanks until we reach the current ID
        for (int blank = curId+1; blank < nt->first; ++blank) {
            write_int(0xffffffffu);
            write_int(0xffffffffu);
        }
        curId = nt->first;

        // Write out the type of this nonterminal
        write_int(item->type());

        // Write out the identifier of the string for this nonterminal
        write_int(get_string(nt->second));
    }
}

/// \brief Writes out the definitions of the rules
void output_binary::write_rule_definitions() {
    // Start the table
    start_table(table::rule_definitions);

    // Fetch the grammar
    const grammar&              gram        = output_stage::gram();
    const terminal_dictionary&  terminals   = output_stage::terminals();

    // Count the number of rules
    int numRules = gram.max_rule_identifier();
    write_int(numRules);

    // Work out where the rules will start in the file
    int firstRuleOffset = m_WritePos + numRules * 4;
    int curRuleOffset   = firstRuleOffset;

    // Write out the positions for each rule
    for (int ruleId = 0; ruleId < numRules; ++ruleId) {
        // Each rule has the following format:
        //      nonterminal ID
        //      number of items
        //      n * item
        //          +ve value = terminal ID
        //          -ve value = (-1 - nonterminal ID)
        const rule_container& rule = gram.rule_with_identifier(ruleId);

        // 2 words header + n words items
        int ruleSize = 8 + 4*(int) rule->items().size();

        // Write where this rule will appear
        write_int(curRuleOffset);

        // Move the offset on to where the next rule will appear
        curRuleOffset += ruleSize;
    }

    // Write out the actual data for the rules
    for (int ruleId = 0; ruleId < numRules; ++ruleId) {
        // Fetch the rule
        const rule_container& rule = gram.rule_with_identifier(ruleId);

        // Get the nonterminal ID
        int ntId = gram.identifier_for_item(*rule->nonterminal());
        write_int(ntId);

        // Number of items in this rule
        write_int((int) rule->items().size());

        // The items themselves
        for (rule::iterator ruleItem = rule->begin(); ruleItem != rule->end(); ++ruleItem) {
            if ((*ruleItem)->type() == item::terminal) {
                // Terminal items get a positive ID
                write_int((*ruleItem)->symbol());
            } else {
                // Nonterminal items get a negative ID
                write_int(-1 - gram.identifier_for_item(*ruleItem));
            }
        }
    }
}

/// =========
///  Strings
/// =========

/// \brief Writes out the strings
///
/// Needs to be called after all the strings are generated
void output_binary::write_string_table() {
    // Start writing the string
    start_table(table::strings);

    // Create a sorted list of strings
    map<int32_t, wstring>   idToString;
    int32_t                 maxStringId = 0;

    for (map<wstring, int32_t>::const_iterator nextString = m_StringIdentifiers.begin(); nextString != m_StringIdentifiers.end(); ++nextString) {
        idToString[nextString->second] = nextString->first;

        if (nextString->second > maxStringId) {
            maxStringId = nextString->second;
        }
    }

    // Generate offsets for the strings
    write_int(maxStringId+1);
    uint32_t    curPos = m_WritePos + (maxStringId+1)*4;
    int         lastId = 0;

    for (map<int32_t, wstring>::const_iterator nextString = idToString.begin(); nextString != idToString.end(); ++nextString) {
        // Write out 0s until we get to this ID (shouldn't be necessary unless something else has gone wrong)
        while (lastId < nextString->first) {
            write_int(0u);
            ++lastId;
        }

        // Write out the position of this string
        write_int(curPos);

        // Add the number of bytes in this string
        curPos += 4 + 4 * ((nextString->second.size()+1)/2);
    }

    // Write out the strings themselves (which should appear at the offsets we calculated)
    for (map<int32_t, wstring>::const_iterator nextString = idToString.begin(); nextString != idToString.end(); ++nextString) {
        write_string(nextString->second);
    }
}

/// =============
///  Compilation
/// =============

/// \brief Compiles the parser specified by this stage
void output_binary::compile() {
    // Allocate space for the header and offsets table
    m_FileLength = m_WritePos = 0;

    // No errors yet
    m_Errored = false;

    // Write out the header
    write_int(header::format_indicator);
    write_int(header::format_version);
    write_int(get_string(language_name()));
    write_int((version::major_version<<16) | (version::minor_version<<8) | (version::revision<<0));
    write_int(get_string(version::version_string));
    for (int unsetHeader = 0; unsetHeader<11; ++unsetHeader) {
        write_int(0xffffffffu);
    }

    // Write out a blank offset table
    for (int offset = 0; offset < 16; ++offset) {
        write_int(0u);
    }

    // Write out the lexer
    write_symbol_map();
    write_lexer_dfa();
    write_lexer_accept();

    // Write out the parser
    write_action_tables();
    write_guard_endings();
    write_rule_counts();
    write_weak_to_strong();

    // Write out the names of things and the rule definition table
    write_terminal_names();
    write_nonterminal_names();
    write_rule_definitions();

    // Finally, write the string table
    write_string_table();

    // Write the final binary file
    if (!m_Errored) {
        std::ostream* outputFile = cons().open_binary_file_for_writing(m_TargetFilename + L".tpdat");
        if (outputFile) {
            outputFile->write((const char*) m_File, (streamsize) m_WritePos);

            outputFile->flush();
            delete outputFile;
        }
    }
}
