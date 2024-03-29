//
//  cplusplus.cpp
//  Parse
//
//  Created by Andrew Hunter on 29/08/2011.
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

#include <time.h>
#include <sstream>
#include <algorithm>
#include <locale>

#include "TameParse/Compiler/OutputStages/cplusplus.h"

using namespace std;
using namespace dfa;
using namespace contextfree;
using namespace compiler;

/// \brief A suffix used to distinguish node types from the variables that reference them
static const string s_TypeSuffix = "_n";

/// \brief A suffix used for classes that represent the content of repeating EBNF items (classes for the * or + closures)
static const string s_ContentSuffix = "_content";

/// \brief Creates a new output stage
output_cplusplus::output_cplusplus(console_container& console, const std::wstring& filename, lexer_stage* lexer, language_stage* language, lr_parser_stage* parser, const std::wstring& filenamePrefix, const std::wstring& className, const std::wstring& namespaceName)
: output_stage(console, filename, lexer, language, parser)
, m_FilenamePrefix(filenamePrefix)
, m_ClassName(className)
, m_Namespace(namespaceName)
, m_SourceFile(NULL)
, m_HeaderFile(NULL) {
}

/// \brief Destructor
output_cplusplus::~output_cplusplus() {
    if (m_SourceFile)               delete m_SourceFile;
    if (m_HeaderFile)               delete m_HeaderFile;
}

/// \brief The current locale
static std::locale loc;

/// \brief Converts a string to upper case
static string toupper(const string& s) {
    string res = s;
    for (string::iterator c = res.begin(); c != res.end(); ++c) {
        *c = std::toupper(*c, loc);
    }
    return res;
}


//              ===================
//               String converters
//              ===================

/// \brief Returns a valid C++ identifier for the specified symbol name
std::string output_cplusplus::get_identifier(const std::wstring& name, bool allowReserved) {
    // Empty string if the name is empty
    if (name.size() == 0) return "_";

    // Strip out any quotes that the name might have
    wstring stripped;

    if (name[0] == L'\'' && name[name.size()-1] == L'\'') {
        stripped = name.substr(1, name.size()-2);
    }

    else if (name[0] == L'"' && name[name.size()-1] == L'"') {
        stripped = name.substr(1, name.size()-2);
    }

    else if (name[0] == L'<' && name[name.size()-1] == L'>') {
        stripped = name.substr(1, name.size()-2);
    }

    else {
        stripped = name;
    }

    stringstream res;
    res << "yy_";
    for (wstring::const_iterator wideChar = stripped.begin(); wideChar != stripped.end(); ++wideChar) {
        // Just append values that are valid C++ identifiers
        wchar_t c = *wideChar;
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (res.tellp() > 0 && c >= '0' && c <= '9')) {
            res << (char) c;
            continue;
        }

        // Some punctuation characters are written out as names
        switch (c) {
            case L'<':  res << "_lessthan_"; break;
            case L'>':  res << "_greaterthan_"; break;
            case L'\'': res << "_quote_"; break;
            case L'"':  res << "_doublequote_"; break;
            case L'&':  res << "_ampersand_"; break;
            case L':':  res << "_colon_"; break;
            case L'=':  res << "_equals_"; break;
            case L'!':  res << "_exclamation_"; break;
            case L'@':  res << "_at_"; break;
            case L'#':  res << "_hash_"; break;
            case L'$':  res << "_dollar_"; break;
            case L'%':  res << "_percent_"; break;
            case L'^':  res << "_cicumflex_"; break;
            case L'*':  res << "_star_"; break;
            case L'(':  res << "_openparen_"; break;
            case L')':  res << "_closeparen_"; break;
            case L'+':  res << "_plus_"; break;
            case L'/':  res << "_slash_"; break;
            case L'\\': res << "_backslash_"; break;
            case L'|':  res << "_pipe_"; break;
            case L';':  res << "_semicolon_"; break;
            case L'.':  res << "_dot_"; break;
            case L',':  res << "_comma_"; break;
            case L'?':  res << "_question_"; break;
            case L'~':  res << "_tilde_"; break;
            case L'_':  res << "_"; break;
            case L'{':  res << "_opencurly_"; break;
            case L'}':  res << "_closecurly_"; break;
            case L'[':  res << "_opensquare_"; break;
            case L']':  res << "_closesquare_"; break;

            case L'-':  
                // Bit weird, the language encourages users to use this as part of identifiers
                if (stripped.size() == 1) {
                    res << "_minus_"; 
                } else {
                    res << "_";
                }
                break;

            default:
                // We use '_' as a placeholder for everything else
                res << "_";
                break;
        }
    }
    
    // Produce the final result
    return res.str();
}

/// \brief Retrieves or assigns a class name for an item with the specified ID
std::string output_cplusplus::class_name_for_item(const contextfree::item_container& item) {
    // Get the item identifier from the grammar
    int identifier = gram().identifier_for_item(item);

    // Try to find an existing name for this nonterminal
    map<int, string>::const_iterator found = m_ClassNameForItem.find(identifier);
    if (found != m_ClassNameForItem.end()) {
        // Use the found definition if it exists
        return found->second;
    }

    // Get the base name for this item
    string baseName = get_identifier(name_for_item(item), true);
    if (baseName.size() == 0) baseName = "unknown";

    // Find a unique name that we can assign to this item
    int variant = 0;

    for (;;) {
        // Get the next variant
        ++variant;

        // Generate the variant name
        stringstream varNameStream;
        varNameStream << baseName;
        if (variant > 1) {
            varNameStream << "_" << variant;
        }

        string varName = varNameStream.str();

        // See if this name has already been used or is reserved
        if (m_UsedClassNames.find(varName) == m_UsedClassNames.end()) {
            // This is the name we shall use

            // Set as the name for this nonterminal, and mark as used
            m_ClassNameForItem[identifier] = varName;
            m_UsedClassNames.insert(varName);

            // Return as the result
            return varName;
        }

        // Keep trying until we find a valid name
    }
}

/// \brief Returns true if the specified name should be considered 'valid'
bool output_cplusplus::name_is_valid(const std::wstring& name) {
    // Empty names are invalid
    if (name.empty()) return false;

    // All other names are valid
    return true;
}

//              ================
//               General output
//              ================

/// \brief Writes out a header to the specified file
void output_cplusplus::write_header(const std::wstring& filename, std::ostream* target) {
    // Get the current time
    char        timeString[128];
    time_t      nowTime     = time(NULL);
    struct tm*  nowLocal    = localtime(&nowTime);
    
    strftime(timeString, 127, "%a %Y/%m/%d %H:%M:%S", nowLocal);
    
    (*target) << "///\n";
    (*target) << "/// " << cons().convert_filename(filename) << "\n";
    (*target) << "/// Parser file generated by TameParse at " << timeString << "\n";
    (*target) << "///\n\n";
}

