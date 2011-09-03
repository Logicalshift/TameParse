//
//  cplusplus.cpp
//  Parse
//
//  Created by Andrew Hunter on 29/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <time.h>
#include <sstream>
#include <algorithm>

#include "Compiler/OutputStages/cplusplus.h"

using namespace std;
using namespace dfa;
using namespace compiler;

/// \brief Creates a new output stage
output_cplusplus::output_cplusplus(console_container& console, const std::wstring& filename, lexer_stage* lexer, language_stage* language, lr_parser_stage* parser, const std::wstring& filenamePrefix, const std::wstring& className, const std::wstring& namespaceName)
: output_stage(console, filename, lexer, language, parser)
, m_FilenamePrefix(filenamePrefix)
, m_ClassName(className)
, m_Namespace(namespaceName)
, m_SourceFile(NULL)
, m_HeaderFile(NULL)
, m_SymbolLevels(NULL) {
}

/// \brief Destructor
output_cplusplus::~output_cplusplus() {
	if (m_SourceFile) 	delete m_SourceFile;
	if (m_HeaderFile) 	delete m_HeaderFile;
	if (m_SymbolLevels) delete m_SymbolLevels;
}

/// \brief Converts a string to upper case
static string toupper(const string& s) {
	string res = s;
    for (string::iterator c = res.begin(); c != res.end(); c++) {
        *c = std::toupper(*c);
    }
	return res;
}