/// \brief About to begin writing out output
void output_cplusplus::begin_output() {
    // Clean up if there are existing files for any reasom
    if (m_SourceFile) delete m_SourceFile;
    if (m_HeaderFile) delete m_HeaderFile;

    m_SourceFile = NULL;
    m_HeaderFile = NULL;

    // Create the source files
    wstring sourceFilename = m_FilenamePrefix + L".cpp";
    wstring headerFilename = m_FilenamePrefix + L".h";

    // Open the files for writing
    m_SourceFile = cons().open_binary_file_for_writing(sourceFilename);
    m_HeaderFile = cons().open_binary_file_for_writing(headerFilename);

    // TODO: check for errors (file streams will be NULL)

    // Write out a header
    write_header(sourceFilename, m_SourceFile);
    write_header(headerFilename, m_HeaderFile);

    // Write out the boilerplate at the start of the header (declare the class)
    *m_HeaderFile << "#ifndef TAMEPARSE_PARSER_" << toupper(get_identifier(m_FilenamePrefix, true)) << "\n";
    *m_HeaderFile << "#define TAMEPARSE_PARSER_" << toupper(get_identifier(m_FilenamePrefix, true)) << "\n";
    *m_HeaderFile << "\n";

    *m_HeaderFile << "#include \"TameParse/Util/syntax_ptr.h\"\n";
    *m_HeaderFile << "#include \"TameParse/Dfa/lexer.h\"\n";
    *m_HeaderFile << "#include \"TameParse/Lr/parser.h\"\n";
    *m_HeaderFile << "#include \"TameParse/Lr/parser_tables.h\"\n";
    *m_HeaderFile << "\n";
    
    if (!m_Namespace.empty()) {
        *m_HeaderFile << "namespace " << get_identifier(m_Namespace, false) << " {\n";
    }
    *m_HeaderFile << "class " << get_identifier(m_ClassName, false) << " {\n";

    // Add to the list of used class names
    m_UsedClassNames.insert(get_identifier(m_ClassName, false));
    
    // Write out the boilerplate at the start of the source file (include the header)
    *m_SourceFile << "#include \"" << cons().convert_filename(headerFilename) << "\"\n";

    if (!m_Namespace.empty()) {
        *m_SourceFile << "using namespace " << get_identifier(m_Namespace, false) << ";\n";
    }
}

/// \brief Finishing writing out output
void output_cplusplus::end_output() {
    // Finish off the header file
    *m_HeaderFile << "\n";
    *m_HeaderFile << "};\n";            // End of class
    if (!m_Namespace.empty()) {
        *m_HeaderFile << "}\n";         // End of namespace
    }

    *m_HeaderFile << "\n#endif\n";    // End of the conditional
}


//              =========
//               Symbols
//              =========

/// \brief Writes out the terminal symbols definitions to the header file
void output_cplusplus::header_terminal_symbols() {
    // Create a public class to contain the list of terminal identifiers
    *m_HeaderFile << "\npublic:\n";
    *m_HeaderFile << "    class t {\n";
    *m_HeaderFile << "    public:\n";

    // Add to the list of used class names
    m_UsedClassNames.insert("t");
        
    /// \brief Number of times a terminal symbol with a particular name has been used
    map<string, int> terminalSymbolCount;
    
    // Output the terminal symbols
    for (terminal_symbol_iterator term = begin_terminal_symbol(); term != end_terminal_symbol(); ++term) {
        // Get the short name
        string       shortName = get_identifier(term->name, false);
        
        // Choose a unique name (gets a bit weird if we chose a unique name that later clashes)
        stringstream ourName;
        
        ourName << shortName;
        int count = terminalSymbolCount[shortName];
        
        while (terminalSymbolCount[ourName.str()] > 0) {
            ourName << "_" << count;
        }

        ++terminalSymbolCount[shortName];

        // Output a constant for this terminal
        *m_HeaderFile << "        static const int " << ourName.str() << " = " << term->identifier << ";\n";
    }

    // Done
    *m_HeaderFile << "    };\n";
}

/// \brief Writes out definitions for the nonterminal symbols to the header file
void output_cplusplus::header_nonterminal_symbols() {
    // Create a public class to contain the list of nonterminal identifiers
    *m_HeaderFile << "\npublic:\n";
    *m_HeaderFile << "    class nt {\n";
    *m_HeaderFile << "    public:\n";

    // Add to the list of used class names
    m_UsedClassNames.insert("nt");
        
    // Number of times a nonterminal symbol with a particular name has been used
    map<string, int> nonterminalSymbolCount;

    // Write out the nonterminal definitions
    for (nonterminal_symbol_iterator nonterm = begin_nonterminal_symbol(); nonterm != end_nonterminal_symbol(); ++nonterm) {
        // Only deal with the named nonterminals (ignore nonterminals generated for EBNF items, etc)
        if (nonterm->item->type() != item::nonterminal) continue;

        // Get the short name
        string       shortName = get_identifier(nonterm->name, false);
        
        if (nonterm->name.empty()) {
            // Some nonterminals don't have a name
            shortName = "_unnamed";
        }
        
        // Choose a unique name (gets a bit weird if we chose a unique name that later clashes)
        stringstream ourName;
        
        ourName << shortName;
        int count = nonterminalSymbolCount[shortName];
        
        while (nonterminalSymbolCount[ourName.str()] > 0) {
            ourName << "_" << count;
        }
        
        ++nonterminalSymbolCount[shortName];
        
        // Output a constant for this terminal
        *m_HeaderFile << "        static const int " << ourName.str() << " = " << nonterm->identifier << ";\n";
    }

    // Done
    *m_HeaderFile << "    };\n";
}

/// \brief Defines the symbols associated with this language
void output_cplusplus::define_symbols() {
    header_terminal_symbols();
    header_nonterminal_symbols();
}

//              ==================
//               Lexer generation
//              ==================

/// \brief Writes the symbol map definitions to the header
void output_cplusplus::header_symbol_map() {
    // Add the name of the symbols we'll define to the list of used class names
    m_UsedClassNames.insert("number_of_symbol_sets");

    // Write out the number of symbol sets to the header file
    *m_HeaderFile << "\npublic:\n";
    *m_HeaderFile << "    static const int number_of_symbol_sets = " << count_lexer_symbol_sets() << ";\n";
}

/// \brief Writes the symbol map definitions to the source file
void output_cplusplus::source_symbol_map() {
    // Include the hard-coded symbol table in the source file
    *m_SourceFile << "\n#include \"TameParse/Dfa/hard_coded_symbol_table.h\"\n";

    // Build up a symbol table from the symbol sets
    symbol_table<wchar_t> symbolLevels;

    // Iterate through the symbol ranges
    for (symbol_map_iterator symbolMap = begin_symbol_map(); symbolMap != end_symbol_map(); ++symbolMap) {
        symbolLevels.add_range(symbolMap->symbolRange, symbolMap->identifier);
    }

    // Convert to a hard coded table and write out
    *m_SourceFile << "\nstatic const int s_SymbolMapTable[] = {";

    // Convert to a hard-coded table
    size_t  size;
    int*    hcst = symbolLevels.table.to_hard_coded_table(size);

    // Write it out
    for (size_t tablePos = 0; tablePos < size; ++tablePos) {
        // Add newlines
        if ((tablePos % 10) == 0) {
            *m_SourceFile << "\n        ";
        }

        // Write out this entry
        *m_SourceFile << dec << hcst[tablePos];
        if (tablePos+1 < size) {
            *m_SourceFile << ", ";
        }
    }

    // Finished with the table
    delete[] hcst;

    // Finished the table
    *m_SourceFile << "\n    };\n";
    
    // Add the symbol table class
    *m_SourceFile << "\nstatic const dfa::hard_coded_symbol_table<wchar_t, 2> s_SymbolMap(s_SymbolMapTable);\n";
}

/// \brief Writes out the header items for the lexer state machine
void output_cplusplus::header_lexer_state_machine() {
    // Write out the number of states to the header file
    *m_HeaderFile << "\n        static const int number_of_lexer_states = " << count_lexer_states() << ";\n";
    *m_HeaderFile << "\npublic:\n";
    *m_HeaderFile << "    static const dfa::lexer lexer;\n";

    // Add to the list of used class names
    m_UsedClassNames.insert("number_of_lexer_states");
    m_UsedClassNames.insert("lexer");
}

/// \brief Writes out the source code for the lexer state machine
void output_cplusplus::source_lexer_state_machine() {
    // Need to include the state machine class
    *m_SourceFile << "\n#include \"TameParse/Dfa/state_machine.h\"\n";

    // Begin writing out the state machine table
    // TODO: support table styles other than 'compact' (the flat table is faster for all character types and more compact for some lexer types)
    *m_SourceFile << "\nstatic const dfa::state_machine_compact_table<false>::entry s_LexerStateMachine[] = {\n";

    // Set the current position
    int entryPos = 0;

    // Entry offset for each state
    vector<int> stateToEntryOffset;
    stateToEntryOffset.push_back(entryPos);

    // Current state ID (initially -1)
    int stateId = -1;

    // Iterate through all of the transitions
    for (lexer_state_transition_iterator transit = begin_lexer_state_transition(); transit != end_lexer_state_transition(); ++transit) {
        // Deal with changes to the current state
        if (transit->stateIdentifier != stateId) {
            // Update the current state ID
            stateId = transit->stateIdentifier;

            // Write out entry offsets
            while (stateId >= (int)stateToEntryOffset.size()) {
                stateToEntryOffset.push_back(entryPos);
            }

            // Write out comments
            *m_SourceFile << "\n\n        // State " << stateId << "\n        ";
        }

        // Write out some formatting
        if (entryPos > 0) {
            *m_SourceFile << ", ";
            if ((entryPos%10) == 0) {
                *m_SourceFile << "\n        ";
            }
        }

        // Write out this transition
        *m_SourceFile << "{ " << transit->symbolSet << ", " << transit->newState << " }";

        // Update the lexer state position
        ++entryPos;
    }

    // Finish off the table
    *m_SourceFile << "\n    };\n";

    // Add a final state to point to the end of the array
    stateToEntryOffset.push_back(entryPos);

    // Write out the rows table
    *m_SourceFile << "\nstatic const dfa::state_machine_compact_table<false>::entry* s_LexerStates[" << stateToEntryOffset.size()-1 << "] = {\n        ";

    // Write the actual rows
    bool first = true;
    for (vector<int>::iterator offset = stateToEntryOffset.begin(); offset != stateToEntryOffset.end()-1; ++offset) {
        // Commas between entries
        if (!first) *m_SourceFile << ", ";

        // Entries point to a position in the state machine table
        *m_SourceFile << "s_LexerStateMachine + " << *offset;

        // No longer the first entry
        first = false;
    }

    // Finish off the table
    *m_SourceFile << "\n    };\n";

    // Write out the table of state actions
    *m_SourceFile << "\nstatic const int s_AcceptingStates[] = {\n        ";

    // Iterate through the action table
    for (lexer_state_action_iterator act = begin_lexer_state_action(); act != end_lexer_state_action(); ++act) {
        // Separator
        if (act->stateId > 0) {
            *m_SourceFile << ", ";
        }

        if (act->accepting) {
            // Write out the action for this state
            *m_SourceFile << act->acceptSymbolId;
        } else {
            // Non-accepting states get -1 as the action
            *m_SourceFile << "-1";
        }
    }

    // Finish up the acceptance table
    *m_SourceFile << "\n    };\n";

    // Create a state machine
    *m_SourceFile << "\ntypedef dfa::state_machine_tables<wchar_t, dfa::hard_coded_symbol_table<wchar_t, 2> > lexer_state_machine;\n";
    *m_SourceFile << "static const lexer_state_machine s_StateMachine(s_SymbolMap, s_LexerStates, " << stateToEntryOffset.size()-1 << ");\n";

    // Create the lexer itself
    *m_SourceFile << "\ntypedef dfa::dfa_lexer_base<const lexer_state_machine&, 0, 0, false, const lexer_state_machine&> lexer_definition;\n";
    *m_SourceFile << "static lexer_definition s_LexerDefinition(s_StateMachine, " << stateToEntryOffset.size()-1 << ", s_AcceptingStates);\n";

    // Finally, the lexer class itself
    *m_SourceFile << "\nconst dfa::lexer " << get_identifier(m_ClassName, false) << "::lexer(&s_LexerDefinition, false);\n";
}

/// \brief Defines the symbols associated with this language
void output_cplusplus::define_lexer_tables() {
    header_symbol_map();
    header_lexer_state_machine();

    source_symbol_map();
    source_lexer_state_machine();
}

//              ===================
//               Parser generation
//              ===================

/// \brief Defines the parser tables for this language
void output_cplusplus::define_parser_tables() {
    header_parser_tables();
    source_parser_tables();
}

/// \brief Type of an action
typedef lr::parser_tables::action action;

/// \brief Writes out an action table
template<class get_count> void write_action_table(string tableName, const lr::parser_tables::action* const* actionTable, const lr::parser_tables& tables, ostream& output) {
    // Count getter object
    get_count gc;

    // Start the table
    output << "static lr::parser_tables::action " << tableName << "_data[] = {";

    // Iterate through the states
    bool first = true;
    int count = 0;
    for (int state = 0; state < tables.count_states(); ++state) {
        // Add the actions for this state
        int numActions = gc(tables, state);
        
        bool showingState = true;
        output << "\n\n    // State " << state << "\n    ";

        // Write out each action for this state
        for (int actionId = 0; actionId < numActions; ++actionId) {
            // Comma if this is not the first item
            if (!first) {
                output << ", ";
            }

            // Add newlines for formatting
            if ((count%5) == 0 && !showingState) {
                output << "\n    ";
            }

            // Write out this action
            const action& thisAction = actionTable[state][actionId];
            output << "{ " << thisAction.type << ", " << thisAction.nextState << ", " << thisAction.symbolId << " }";

            // Move on
            first = false;
            showingState = false;
            ++count;
        }
    }
    output << "\n};\n";
    
    // Output the final table
    output << "static lr::parser_tables::action* " << tableName << "[] = {";

    int pos = 0;
    count   = 0;
    first   = true;
    
    for (int state = 0; state < tables.count_states(); ++state) {
        // Comma if this is not the first item
        if (!first) {
            output << ", ";
        }
        
        // Add newlines for formatting
        if ((count%3) == 0) {
            output << "\n    ";
        }

        // Output this state
        output << tableName << "_data + " << pos;
        
        // Add the actions for this state
        int numActions = gc(tables, state);
        pos += numActions;
        
        // Move on
        ++count;
        first = false;
    }
    
    output << "\n};\n";
}

/// \brief Functor that returns the number of actions for a given terminal object
class count_terminal_actions {
public:
    int operator()(const lr::parser_tables& tables, int state) const {
        return tables.action_counts()[state].numTerminals;
    }   
};

/// \brief Functor that returns the number of actions for a given non-terminal object
class count_nonterminal_actions {
public:
    int operator()(const lr::parser_tables& tables, int state) const {
        return tables.action_counts()[state].numNonterminals;
    }   
};

/// \brief Writes out the header items for the parser tables
void output_cplusplus::header_parser_tables() {
    *m_HeaderFile   << "\n"
                << "public:\n"
                << "    static const lr::parser_tables lr_tables;\n";
}