/// \brief Returns a valid C++ identifier for the specified symbol name
std::string output_cplusplus::get_identifier(const std::wstring& name) {
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
	for (wstring::const_iterator wideChar = stripped.begin(); wideChar != stripped.end(); wideChar++) {
		// Just append values that are valid C++ identifiers
        wchar_t c = *wideChar;
		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (res.tellp() > 0 && c >= '0' && c <= '9')) {
			res << (char) c;
			continue;
		}

		// Some punctuation characters are written out as names
		switch (c) {
			case L'<':	res << "_lessthan_"; break;
			case L'>':	res << "_greaterthan_"; break;
			case L'\'':	res << "_quote_"; break;
			case L'"':	res << "_doublequote_"; break;
			case L'&':	res << "_ampersand_"; break;
			case L':':	res << "_colon_"; break;
			case L'=':	res << "_equals_"; break;
			case L'!':	res << "_exclamation_"; break;
			case L'@':	res << "_at_"; break;
			case L'#':	res << "_hash_"; break;
			case L'$':	res << "_dollar_"; break;
			case L'%':	res << "_percent_"; break;
			case L'^':	res << "_cicumflex_"; break;
			case L'*':	res << "_star_"; break;
			case L'(':	res << "_openparen_"; break;
			case L')':	res << "_closeparen_"; break;
			case L'+':	res << "_plus_"; break;
			case L'/':	res << "_slash_"; break;
			case L'\\':	res << "_backslash_"; break;
			case L'|':	res << "_pipe_"; break;
			case L';':	res << "_semicolon_"; break;
			case L'.':	res << "_dot_"; break;
			case L',':	res << "_comma_"; break;
			case L'?':	res << "_question_"; break;
			case L'~':	res << "_tilde_"; break;
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
	(*m_HeaderFile) << "#ifndef TAMEPARSE_PARSER_" << toupper(get_identifier(m_FilenamePrefix)) << "\n";
	(*m_HeaderFile) << "#define TAMEPARSE_PARSER_" << toupper(get_identifier(m_FilenamePrefix)) << "\n";
    (*m_HeaderFile) << "\n";
    
    if (!m_Namespace.empty()) {
        (*m_HeaderFile) << "namespace " << get_identifier(m_Namespace) << " {\n";
    }
    (*m_HeaderFile) << "class " << get_identifier(m_ClassName) << " {\n";
    
    // Write out the boilerplate at hte start of the source file (include the header)
    (*m_SourceFile) << "#include \"" << cons().convert_filename(headerFilename) << "\"\n";
}

/// \brief Finishing writing out output
void output_cplusplus::end_output() {
    // Finish off the header file
    (*m_HeaderFile) << "\n";
    (*m_HeaderFile) << "};\n";            // End of class
    if (!m_Namespace.empty()) {
        (*m_HeaderFile) << "}\n";         // End of namespace
    }
}

/// \brief The output stage is about to produce a list of terminal symbols
void output_cplusplus::begin_terminal_symbols() {
    // Create a public class to contain the list of terminal identifiers
    (*m_HeaderFile) << "\npublic:\n";
    (*m_HeaderFile) << "    class t {\n";
    (*m_HeaderFile) << "    public:\n";
}

/// \brief Specifies the identifier for the terminal symbol with a given name
void output_cplusplus::terminal_symbol(const std::wstring& name, int identifier) {
    // Get the short name
    string       shortName = get_identifier(name);
    
    // Choose a unique name (gets a bit weird if we chose a unique name that later clashes)
    stringstream ourName;
    
    ourName << shortName;
    int count = m_TerminalSymbolCount[shortName];
    
    while (m_TerminalSymbolCount[ourName.str()] > 0) {
        ourName << "_" << count;
    }

    m_TerminalSymbolCount[shortName]++;

    // Output a constant for this terminal
    (*m_HeaderFile) << "        static const int " << ourName.str() << " = " << identifier << ";\n";
}

/// \brief Finished writing out the terminal symbols
void output_cplusplus::end_terminal_symbols() {
    (*m_HeaderFile) << "    };\n";
}

/// \brief The output stage is about to produce a list of non-terminal symbols
void output_cplusplus::begin_nonterminal_symbols() {
    // Create a public class to contain the list of nonterminal identifiers
    (*m_HeaderFile) << "\npublic:\n";
    (*m_HeaderFile) << "    class nt {\n";
    (*m_HeaderFile) << "    public:\n";
}

/// \brief Specifies the identifier for the non-terminal symbol with a given name
void output_cplusplus::nonterminal_symbol(const std::wstring& name, int identifier) {
    // Get the short name
    string       shortName = get_identifier(name);
    
    if (name.empty()) {
        // Some nonterminals don't have a name
        shortName = "_unnamed";
    }
    
    // Choose a unique name (gets a bit weird if we chose a unique name that later clashes)
    stringstream ourName;
    
    ourName << shortName;
    int count = m_NonterminalSymbolCount[shortName];
    
    while (m_NonterminalSymbolCount[ourName.str()] > 0) {
        ourName << "_" << count;
    }
    
    m_NonterminalSymbolCount[shortName]++;
    
    // Output a constant for this terminal
    (*m_HeaderFile) << "        static const int " << ourName.str() << " = " << identifier << ";\n";
}

/// \brief Finished writing out the terminal symbols
void output_cplusplus::end_nonterminal_symbols() {
    (*m_HeaderFile) << "    };\n";
}

/// \brief Starting to write out the symbol map for the lexer
void output_cplusplus::begin_lexer_symbol_map(int maxSetId) {
	// Write out the number of symbol sets
	(*m_HeaderFile) << L"\npublic:\n";
	(*m_HeaderFile) << L"    static const int number_of_symbol_sets = " << maxSetId << L";\n";

	// Include the hard-coded symbol table in the source file
	(*m_SourceFile) << L"\n#include \"Dfa/hard_coded_symbol_table.h\"\n";

	// Begin building the symbol levels object
	if (m_SymbolLevels) {
		delete m_SymbolLevels;
	}
	m_SymbolLevels = new symbol_table<wchar_t>();
}

/// \brief Specifies that a given range of symbols maps to a particular identifier
void output_cplusplus::symbol_map(const dfa::range<int>& symbolRange, int identifier) {
	// Just add to the symbol levels
	m_SymbolLevels->add_range(symbolRange, identifier);
}

/// \brief Finishing writing out the symbol map for the lexer
void output_cplusplus::end_lexer_symbol_map() {
	// Begin writing out the symbol map table
	(*m_SourceFile) << L"\nstatic const int s_SymbolMapTable = {";

	// Convert to a hard-coded table
	size_t	size;
	int* 	hcst = m_SymbolLevels->Table.to_hard_coded_table(size);

	// Write it out
	for (size_t tablePos = 0; tablePos < size; tablePos++) {
		// Add newlines
		if ((tablePos % 20) == 0) {
			(*m_SourceFile) << L"\n    ";
		}

		// Write out this entry
		(*m_SourceFile) << L"0x" << hex << hcst[tablePos] << dec;
		if (tablePos+1 < size) {
			(*m_SourceFile) << L", ";
		}
	}

	// Finished with the table
	delete[] hcst;

	// Finished the table
	(*m_SourceFile) << L"\n    };\n";
}

/// \brief About to begin writing out the lexer tables
void output_cplusplus::begin_lexer_state_machine(int numStates) {
	// TODO: implement me
}

/// \brief Starting to write out the transitions for a given state
void output_cplusplus::begin_lexer_state(int stateId) {
	// TODO: implement me
}

/// \brief Adds a transition for the current state
void output_cplusplus::lexer_state_transition(int symbolSet, int newState) {
	// TODO: implement me
}

/// \brief Finishes writing out a lexer state
void output_cplusplus::end_lexer_state() {
	// TODO: implement me
}

/// \brief Finished writing out the lexer table
void output_cplusplus::end_lexer_state_machine() {
	// TODO: implement me
}

/// \brief About to write out the list of accepting states for a lexer
void output_cplusplus::begin_lexer_accept_table() {
	// TODO: implement me
}

/// \brief The specified state is not an accepting state
void output_cplusplus::nonaccepting_state(int stateId) {
	// TODO: implement me
}

/// \brief The specified state is an accepting state
void output_cplusplus::accepting_state(int stateId, int acceptSymbolId) {
	// TODO: implement me
}

/// \brief Finished the lexer acceptance table
void output_cplusplus::end_lexer_accept_table() {
	// TODO: implement me
}