/// \brief Writes out the source code for the parser tables
void output_cplusplus::source_parser_tables() {
    // Fetch the parser tables built by the generator
    const lr::parser_tables& tables = get_parser_tables();

    // True if we're at the first item
    bool    first;
    
    // Count used when writing tables
    int     count;
    
    // Need to include the parser tables file
    *m_SourceFile << "\n#include \"TameParse/Lr/parser_tables.h\"\n";
    
    // Write out the terminal actions
    *m_SourceFile << "\n";
    write_action_table<count_terminal_actions>("s_TerminalActions", tables.terminal_actions(), tables, *m_SourceFile);
    
    // ... and the nonterminal actions
    *m_SourceFile << "\n";
    write_action_table<count_nonterminal_actions>("s_NonterminalActions", tables.nonterminal_actions(), tables, *m_SourceFile);
    
    // Write out the action counts
    *m_SourceFile << "\nstatic lr::parser_tables::action_count s_ActionCounts[] = {";
    
    first   = true;
    count   = 0;
    for (int stateId=0; stateId < tables.count_states(); ++stateId) {
        // Comma
        if (!first) {
            *m_SourceFile << ", ";
        }

        // Newline
        if ((count%5) == 0) {
            *m_SourceFile << "\n    ";
        }
        
        // Write out the next item
        *m_SourceFile << "{ " << tables.action_counts()[stateId].numTerminals << ", " << tables.action_counts()[stateId].numNonterminals << " }";
        
        // Move on
        first = false;
        ++count;
    }
    
    *m_SourceFile << "\n};\n";
    
    // Write out the end guard states
    *m_SourceFile << "\nstatic int s_EndGuardStates[] = {";
    
    first   = true;
    count   = 0;
    for (int stateId=0; stateId < tables.count_end_of_guards(); ++stateId) {
        // Comma
        if (!first) {
            *m_SourceFile << ", ";
        }
        
        // Newline
        if ((count%20) == 0) {
            *m_SourceFile << "\n    ";
        }
        
        // Write out the next item
        *m_SourceFile << tables.end_of_guard_states()[stateId];
        
        // Move on
        first = false;
        ++count;
    }
    
    *m_SourceFile << "\n};\n";

    // Write out the reduce rules
    *m_SourceFile << "\nstatic lr::parser_tables::reduce_rule s_ReduceRules[] = {";
    
    first   = true;
    count   = 0;
    for (int ruleId=0; ruleId < tables.count_reduce_rules(); ++ruleId) {
        // Comma
        if (!first) {
            *m_SourceFile << ", ";
        }
        
        // Newline
        if ((count%5) == 0) {
            *m_SourceFile << "\n    ";
        }
        
        // Write out the next item
        const lr::parser_tables::reduce_rule& rule = tables.reduce_rules()[ruleId];
        *m_SourceFile << "{ " << rule.identifier << ", " << rule.ruleId << ", " << rule.length << " }";
        
        // Move on
        first = false;
        ++count;
    }
    
    *m_SourceFile << "\n};\n";
    
    // Finally, the weak to strong equivalence table
    *m_SourceFile << "\nstatic lr::parser_tables::symbol_equivalent s_WeakToStrong[] = {";
    
    first   = true;
    count   = 0;
    for (int stateId=0; stateId < tables.count_weak_to_strong(); ++stateId) {
        // Comma
        if (!first) {
            *m_SourceFile << ", ";
        }
        
        // Newline
        if ((count%10) == 0) {
            *m_SourceFile << "\n    ";
        }
        
        // Write out the next item
        *m_SourceFile << "{ " << tables.weak_to_strong()[stateId].m_OriginalSymbol << ", " << tables.weak_to_strong()[stateId].m_MappedTo << " }";
        
        // Move on
        first = false;
        ++count;
    }
    
    *m_SourceFile << "\n};\n";
    
    // Generate the parser tables
    *m_SourceFile   << "\nconst lr::parser_tables " << get_identifier(m_ClassName, false) << "::lr_tables(" 
                    << tables.count_states() << ", " << tables.end_of_input() << ", " 
                    << tables.end_of_guard() 
                    << ", s_TerminalActions, s_NonterminalActions, s_ActionCounts, s_EndGuardStates, " 
                    << tables.count_end_of_guards() << ", " << tables.count_reduce_rules() << ", "
                    << "s_ReduceRules, " << tables.count_weak_to_strong() << ", "
                    << "s_WeakToStrong"
                    << ");\n";

    // Add to the list of used class names
    m_UsedClassNames.insert("lr_tables");
}

//              ================
//               AST generation
//              ================

/// \brief Writes out the AST tables
void output_cplusplus::define_ast_tables() {
    header_ast_forward_declarations();
    header_ast_class_declarations();
    header_parser_actions();

    source_ast_class_definitions();
    source_ast_class_constructors();
    source_ast_position_functions();
    source_shift_actions();
    source_reduce_actions();

    // Output the parser definition
    *m_HeaderFile   << "\npublic:\n"
                    << "    typedef util::syntax_ptr<syntax_node> syntax_node_container;\n"
                    << "    typedef lr::parser<syntax_node_container, parser_actions> ast_parser_type;\n"
                    << "    static const ast_parser_type ast_parser;\n";
    
    *m_SourceFile   << "\nconst " << get_identifier(m_ClassName, false) << "::ast_parser_type " << get_identifier(m_ClassName, false) << "::ast_parser(&lr_tables, false);\n";

    // Generate functions for creating new parsers
    header_start_symbols();
}

/// \brief Writes out inline functions to generate initial parser states for specific start symbols
void output_cplusplus::header_start_symbols() {
    // Begin writing out the definitions
    *m_HeaderFile   << "\npublic:\n"
                    << "    typedef ast_parser_type::state state;\n";

    // Fetch the start symbols
    const vector<wstring>& startSymbols = get_start_symbols();

    // Write out functions for generating the initial state for each start symbol
    int initialState = 0;
    for (vector<wstring>::const_iterator startSymbol = startSymbols.begin(); startSymbol != startSymbols.end(); ++startSymbol) {
        // Assign a name to this symbol
        // TODO: avoid clashes with defined symbols called 'create_X'
        string startName = get_identifier(*startSymbol, true);

        // Define a function 
        *m_HeaderFile   << "    inline static state* create_" << startName << "(parser_actions* actions) {\n"
                        << "        return ast_parser.create_parser(actions, " << initialState << ");\n"
                        << "    }\n"
                        << "\n"
                        << "    inline static state* create_" << startName << "(dfa::lexeme_stream* stream, bool deleteStream = false) {\n"
                        << "        return ast_parser.create_parser(new parser_actions(stream, deleteStream), " << initialState << ");\n"
                        << "    }\n"
                        << "\n"
                        << "    template<typename char_type, typename traits> inline static state* create_" << startName << "(std::basic_istream<char_type, traits>& input) {\n"
                        << "        return create_" << startName << "(lexer.create_stream_from<char_type, traits>(input), true);\n"
                        << "    }\n"
                        << "\n"
                        << "    template<typename char_type, typename custom_stream_alike> inline static state* create_" << startName << "(custom_stream_alike& input) {\n"
                        << "        return create_" << startName << "(lexer.create_stream_from<char_type, custom_stream_alike>(input), true);\n"
                        << "    }\n";

        // Move the initial state on
        initialState++;
    }
}

/// \brief Writes out the forward declarations for the classes that represent items in the grammar
void output_cplusplus::header_ast_forward_declarations() {
    // Write out the AST syntax base class
    *m_HeaderFile   << "\n"
                    << "public:\n"
                    << "    class syntax_node {\n"
                    << "    public:\n"
                    << "        virtual ~syntax_node();\n"
                    << "        virtual std::wstring to_string();\n"
                    << "        virtual dfa::position pos() const;\n"
                    << "        virtual dfa::position final_pos() const;\n"
                    << "    };\n";

    // Create a class that will represent a terminal item
    *m_HeaderFile   << "\n"
                    << "    class terminal : public syntax_node {\n"
                    << "    private:\n"
                    << "        dfa::lexeme_container m_Lexeme;\n"
                    << "\n"
                    << "    public:\n"
                    << "        terminal(const dfa::lexeme_container& lexeme);\n"
                    << "\n"
                    << "        inline const dfa::lexeme_container& get_lexeme() const { return m_Lexeme; }\n"
                    << "\n"
                    << "        template<class symbol_type> inline std::basic_string<symbol_type> content() const { return m_Lexeme->content<symbol_type>(); }\n"
                    << "\n"
                    << "        virtual dfa::position pos() const;\n"
                    << "\n"
                    << "        virtual dfa::position final_pos() const;\n"
                    << "    };\n";
    
    // Iterate through the terminals
    for (terminal_symbol_iterator term = begin_terminal_symbol(); term != end_terminal_symbol(); ++term) {
        // Get the name for this terminal
        string name = class_name_for_item(term->item);

        // Turn into a type name
        name += s_TypeSuffix;

        // Write out a forward declaration for this item
        *m_HeaderFile << "\n    class " << name << ";\n";
    }
    
    // Iterate through the nonterminals
    for (nonterminal_symbol_iterator nonterm = begin_nonterminal_symbol(); nonterm != end_nonterminal_symbol(); ++nonterm) {
        // Get the name for this nonterminal
        string ntName = class_name_for_item(nonterm->item);

        // Turn into a type name
        ntName += s_TypeSuffix;

        // Repeating items have a content and a vector node
        if (nonterm->item->type() == item::repeat || nonterm->item->type() == item::repeat_zero_or_one) {
            *m_HeaderFile   << "\n    class " << ntName << ";\n";
        }

        // Guards have no definitions written for them, but all other item kinds just have a field for each entry in a rule
        else if (nonterm->item->type() != item::guard) {
            // Write out a forward declaration for this item
            *m_HeaderFile << "\n    class " << ntName << ";\n";
        }
    }
}

/// \brief Writes out the class declarations for the classes that represent AST items
void output_cplusplus::header_ast_class_declarations() {
    // Write out the definitions for the terminals
    for (terminal_symbol_iterator term = begin_terminal_symbol(); term != end_terminal_symbol(); ++term) {
        // Get the name for this terminal
        string name = class_name_for_item(term->item);

        // Turn into a type name
        name += s_TypeSuffix;

        // Declare a class for this item
        *m_HeaderFile   << "\n    // " << get_identifier(terminals().name_for_symbol(term->identifier), true) << "\n"
                        << "    class " << name << " : public terminal {\n"
                        << "    public:\n"
                        << "        " << name << "(const dfa::lexeme_container& lex) : terminal(lex) { }\n"
                        << "    };\n";
    }

    // Iterate through the nonterminals
    for (nonterminal_symbol_iterator nonterm = begin_nonterminal_symbol(); nonterm != end_nonterminal_symbol(); ++nonterm) {
        // Guards have no definitions written out for them
        if (nonterm->item->type() == item::guard) {
            continue;
        }

        // Get the name for this nonterminal
        string ntName = class_name_for_item(nonterm->item);

        // Append the type suffix
        ntName += s_TypeSuffix;

        // The class name for the content class for this item
        string ntContentClass = ntName;

        // Begin a class declaration for this item
        bool repeatingItem = false;

        if (nonterm->item->type() == item::repeat || nonterm->item->type() == item::repeat_zero_or_one) {
            repeatingItem = true;
            ntContentClass += s_ContentSuffix;
            *m_HeaderFile << "\n    class " << ntContentClass << " : public syntax_node {\n";       
        } else {
            // Write out a forward declaration for this item
            *m_HeaderFile << "\n    class " << ntContentClass << " : public syntax_node {\n";
        }

        // Write out the pos/final_pos definitions
        if (nonterm->item->type() != item::guard) {
            *m_HeaderFile   << "    private:\n"
                                    << "        int m_Rule;\n"
                                    << "\n"
                                    << "    public:\n"
                                    << "        virtual dfa::position pos() const;\n"
                                    << "        virtual dfa::position final_pos() const;\n";
        }

        // Get the definition for this item
        const ast_nonterminal& ntDefn = get_ast_nonterminal(nonterm->identifier);

        // The names of the variables that are defined for this rule
        set<string> definedVariables;

        // Iterate through the rules
        for (ast_nonterminal_rules::const_iterator ruleDefn = ntDefn.rules.begin(); ruleDefn != ntDefn.rules.end(); ++ruleDefn) {
            int ruleIdentifier = ruleDefn->first;

            // Start appending the private values for this class
            *m_HeaderFile << "\n    public:\n";
            *m_HeaderFile << "        // Rule " << ruleIdentifier << "\n";

            // Iterate through the items in this rule to create the variables used to store them
            bool validItems = false;
            for (ast_rule_item_list::const_iterator ruleItem = ruleDefn->second.begin(); ruleItem != ruleDefn->second.end(); ++ruleItem) {
                // Guard items don't get variables
                if (ruleItem->item->type() == item::guard) continue;

                // The EBNF repeat item doesn't get its own variable within a content item
                if (ruleItem->isEbnfRepetition) {
                    // In these rules, the repeating item has an empty name
                    // These are the only items that can be empty, so this is how we identify the repeating item
                    // when generating reductions
                    continue;
                }

                // There is at least one valid item in this rule
                validItems = true;

                // Add a variable definition for this rule item if this nonterminal doesn't already have one
                string varName = get_identifier(ruleItem->uniqueName, false);

                if (definedVariables.find(varName) == definedVariables.end()) {
                    // Get the type name for this variable
                    string typeName = class_name_for_item(ruleItem->item);

                    // Add a variable declaration
                    *m_HeaderFile << "        const util::syntax_ptr<" << typeName << s_TypeSuffix << "> " << varName << ";\n";

                    // This item is now defined for this class
                    definedVariables.insert(varName);
                } else {
                    *m_HeaderFile << "        // " << varName << " declared in another rule\n";
                }
            }

            // If there are no valid items then we need to declare a position field
            if (!validItems) {
                // Presumably there's only one of these
                *m_HeaderFile << "        dfa::position m_Position;";
            }

            // Declare the constructor for this rule if necessary
            bool declareConstructor = true;

            // The EBNF closures only need a single constructor, as we flatten them into vectors
            if (nonterm->item->type() == item::repeat) {
                // Only declare a constructor for the initial rule
                // (Generated rules are an initial rule and a repeating rule)
                if (ruleDefn->second[0].isEbnfRepetition) {
                    // This is the repeating rule
                    declareConstructor = false;
                }
            }

            else if (nonterm->item->type() == item::repeat_zero_or_one) {
                // Only declare a constructor for the non-empty rule for '*' closures
                // (Generated rules are an empty rule and a repeating rule)
                if (ruleDefn->second.empty()) {
                    declareConstructor = false;
                }
            }

            // Declare a constructor for this rule
            if (declareConstructor) {
                *m_HeaderFile   << "\n    public:\n"
                                << "        " << ntContentClass << "(";
                
                // Iterate through the items in this rule
                bool    first = true;
                int     index = 0;
                for (ast_rule_item_list::const_iterator ruleItem = ruleDefn->second.begin(); ruleItem != ruleDefn->second.end(); ++ruleItem) {
                    // Ignore guards
                    if (ruleItem->item->type() == item::guard) continue;

                    // Ignore repetitions
                    if (ruleItem->isEbnfRepetition) continue;

                    // Get the variable name and type for this item 
                    string varName  = get_identifier(ruleItem->uniqueName, false);
                    string typeName = class_name_for_item(ruleItem->item);

                    // Add to the list of parameters
                    if (!first) {
                        *m_HeaderFile << ", ";
                    }

                    // Declare as a reference to the syntax pointer
                    *m_HeaderFile << "const util::syntax_ptr<class " << typeName << s_TypeSuffix << ">& " << typeName << "_" << index;

                    // No longer the first rule
                    first = false;
                    index++;
                }

                // If there were no valid items, then we need to add a position to this constructor
                if (!validItems) {
                    *m_HeaderFile << "const dfa::position& pos";
                }

                *m_HeaderFile << ");\n";
            }
        }

        // Finish up with the destructor
        *m_HeaderFile   << "\n    public:\n"
                        << "        virtual ~" << ntContentClass << "();\n"
                        << "    };\n";

        // For repeating classes, also declare the container class
        if (repeatingItem) {
            *m_HeaderFile   << "\n"
                            << "    class " << ntName << " : public syntax_node {\n"
                            << "    public:\n"
                            << "        typedef util::syntax_ptr<class " << ntName << s_ContentSuffix << "> node_type;\n"
                            << "        typedef std::vector<node_type> data_type;\n"
                            << "        typedef data_type::const_iterator iterator;\n"
                            << "\n"
                            << "    private:\n"
                            << "        data_type m_Data;\n"
                            << "        dfa::position m_Position;\n"
                            << "\n"
                            << "    public:\n"
                            << "        inline operator const data_type&() const { return m_Data; }\n"
                            << "        inline const " << ntName << s_ContentSuffix << "* operator[](size_t index) const { return m_Data[index].item(); }\n"
                            << "        inline const size_t size() const { return m_Data.size(); }\n"
                            << "        inline iterator begin() const { return m_Data.begin(); }\n"
                            << "        inline iterator end() const { return m_Data.end(); }\n"
                            << "\n"
                            << "        inline void add_child(const node_type& newChild) { m_Data.push_back(newChild); }\n"
                            << "        inline void set_position(const dfa::position& newPos) { m_Position = newPos; }\n"
                            << "\n"
                            << "        virtual dfa::position pos() const;\n"
                            << "        virtual dfa::position final_pos() const;\n"
                            << "    };\n";
        }
    }
}

/// \brief Writes out the implementations of the AST classes to the source file
void output_cplusplus::source_ast_class_definitions() {
    // Write out the declarations for the syntax node
    *m_SourceFile   << "\n"
                    << get_identifier(m_ClassName, false) << "::syntax_node::~syntax_node() { }\n"
                    << "std::wstring " << get_identifier(m_ClassName, false) << "::syntax_node::to_string() { return L\"syntax_node\"; }\n"
                    << "dfa::position " << get_identifier(m_ClassName, false) << "::syntax_node::pos() const { return dfa::position(-1, -1, -1); }\n"
                    << "dfa::position " << get_identifier(m_ClassName, false) << "::syntax_node::final_pos() const { return dfa::position(-1, -1, -1); }\n";

    // Write out the definition of a terminal symbol
    *m_SourceFile << "\n" << get_identifier(m_ClassName, false) << "::terminal::terminal(const dfa::lexeme_container& lexeme) : m_Lexeme(lexeme) { }\n";
    *m_SourceFile << "\ndfa::position " << get_identifier(m_ClassName, false) << "::terminal::pos() const { return m_Lexeme->pos(); }\n";
    *m_SourceFile << "\ndfa::position " << get_identifier(m_ClassName, false) << "::terminal::final_pos() const { return m_Lexeme->final_pos(); }\n";
}

/// \brief Writes out the constructors for each nonterminal symbol
void output_cplusplus::source_ast_class_constructors() {
    // Write out the constructors for each nonterminal symbol
    for (nonterminal_symbol_iterator nonterm = begin_nonterminal_symbol(); nonterm != end_nonterminal_symbol(); ++nonterm) {
        // Guards have no definitions written out for them
        if (nonterm->item->type() == item::guard) {
            continue;
        }

        // Get the definition for this nonterminal
        const ast_nonterminal& ntDefn = get_ast_nonterminal(nonterm->identifier);

        // Get the name for this nonterminal
        string ntName = class_name_for_item(nonterm->item);

        // Append the type suffix
        ntName += s_TypeSuffix;

        // ... and the content suffix if this is a repeating item
        if (nonterm->item->type() == item::repeat || nonterm->item->type() == item::repeat_zero_or_one) {
            ntName += s_ContentSuffix;
        }

        // Write out a constructor for each rule for this nonterminal
        for (ast_nonterminal_rules::const_iterator ruleDefn = ntDefn.rules.begin(); ruleDefn != ntDefn.rules.end(); ++ruleDefn) {
            // Decide if we need to declare a constructor for this rule
            // The EBNF closures only need a single constructor, as we flatten them into vectors
            if (nonterm->item->type() == item::repeat) {
                // Only declare a constructor for the initial rule
                // (Generated rules are an initial rule and a repeating rule)
                if (ruleDefn->second[0].isEbnfRepetition) {
                    // This is the repeating rule: we only declare the constructor for the other rule
                    continue;
                }
            }

            else if (nonterm->item->type() == item::repeat_zero_or_one) {
                // Only declare a constructor for the non-empty rule for '*' closures
                // (Generated rules are an empty rule and a repeating rule)
                if (ruleDefn->second.empty()) {
                    // This is the empty rule: we only declare a constructor for the 'full' rule
                    continue;
                }
            }

            // Write out the declaration for this constructor
            *m_SourceFile << "\n// Rule " << ruleDefn->first << "\n";
            *m_SourceFile << get_identifier(m_ClassName, false) << "::" << ntName << "::" << ntName << "(";

            // Generate the parameters for the constructor by iterating through the items in the rule
            bool    first = true;
            int     index = 0;
            for (ast_rule_item_list::const_iterator ruleItem = ruleDefn->second.begin(); ruleItem != ruleDefn->second.end(); ++ruleItem) {
                // Ignore guards
                if (ruleItem->item->type() == item::guard) continue;

                // Ignore repetitions
                if (ruleItem->isEbnfRepetition) continue;

                // Get the variable name and type for this item 
                string varName  = get_identifier(ruleItem->uniqueName, false);
                string typeName = class_name_for_item(ruleItem->item);

                // Add to the list of parameters
                if (!first) {
                    *m_SourceFile << ", ";
                }

                // Declare as a reference to the syntax pointer
                *m_SourceFile << "const util::syntax_ptr<class " << typeName << s_TypeSuffix << ">& " << typeName << "_" << index;

                // No longer the first rule
                first = false;
                index++;
            }

            // If there were no valid items, then we need to add a position to this constructor
            if (index == 0) {
                *m_SourceFile << "const dfa::position& pos";
            }

            *m_SourceFile << ")\n";

            // Write out the initializers
            *m_SourceFile << ": m_Rule(" << ruleDefn->first << ")";

            // Empty rules need to fill in the position field
            if (index == 0) {
                *m_SourceFile << "\n, m_Position(pos)";
            }

            // Fill in the initialisers from the rule
            index = 0;
            for (ast_rule_item_list::const_iterator ruleItem = ruleDefn->second.begin(); ruleItem != ruleDefn->second.end(); ++ruleItem) {
                // Ignore guards
                if (ruleItem->item->type() == item::guard) continue;

                // Ignore repetitions
                if (ruleItem->isEbnfRepetition) continue;

                // Get the variable name and type for this item 
                string varName  = get_identifier(ruleItem->uniqueName, false);
                string typeName = class_name_for_item(ruleItem->item);

                // Declare as a reference to the syntax pointer
                *m_SourceFile   << "\n"
                                << ", " << varName << "(" << typeName << "_" << index << ")";

                // Index moves on
                index++;
            }

            // Write out the body of the constructor
            *m_SourceFile   << " {\n"
                            << "}\n";
        }

        // Write out a destructor for this nonterminal
        *m_SourceFile << "\n" << get_identifier(m_ClassName, false) << "::" << ntName << "::~" << ntName << "() { }\n";
    }
}


/// \brief Writes out the functions for getting the file positions of each symbol
void output_cplusplus::source_ast_position_functions() {
    // Write out the position and final position definitions for each nonterminal symbol
    for (nonterminal_symbol_iterator nonterm = begin_nonterminal_symbol(); nonterm != end_nonterminal_symbol(); ++nonterm) {
        // Guards have no definitions written out for them
        if (nonterm->item->type() == item::guard) {
            continue;
        }

        // Get the name for this nonterminal
        string ntName = class_name_for_item(nonterm->item);

        // Append the type suffix
        ntName += s_TypeSuffix;

        // Fetch the declaration for this nonterminal
        const ast_nonterminal& astNt = get_ast_nonterminal(nonterm->identifier);

        // Sdet a flag as to whether or not this is a repeating (closure) item
        bool repeatingItem = false;
        if (nonterm->item->type() == item::repeat || nonterm->item->type() == item::repeat_zero_or_one) {
            repeatingItem = true;
        }

        // Write out the initial position definitions
        *m_SourceFile   << "\n"
                        << "dfa::position " << get_identifier(m_ClassName, false) << "::" << ntName << (repeatingItem?s_ContentSuffix:"") << "::pos() const {\n"
                        << "    switch (m_Rule) {";
        
        // The container of the initial position depends on which rule was matched
        for (ast_nonterminal_rules::const_iterator ruleDefn = astNt.rules.begin(); ruleDefn != astNt.rules.end(); ++ruleDefn) {
            // In case we get this rule...
            *m_SourceFile << "\n    case " << ruleDefn->first << ":\n";

            // Iterate through the items in the rule to find the first one that has a variable declared
            bool foundValid = false;
            for (ast_rule_item_list::const_iterator ruleItem = ruleDefn->second.begin(); ruleItem != ruleDefn->second.end(); ++ruleItem) {
                // Guard items don't get variables
                if (ruleItem->item->type() == item::guard) continue;

                // Neither does the 'repeat' item of a * or + closure
                if (repeatingItem && ruleItem->item->type() == nonterm->item->type() && ruleItem->item->symbol() == nonterm->item->symbol()) {
                    continue;
                }

                // This item has a variable, so we can return its first position as the position of this item
                foundValid = true;
                *m_SourceFile << "        return " << get_identifier(ruleItem->uniqueName, false) << "->pos();\n";

                // Only write out a single position item
                break;
            }

            // If the item doesn't have a position then return the contents of the m_Position variable
            if (!foundValid) {
                *m_SourceFile << "        return m_Position;\n";
            }
        }
        
        // Default is an unknown position (should never happen if the parser works correctly)
        *m_SourceFile   << "\n"
                        << "    default:\n"
                        << "        return dfa::position(-1, -1, -1);\n"
                        << "    }\n"
                        << "}\n";

        // ... and the final position definitions
        *m_SourceFile   << "\n"
                        << "dfa::position " << get_identifier(m_ClassName, false) << "::" << ntName << (repeatingItem?s_ContentSuffix:"")  << "::final_pos() const {\n"
                        << "    switch (m_Rule) {";

        // The container of the final position depends on which rule was matched
        for (ast_nonterminal_rules::const_iterator ruleDefn = astNt.rules.begin(); ruleDefn != astNt.rules.end(); ++ruleDefn) {
            // In case we get this rule...
            *m_SourceFile << "\n    case " << ruleDefn->first << ":\n";

            // Iterate through the items in the rule to find the last one that has a variable declared
            bool foundValid = false;
            for (ast_rule_item_list::const_reverse_iterator ruleItem = ruleDefn->second.rbegin(); ruleItem != ruleDefn->second.rend(); ++ruleItem) {
                // Guard items don't get variables
                if (ruleItem->item->type() == item::guard) continue;

                // Neither does the 'repeat' item of a * or + closure
                if (repeatingItem && ruleItem->item->type() == nonterm->item->type() && ruleItem->item->symbol() == nonterm->item->symbol()) {
                    continue;
                }

                // This item has a variable, so we can return its final position as the position of this item
                foundValid = true;
                *m_SourceFile << "        return " << get_identifier(ruleItem->uniqueName, false) << "->final_pos();\n";

                // Only write out a single position item
                break;
            }

            // If the item doesn't have a position then return the contents of the m_Position variable
            if (!foundValid) {
                *m_SourceFile << "        return m_Position;\n";
            }
        }

        // Default is an unknown position
        *m_SourceFile   << "\n"
                        << "    default:\n"
                        << "        return dfa::position(-1, -1, -1);\n"
                        << "    }\n"
                        << "}\n";

        // Write out the pos/final_pos items for the container class for repeating items
        if (repeatingItem) {
            *m_SourceFile           << "\n"
                        << "dfa::position " << get_identifier(m_ClassName, false) << "::" << ntName << "::pos() const {\n"
                        << "    if (m_Data.empty()) return m_Position;\n"
                        << "    return m_Data.front()->pos();\n"
                        << "}\n"
                        << "\n"
                        << "dfa::position " << get_identifier(m_ClassName, false) << "::" << ntName << "::final_pos() const {\n"
                        << "    if (m_Data.empty()) return m_Position;\n"
                        << "    return m_Data.back()->final_pos();\n"
                        << "}\n";
        }
    }
}

/// \brief Writes out the parser actions to the header file
void output_cplusplus::header_parser_actions() {
    // Declare the parser actions class
    *m_HeaderFile   << "\n"
                    << "    class parser_actions {\n"
                    << "    public:\n"
                    << "        typedef util::syntax_ptr<syntax_node> node;\n"
                    << "        typedef lr::parser<node, parser_actions> parser;\n"
                    << "        typedef parser::reduce_list reduce_list;\n"
                    << "\n"
                    << "    private:\n"
                    << "        dfa::lexeme_stream* m_Stream;\n"
                    << "        bool m_OwnStream;\n"
                    << "\n"
                    << "        parser_actions(parser_actions& noCopying);\n"
                    << "        parser_actions& operator=(const parser_actions& noCopying);\n"
                    << "\n"
                    << "    public:\n"
                    << "        parser_actions(dfa::lexeme_stream* stream, bool ownStream = false)\n"
                    << "        : m_Stream(stream)\n"
                    << "        , m_OwnStream(ownStream) { }\n"
                    << "\n"
                    << "        ~parser_actions() {\n"
                    << "            if (m_OwnStream && m_Stream) {\n"
                    << "                delete m_Stream;\n"
                    << "                m_Stream = NULL;\n"
                    << "            }\n"
                    << "        }\n"
                    << "\n"
                    << "        inline dfa::lexeme* read() {\n"
                    << "            dfa::lexeme* result = NULL;\n"
                    << "            (*m_Stream) >> result;\n"
                    << "            return result;\n"
                    << "        }\n"
                    << "\n"
                    << "        node shift(const dfa::lexeme_container& lexeme);\n"
                    << "\n"
                    << "        node reduce(int nonterminal, int rule, const reduce_list& reduce, const dfa::position& lookaheadPosition);\n"
                    << "    };\n";
}

/// \brief Writes out the shift actions to the source file
void output_cplusplus::source_shift_actions() {
    string className = get_identifier(m_ClassName, false);

    // Declare the shift function
    *m_SourceFile   << "\n" 
                    << className << "::parser_actions::node " << className << "::parser_actions::shift(const dfa::lexeme_container& lexeme) {\n"
                    << "    switch (lexeme->matched()) {";
    
    // Generate a shift action for each terminal symbol
    for (terminal_symbol_iterator term = begin_terminal_symbol(); term != end_terminal_symbol(); ++term) {
        // Get the name for this terminal
        string name = class_name_for_item(term->item);

        // Turn into a type name
        name += s_TypeSuffix;

        // Declare a shift action for this symbol
        *m_SourceFile   << "\n    case " << term->identifier << ": // " << get_identifier(terminals().name_for_symbol(term->identifier), true) << "\n"
                        << "        return node(new " << name << "(lexeme));\n";
    }
                    
    // Default actions is to create an empty node
    *m_SourceFile   << "\n    default:\n"
                    << "        return node(new terminal(lexeme));\n"
                    << "    }\n"
                    << "}\n";
}

/// \brief Writes out the reduce actions to the source file
void output_cplusplus::source_reduce_actions() {
    string className = get_identifier(m_ClassName, false);

    // Declare the reduce function
    *m_SourceFile   << "\n"
                    << className << "::parser_actions::node " << className << "::parser_actions::reduce(int nonterminal, int rule, const reduce_list& reduce, const dfa::position& lookaheadPosition) {\n"
                    << "    switch (rule) {";

    // Iterate through the nonterminals
    for (nonterminal_symbol_iterator nonterm = begin_nonterminal_symbol(); nonterm != end_nonterminal_symbol(); ++nonterm) {
        // Get the definition for this nonterminal
        const ast_nonterminal& ntDefn = get_ast_nonterminal(nonterm->identifier);

        // Get the name for this nonterminal
        string ntName = class_name_for_item(nonterm->item);

        // Append the type suffix
        ntName += s_TypeSuffix;

        // Get the name of the content class
        string ntContentClass = ntName;

        // ... and the content suffix if this is a repeating item
        if (nonterm->item->type() == item::repeat || nonterm->item->type() == item::repeat_zero_or_one) {
            ntContentClass += s_ContentSuffix;
        }

        // Write out a constructor for each rule for this nonterminal
        for (ast_nonterminal_rules::const_iterator ruleDefn = ntDefn.rules.begin(); ruleDefn != ntDefn.rules.end(); ++ruleDefn) {
            // Begin a new case statement for this rule
            *m_SourceFile   << "\n    case " << ruleDefn->first << ":\n"
                            << "    {\n";

            // TODO: we have the rules for this in three separate places now: factor into a function
            bool hasConstructor = true;
            bool repeating      = false;

            // The EBNF closures only need a single constructor, as we flatten them into vectors
            if (nonterm->item->type() == item::repeat) {
                // Only declare a constructor for the initial rule
                // (Generated rules are an initial rule and a repeating rule)
                repeating = true;
                if (ruleDefn->second[0].isEbnfRepetition) {
                    // This is the repeating rule: we only declare the constructor for the other rule
                    hasConstructor = false;
                }
            }

            else if (nonterm->item->type() == item::repeat_zero_or_one) {
                // Only declare a constructor for the non-empty rule for '*' closures
                // (Generated rules are an empty rule and a repeating rule)
                repeating = true;
                if (ruleDefn->second.empty()) {
                    // This is the empty rule: we only declare a constructor for the 'full' rule
                    hasConstructor = false;
                }
            }

            else if (nonterm->item->type() == item::guard) {
                // Guard items are always ignored
                hasConstructor = false;
            }

            // Construct the content item for any item with a constructor, or all of the items for a repeating item
            if (hasConstructor || nonterm->item->type() == item::repeat) {
                if (nonterm->item->type() == item::repeat || nonterm->item->type() == item::repeat_zero_or_one) {
                    // For repeating items, we construct the content into a variable
                    *m_SourceFile << "        util::syntax_ptr<class " << ntContentClass << "> content(new " << ntContentClass << "(";
                } else {
                    // For non-repeating items, just return the item directly
                    *m_SourceFile << "        return node(new " << ntContentClass << "(";
                }

                // Generate the constructor parameters
                bool first = true;
                for (size_t index = 0; index < ruleDefn->second.size(); ++index) {
                    // Get the item at this index
                    const ast_rule_item& ruleItem = ruleDefn->second[index];

                    // Guard items are ignored
                    if (ruleItem.item->type() == item::guard) continue;

                    // ... as are repetition items
                    if (ruleItem.isEbnfRepetition) continue;

                    // Other items are put into the constructor
                    string typeName = class_name_for_item(ruleItem.item);

                    // Add commas to separate the values
                    if (!first) {
                        *m_SourceFile << ", ";
                    }
                    first = false;

                    // The reduce list is passed in in reverse
                    size_t reduceIndex = ruleDefn->second.size() - index - 1;

                    // Cast to the type
                    *m_SourceFile  << "reduce[" << reduceIndex << "].cast_to<" << typeName << s_TypeSuffix << ">()";
                }

                // If there aren't any valid items, then pass in the lookahead position (these items will take a position parameter)
                if (first) {
                    *m_SourceFile << "lookaheadPosition";
                }

                *m_SourceFile << "));\n";
            }

            // For repeating items either create or retrieve the node
            if (repeating) {
                // The constructor is delcared for the item that contains the repetition
                if (!hasConstructor && nonterm->item->type() == item::repeat_zero_or_one) {
                    // This is the empty rule in a zero-or-more repetition: create an empty item
                    *m_SourceFile << "        return node(new " << ntName << "());\n";
                } else {
                    // Get the node where the definition is being built up
                    *m_SourceFile << "        util::syntax_ptr<class " << ntName << "> list(";

                    // If the first item is a repetition then use that, otherwise create a new item
                    if (!ruleDefn->second.empty() && ruleDefn->second[0].isEbnfRepetition) {
                        // The first item is the repetition
                        *m_SourceFile << "reduce[" << ruleDefn->second.size()-1 << "].cast_to<" << ntName << ">());\n";
                    } else {
                        // Need to create a new item
                        *m_SourceFile << "new " << ntName << "());\n";

                        // Set the position (hideous const cast, sigh)
                        *m_SourceFile << "        const_cast<" << ntName << "*>(list.item())->set_position(lookaheadPosition);\n";
                    }

                    // Add the content as a child item
                    // Hideous const cast :-(
                    *m_SourceFile << "        const_cast<" << ntName << "*>(list.item())->add_child(content);\n";

                    // Cast to a node and return
                    *m_SourceFile << "        return list.cast_to<syntax_node>();\n";
                }
            }

            // Finished this case definition
            *m_SourceFile << "    }\n";
        }
    }

    // Default action is to generate an empty node  
    *m_SourceFile   << "\n    default:\n"
                    << "        return node();\n"
                    << "    }\n"
                    << "}\n";
}
