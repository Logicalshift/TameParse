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
using namespace contextfree;
using namespace compiler;

/// \brief Creates a new output stage
output_cplusplus::output_cplusplus(console_container& console, const std::wstring& filename, lexer_stage* lexer, language_stage* language, lr_parser_stage* parser, const std::wstring& filenamePrefix, const std::wstring& className, const std::wstring& namespaceName)
: output_stage(console, filename, lexer, language, parser)
, m_FilenamePrefix(filenamePrefix)
, m_ClassName(className)
, m_Namespace(namespaceName)
, m_SourceFile(NULL)
, m_HeaderFile(NULL)
, m_SymbolLevels(NULL)
, m_NtForwardDeclarations(NULL)
, m_NtClassDefinitions(NULL) {
}

/// \brief Destructor
output_cplusplus::~output_cplusplus() {
	if (m_SourceFile) 				delete m_SourceFile;
	if (m_HeaderFile) 				delete m_HeaderFile;
	if (m_SymbolLevels) 			delete m_SymbolLevels;
	if (m_NtForwardDeclarations)	delete m_NtForwardDeclarations;
	if (m_NtClassDefinitions)		delete m_NtClassDefinitions;
}

/// \brief Converts a string to upper case
static string toupper(const string& s) {
	string res = s;
    for (string::iterator c = res.begin(); c != res.end(); c++) {
        *c = std::toupper(*c);
    }
	return res;
}


// 				===================
//  			 String converters
// 				===================

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

/// \brief Returns a valid C++ name for a grammar rule
std::string output_cplusplus::name_for_rule(const contextfree::rule_container& thisRule, const contextfree::grammar& gram, const contextfree::terminal_dictionary& terminals) {
	// Zero-length rules are called 'empty'
	if (thisRule->items().size() == 0) {
		return "empty";
	}

	// Short rules are just named after the items
	else if (thisRule->items().size() <= 3) {
		bool 			first = true;
		stringstream	res;

		for (int itemId = 0; itemId < thisRule->items().size(); itemId++) {
			// Append divider
			if (!first) {
				res << "_";
			}

			// Append this item
			res << name_for_item(thisRule->items()[itemId], gram, terminals);

			// No longer first
			first = false;
		}

		return res.str();
	}

	// Other rules are named after the first item and _etc
	else {
		return name_for_item(thisRule->items()[0], gram, terminals) + "_etc";		
	}
}

/// \brief Returns a valid C++ name for an EBNF item
std::string output_cplusplus::name_for_ebnf_item(const contextfree::ebnf& ebnfItem, const contextfree::grammar& gram, const contextfree::terminal_dictionary& terminals) {
	// Work out the number of rules in this item
	size_t numRules = ebnfItem.count_rules();

	// Items with no rules are just called 'empty'
	if (numRules == 0) {
		return "empty";
	}

	// Items with rules are called 'x' or 'y' or 'z' etc
	else {
		stringstream	res;
		bool 			first = true;

		for (ebnf::rule_iterator nextRule = ebnfItem.first_rule(); nextRule != ebnfItem.last_rule(); nextRule++) {
			// Append _or_
			if (!first) {
				res << "_or_";
			}

			// Append the name for this rule
			res << name_for_rule(*nextRule, gram, terminals);
			
			// Move on
			first = false;
		}

		// Convert to a string
		return res.str();
	}
}

/// \brief Returns a valid C++ name for the specified item
///
/// This can be treated as a base name for getting names for nonterminals with particular identifiers
std::string output_cplusplus::name_for_item(const contextfree::item_container& it, const contextfree::grammar& gram, const contextfree::terminal_dictionary& terminals) {
	// Start building up the result
	stringstream res;

	// Action depends on the kind of item
	switch (it->type()) {
	case item::empty:
		res << "epsilon";
		break;

	case item::eoi:
		res << "end_of_input";
		break;

	case item::eog:
		res << "end_of_guard";
		break;

	case item::terminal:
		res << get_identifier(terminals.name_for_symbol(it->symbol()));
		break;

	case item::nonterminal:
		res << get_identifier(gram.name_for_nonterminal(it->symbol()));
		break;

	case item::optional:
		res << "optional_" << name_for_ebnf_item((const ebnf&)*it, gram, terminals);
		break;

	case item::repeat:
	case item::repeat_zero_or_one:
		res << "list_of_" << name_for_ebnf_item((const ebnf&)*it, gram, terminals);
		break;

	case item::alternative:
		res << "one_of_" << name_for_ebnf_item((const ebnf&)*it, gram, terminals);
		break;

	default:
		// Unknown type of item
		res << "unknown_item";
		break;
	}

	// Don't allow 0-length item names
	if (res.tellp() == 0) {
		res << "item";
	}

	// Return the result
	return res.str();
}

/// \brief Retrieves or assigns a name for a nonterminal with the specified ID
std::string output_cplusplus::name_for_nonterminal(int ntId, const contextfree::item_container& item, const contextfree::grammar& gram, const contextfree::terminal_dictionary& terminals) {
	// Try to find an existing name for this nonterminal
	map<int, string>::const_iterator found = m_ClassNameForItem.find(ntId);
	if (found != m_ClassNameForItem.end()) {
		// Use the found definition if it exists
		return found->second;
	}

	// Get the base name for this item
	string baseName = name_for_item(item, gram, terminals);
	if (baseName.size() == 0) baseName = "unknown";

	// Find a unique name that we can assign to this item
	int variant = 0;

	for (;;) {
		// Get the next variant
		variant++;

		// Generate the variant name
		stringstream varNameStream;
		varNameStream << baseName;
		if (variant > 1) {
			varNameStream << "_" << variant;
		}

		string varName = varNameStream.str();

		// See if this name has already been used
		if (m_UsedClassNames.find(varName) == m_UsedClassNames.end()) {
			// This is the name we shall use

			// Set as the name for this nonterminal, and mark as used
			m_ClassNameForItem[ntId] = varName;
			m_UsedClassNames.insert(varName);

			// Return as the result
			return varName;
		}

		// Keep trying until we find a valid name
	}
}


// 				================
//  			 General output
// 				================

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

    (*m_HeaderFile) << "#include \"Util/syntax_ptr.h\"\n";
    (*m_HeaderFile) << "#include \"Dfa/lexer.h\"\n";
    (*m_HeaderFile) << "#include \"Lr/parser.h\"\n";
    (*m_HeaderFile) << "#include \"Lr/parser_tables.h\"\n";
    (*m_HeaderFile) << "\n";
    
    if (!m_Namespace.empty()) {
        (*m_HeaderFile) << "namespace " << get_identifier(m_Namespace) << " {\n";
    }
    (*m_HeaderFile) << "class " << get_identifier(m_ClassName) << " {\n";

    // Add to the list of used class names
    m_UsedClassNames.insert(get_identifier(m_ClassName));
    
    // Write out the boilerplate at the start of the source file (include the header)
    (*m_SourceFile) << "#include \"" << cons().convert_filename(headerFilename) << "\"\n";

    if (!m_Namespace.empty()) {
        (*m_SourceFile) << "using namespace " << get_identifier(m_Namespace) << ";\n";
    }
}

/// \brief Finishing writing out output
void output_cplusplus::end_output() {
    // Finish off the header file
    (*m_HeaderFile) << "\n";
    (*m_HeaderFile) << "};\n";            // End of class
    if (!m_Namespace.empty()) {
        (*m_HeaderFile) << "}\n";         // End of namespace
    }

    (*m_HeaderFile) << "\n#endif\n";	  // End of the conditional
}


// 				=========
//  			 Symbols
// 				=========

/// \brief The output stage is about to produce a list of terminal symbols
void output_cplusplus::begin_terminal_symbols(const contextfree::grammar& gram) {
    // Create a public class to contain the list of terminal identifiers
    (*m_HeaderFile) << "\npublic:\n";
    (*m_HeaderFile) << "    class t {\n";
    (*m_HeaderFile) << "    public:\n";

    // Add to the list of used class names
    m_UsedClassNames.insert("t");
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
void output_cplusplus::begin_nonterminal_symbols(const contextfree::grammar& gram) {
    // Create a public class to contain the list of nonterminal identifiers
    (*m_HeaderFile) << "\npublic:\n";
    (*m_HeaderFile) << "    class nt {\n";
    (*m_HeaderFile) << "    public:\n";

    // Add to the list of used class names
    m_UsedClassNames.insert("nt");
}

/// \brief Specifies the identifier for the non-terminal symbol with a given name
void output_cplusplus::nonterminal_symbol(const std::wstring& name, int identifier, const contextfree::item_container& item) {
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


// 				==================
//  			 Lexer generation
// 				==================

/// \brief Starting to write out the symbol map for the lexer
void output_cplusplus::begin_lexer_symbol_map(int maxSetId) {
	// TODO: support symbol maps for alphabets other than wchar_t

    // Add to the list of used class names
    m_UsedClassNames.insert("number_of_symbol_sets");

	// Write out the number of symbol sets
	(*m_HeaderFile) << "\npublic:\n";
	(*m_HeaderFile) << "    static const int number_of_symbol_sets = " << maxSetId << ";\n";

	// Include the hard-coded symbol table in the source file
	(*m_SourceFile) << "\n#include \"Dfa/hard_coded_symbol_table.h\"\n";

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
	(*m_SourceFile) << "\nstatic const int s_SymbolMapTable[] = {";

	// Convert to a hard-coded table
	size_t	size;
	int* 	hcst = m_SymbolLevels->Table.to_hard_coded_table(size);

	// Write it out
	for (size_t tablePos = 0; tablePos < size; tablePos++) {
		// Add newlines
		if ((tablePos % 10) == 0) {
			(*m_SourceFile) << "\n        ";
		}

		// Write out this entry
		(*m_SourceFile) << dec << hcst[tablePos];
		if (tablePos+1 < size) {
			(*m_SourceFile) << ", ";
		}
	}

	// Finished with the table
	delete[] hcst;

	// Finished the table
	(*m_SourceFile) << "\n    };\n";
    
    // Add the symbol table class
    (*m_SourceFile) << "\nstatic const dfa::hard_coded_symbol_table<wchar_t, 2> s_SymbolMap(s_SymbolMapTable);\n";
}

/// \brief About to begin writing out the lexer tables
void output_cplusplus::begin_lexer_state_machine(int numStates) {
	// Write out the number of states to the header file
	(*m_HeaderFile) << "\n        static const int number_of_lexer_states = " << numStates << ";\n";

    // Add to the list of used class names
    m_UsedClassNames.insert("number_of_lexer_states");

	// Need to include the state machine class
	(*m_SourceFile) << "\n#include \"Dfa/state_machine.h\"\n";

	// Begin writing out the state machine table
	// TODO: support table styles other than 'compact' (the flat table is faster for all character types and more compact for some lexer types)
	(*m_SourceFile) << "\nstatic const dfa::state_machine_compact_table<false>::entry s_LexerStateMachine[] = {\n";

	// Reset the 
	m_LexerEntryPos = 0;
}

/// \brief Starting to write out the transitions for a given state
void output_cplusplus::begin_lexer_state(int stateId) {
	// Write out a comment
	(*m_SourceFile) << "\n\n        // State " << stateId << "\n        ";

	// Remember the current state
	m_LexerCurrentState = stateId;

	// Record its position
	m_StateToEntryOffset.push_back(m_LexerEntryPos);
}

/// \brief Adds a transition for the current state
void output_cplusplus::lexer_state_transition(int symbolSet, int newState) {
	// Write out a separator
	if (m_LexerEntryPos > 0) {
		(*m_SourceFile) << ", ";
		if ((m_LexerEntryPos%10) == 0) {
			(*m_SourceFile) << "\n        ";
		}
	}

	// Write out this transition
	(*m_SourceFile) << "{ " << symbolSet << ", " << newState << " }";

	// Update the lexer state position
	m_LexerEntryPos++;
}

/// \brief Finishes writing out a lexer state
void output_cplusplus::end_lexer_state() {
	// Nothing to do
}

/// \brief Finished writing out the lexer table
void output_cplusplus::end_lexer_state_machine() {
	// Finish off the table
	(*m_SourceFile) << "\n    };\n";
}

/// \brief About to write out the list of accepting states for a lexer
void output_cplusplus::begin_lexer_accept_table() {
	// Start the accepting action tables
	(*m_SourceFile) << "\nstatic const int s_AcceptingStates[] = {\n        ";
}

/// \brief The specified state is not an accepting state
void output_cplusplus::nonaccepting_state(int stateId) {
	if (stateId > 0) {
		(*m_SourceFile) << ", ";
	}

	// Non-accepting states get -1 as the action
	(*m_SourceFile) << "-1";
}

/// \brief The specified state is an accepting state
void output_cplusplus::accepting_state(int stateId, int acceptSymbolId) {
	if (stateId > 0) {
		(*m_SourceFile) << ", ";
	}

	// Write out the action for this state
	(*m_SourceFile) << acceptSymbolId;
}

/// \brief Finished the lexer acceptance table
void output_cplusplus::end_lexer_accept_table() {
	// Finish up the acceptance table
	(*m_SourceFile) << "\n    };\n";
}

/// \brief Finished all of the lexer definitions
void output_cplusplus::end_lexer_definitions() {
	// Add a final state to point to the end of the array
	m_StateToEntryOffset.push_back(m_LexerEntryPos);

	// Write out the rows table
	(*m_SourceFile) << "\nstatic const dfa::state_machine_compact_table<false>::entry* s_LexerStates[" << m_StateToEntryOffset.size()-1 << "] = {\n        ";

	// Write the actual rows
	bool first = true;
	for (vector<int>::iterator offset = m_StateToEntryOffset.begin(); offset != m_StateToEntryOffset.end()-1; offset++) {
		// Commas between entries
		if (!first) (*m_SourceFile) << ", ";

		// Entries point to a position in the state machine table
		(*m_SourceFile) << "s_LexerStateMachine + " << *offset;

		// No longer the first entry
		first = false;
	}

	(*m_SourceFile) << "\n    };\n";

	// Create a state machine
	(*m_SourceFile) << "\ntypedef dfa::state_machine_tables<wchar_t, dfa::hard_coded_symbol_table<wchar_t, 2> > lexer_state_machine;\n";
	(*m_SourceFile) << "static const lexer_state_machine s_StateMachine(s_SymbolMap, s_LexerStates, " << m_StateToEntryOffset.size()-1 << ");\n";

	// Create the lexer itself
	(*m_SourceFile) << "\ntypedef dfa::dfa_lexer_base<const lexer_state_machine&, 0, 0, false> lexer_definition;\n";
	(*m_SourceFile) << "static lexer_definition s_LexerDefinition(s_StateMachine, " << m_StateToEntryOffset.size()-1 << ", s_AcceptingStates);\n";

	// Finally, the lexer class itself
	(*m_HeaderFile) << "\npublic:\n";
	(*m_HeaderFile) << "    static const dfa::lexer lexer;\n";

	(*m_SourceFile) << "\nconst dfa::lexer " << get_identifier(m_ClassName) << "::lexer(&s_LexerDefinition, false);\n";

    // Add to the list of used class names
    m_UsedClassNames.insert("lexer");
}


// 				===================
//  			 Parser generation
// 				===================

/// \brief Starting to write out the definitions associated with the parser
void output_cplusplus::begin_parser_definitions() {
}

/// \brief Type of an action
typedef lr::parser_tables::action action;

/// \brief Writes out an action table
template<typename get_count> void write_action_table(string tableName, const lr::parser_tables::action* const* actionTable, const lr::parser_tables& tables, ostream& output) {
	// Count getter object
	get_count gc;

	// Start the table
	output << "static lr::parser_tables::action " << tableName << "_data[] = {";

	// Iterate through the states
	bool first = true;
	int count = 0;
	for (int state = 0; state < tables.count_states(); state++) {
		// Add the actions for this state
		int numActions = gc(tables, state);
        
        bool showingState = true;
        output << "\n\n    // State " << state << "\n    ";

		// Write out each action for this state
		for (int actionId = 0; actionId < numActions; actionId++) {
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
			output << "{ " << thisAction.m_Type << ", " << thisAction.m_NextState << ", " << thisAction.m_SymbolId << " }";

			// Move on
			first = false;
            showingState = false;
			count++;
		}
	}
	output << "\n};\n";
    
    // Output the final table
    output << "static lr::parser_tables::action* " << tableName << "[] = {";

    int pos = 0;
	count   = 0;
    first   = true;
    
	for (int state = 0; state < tables.count_states(); state++) {
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
        count++;
        first = false;
	}
    
    output << "\n};\n";
}

/// \brief Functor that returns the number of actions for a given terminal object
class count_terminal_actions {
public:
	int operator()(const lr::parser_tables& tables, int state) {
		return tables.action_counts()[state].m_NumTerms;
	}	
};

/// \brief Functor that returns the number of actions for a given non-terminal object
class count_nonterminal_actions {
public:
	int operator()(const lr::parser_tables& tables, int state) {
		return tables.action_counts()[state].m_NumNonterms;
	}	
};

/// \brief Supplies the parser tables generated by the compiler
void output_cplusplus::parser_tables(const lr::lalr_builder& builder, const lr::parser_tables& tables) {
    // True if we're at the first item
    bool    first;
    
    // Count used when writing tables
    int     count;
    
    // Need to include the parser tables file
    (*m_SourceFile) << "\n#include \"Lr/parser_tables.h\"\n";
    
	// Write out the terminal actions
    (*m_SourceFile) << "\n";
	write_action_table<count_terminal_actions>("s_TerminalActions", tables.terminal_actions(), tables, *m_SourceFile);
    
    // ... and the nonterminal actions
    (*m_SourceFile) << "\n";
	write_action_table<count_nonterminal_actions>("s_NonterminalActions", tables.nonterminal_actions(), tables, *m_SourceFile);
    
    // Write out the action counts
    (*m_SourceFile) << "\nstatic lr::parser_tables::action_count s_ActionCounts[] = {";
    
    first   = true;
    count   = 0;
    for (int stateId=0; stateId < tables.count_states(); stateId++) {
        // Comma
        if (!first) {
            (*m_SourceFile) << ", ";
        }

        // Newline
        if ((count%5) == 0) {
            (*m_SourceFile) << "\n    ";
        }
        
        // Write out the next item
        (*m_SourceFile) << "{ " << tables.action_counts()[stateId].m_NumTerms << ", " << tables.action_counts()[stateId].m_NumNonterms << " }";
        
        // Move on
        first = false;
        count++;
    }
    
    (*m_SourceFile) << "\n};\n";
    
    // Write out the end guard states
    (*m_SourceFile) << "\nstatic int s_EndGuardStates[] = {";
    
    first   = true;
    count   = 0;
    for (int stateId=0; stateId < tables.count_end_of_guards(); stateId++) {
        // Comma
        if (!first) {
            (*m_SourceFile) << ", ";
        }
        
        // Newline
        if ((count%20) == 0) {
            (*m_SourceFile) << "\n    ";
        }
        
        // Write out the next item
        (*m_SourceFile) << tables.end_of_guard_states()[stateId];
        
        // Move on
        first = false;
        count++;
    }
    
    (*m_SourceFile) << "\n};\n";

    // Write out the reduce rules
    (*m_SourceFile) << "\nstatic lr::parser_tables::reduce_rule s_ReduceRules[] = {";
    
    first   = true;
    count   = 0;
    for (int ruleId=0; ruleId < tables.count_reduce_rules(); ruleId++) {
        // Comma
        if (!first) {
            (*m_SourceFile) << ", ";
        }
        
        // Newline
        if ((count%5) == 0) {
            (*m_SourceFile) << "\n    ";
        }
        
        // Write out the next item
        const lr::parser_tables::reduce_rule& rule = tables.reduce_rules()[ruleId];
        (*m_SourceFile) << "{ " << rule.m_Identifier << ", " << rule.m_RuleId << ", " << rule.m_Length << " }";
        
        // Move on
        first = false;
        count++;
    }
    
    (*m_SourceFile) << "\n};\n";
    
    // Finally, the weak to strong equivalence table
    (*m_SourceFile) << "\nstatic lr::parser_tables::symbol_equivalent s_WeakToStrong[] = {";
    
    first   = true;
    count   = 0;
    for (int stateId=0; stateId < tables.count_weak_to_strong(); stateId++) {
        // Comma
        if (!first) {
            (*m_SourceFile) << ", ";
        }
        
        // Newline
        if ((count%10) == 0) {
            (*m_SourceFile) << "\n    ";
        }
        
        // Write out the next item
        (*m_SourceFile) << "{ " << tables.weak_to_strong()[stateId].m_OriginalSymbol << ", " << tables.weak_to_strong()[stateId].m_MappedTo << " }";
        
        // Move on
        first = false;
        count++;
    }
    
    (*m_SourceFile) << "\n};\n";
    
    // Generate the parser tables
    (*m_HeaderFile) << "\npublic:\n";
    (*m_HeaderFile) << "    static const lr::parser_tables lr_tables;\n";
    (*m_HeaderFile) << "    static const lr::simple_parser simple_parser;\n";
    
	(*m_SourceFile) << "\nconst lr::parser_tables " << get_identifier(m_ClassName) << "::lr_tables(" 
					<< tables.count_states() << ", " << tables.end_of_input() << ", " 
					<< tables.end_of_guard() 
					<< ", s_TerminalActions, s_NonterminalActions, s_ActionCounts, s_EndGuardStates, " 
					<< tables.count_end_of_guards() << ", " << tables.count_reduce_rules() << ", "
					<< "s_ReduceRules, " << tables.count_weak_to_strong() << ", "
					<< "s_WeakToStrong"
					<< ");\n";
	
	(*m_SourceFile) << "\nconst lr::simple_parser " << get_identifier(m_ClassName) << "::simple_parser(lr_tables);\n";

    // Add to the list of used class names
    m_UsedClassNames.insert("lr_tables");
    m_UsedClassNames.insert("simple_parser");
}

/// \brief Finished the parser definitions
void output_cplusplus::end_parser_definitions() {
}


// 				================
//  			 AST generation
// 				================

/// \brief Starting to write out the definitions associated with the AST
void output_cplusplus::begin_ast_definitions(const contextfree::grammar& grammar, const contextfree::terminal_dictionary& terminals) {
	// Remember the terminals and the grammar
	m_Grammar	= &grammar;
	m_Terminals	= &terminals;

	// Write out the AST base class
	(*m_HeaderFile) << "\npublic:\n";
	(*m_HeaderFile) << "    class syntax_node {\n";
	(*m_HeaderFile) << "    public:\n";
	(*m_HeaderFile) << "        virtual ~syntax_node();\n";
	(*m_HeaderFile) << "        virtual std::wstring to_string();\n";
	(*m_HeaderFile) << "        virtual dfa::position pos();\n";
	(*m_HeaderFile) << "        virtual dfa::position final_pos();\n";
	(*m_HeaderFile) << "    };\n";

	(*m_SourceFile) << "\n";
	(*m_SourceFile) << get_identifier(m_ClassName) << "::syntax_node::~syntax_node() { }\n";
	(*m_SourceFile) << "std::wstring " << get_identifier(m_ClassName) << "::syntax_node::to_string() { return L\"syntax_node\"; }\n";
	(*m_SourceFile) << "dfa::position " << get_identifier(m_ClassName) << "::syntax_node::pos() { return dfa::position(-1, -1, -1); }\n";
	(*m_SourceFile) << "dfa::position " << get_identifier(m_ClassName) << "::syntax_node::final_pos() { return dfa::position(-1, -1, -1); }\n";

    // Add to the list of used class names
    m_UsedClassNames.insert("syntax_node");

	// Clear the definitions
	if (m_NtForwardDeclarations)	delete m_NtForwardDeclarations;
	if (m_NtClassDefinitions)		delete m_NtClassDefinitions;

	m_NtForwardDeclarations	= new stringstream();
	m_NtClassDefinitions	= new stringstream();

	// Create a terminal definition item
	*m_NtForwardDeclarations << "\n    class terminal : public syntax_node {\n";
	*m_NtForwardDeclarations << "    private:\n";
	*m_NtForwardDeclarations << "        dfa::lexeme_container m_Lexeme;\n";
	*m_NtForwardDeclarations << "\n";
	*m_NtForwardDeclarations << "    public:\n";
	*m_NtForwardDeclarations << "        terminal(const dfa::lexeme_container& lexeme);\n";
	*m_NtForwardDeclarations << "\n";
	*m_NtForwardDeclarations << "        inline const dfa::lexeme_container& get_lexeme() const { return m_Lexeme; }\n";
	*m_NtForwardDeclarations << "\n";
	*m_NtForwardDeclarations << "        template<typename symbol_type> inline std::basic_string<symbol_type> content() const { return m_Lexeme->content<symbol_type>(); }\n";
	*m_NtForwardDeclarations << "\n";
	*m_NtForwardDeclarations << "        virtual dfa::position pos();\n";
	*m_NtForwardDeclarations << "\n";
	*m_NtForwardDeclarations << "        virtual dfa::position final_pos();\n";
	*m_NtForwardDeclarations << "    };\n";

	(*m_SourceFile) << "\n" << get_identifier(m_ClassName) << "::terminal::terminal(const dfa::lexeme_container& lexeme) : m_Lexeme(lexeme) { }\n";
	(*m_SourceFile) << "\ndfa::position " << get_identifier(m_ClassName) << "::terminal::pos() { return m_Lexeme->pos(); }\n";
	(*m_SourceFile) << "\ndfa::position " << get_identifier(m_ClassName) << "::terminal::final_pos() { return m_Lexeme->final_pos(); }\n";

	m_UsedClassNames.insert("terminal");
}

/// \brief Starting to write the AST definitions for a particular terminal symbol
void output_cplusplus::begin_ast_terminal(int itemIdentifier, const contextfree::item_container& item) {
	// Get the name for this nonterminal
	string name = name_for_nonterminal(itemIdentifier, item, *m_Grammar, *m_Terminals);
	m_CurrentNonterminal = name;

	// Write out a forward declaration for this item
	*m_NtForwardDeclarations << "\n    class " << name << ";\n";

	// Begin a class declaration for this item
	*m_NtClassDefinitions << "\n    class " << name << " : public terminal {\n";
	*m_NtClassDefinitions << "    public:\n";
	*m_NtClassDefinitions << "        " << name << "(const dfa::lexeme_container& lex) : terminal(lex) { }\n";
	*m_NtClassDefinitions << "    };\n";
}

/// \brief Finished writing the definitions for a terminal
void output_cplusplus::end_ast_terminal() {
}

/// \brief Starting to write the AST definitions for the specified nonterminal
void output_cplusplus::begin_ast_nonterminal(int identifier, const contextfree::item_container& item) {
	// Get the name for this nonterminal
	string ntName = name_for_nonterminal(identifier, item, *m_Grammar, *m_Terminals);
	m_CurrentNonterminal = ntName;

	// Write out a forward declaration for this item
	*m_NtForwardDeclarations << "\n    class " << ntName << ";\n";

	// Begin a class declaration for this item
	*m_NtClassDefinitions << "\n    class " << ntName << " : public syntax_node {\n";

	// No items are used for this nonterminal yet
	m_UsedNtItems.clear();
}

/// \brief Starting to write out a rule in the current nonterminal
void output_cplusplus::begin_ast_rule(int identifier) {
	// Start appending the private values for this class
	*m_NtClassDefinitions << "    private:\n";
	*m_NtClassDefinitions << "        // Rule " << identifier << "\n";

	// Set the rule identifier
	m_CurrentRuleId = identifier;

	// No items are used for the rule yet
	m_UsedRuleItems.clear();
	m_CurrentRuleNames.clear();
	m_CurrentRuleTypes.clear();
}

/// \brief Writes out an individual item in the current rule (a nonterminal)
void output_cplusplus::rule_item_nonterminal(int nonterminalId, const contextfree::item_container& item) {
	// Generate a name for this item
	// TODO: would be nice to be able to specify aliases in the grammar
	string baseName = name_for_nonterminal(nonterminalId, item, *m_Grammar, *m_Terminals);

	// Uniquify it
	string 	itemName;
	int 	variant = 0;
	for (;;) {
		// Next variant
		variant++;

		// Generate the name for this variant
		stringstream thisName;
		thisName << "m_" << baseName;
		if (variant > 1) {
			thisName << "_" << variant;
		}

		// Should be unique within the rule
		if (m_UsedRuleItems.find(thisName.str()) == m_UsedRuleItems.end()) {
			// This item hasn't been used in this rule before
			itemName = thisName.str();
			break;
		}
	}

	// Add to the definition if we haven't declared it in this nonterminal yet
	if (m_UsedNtItems.find(itemName) == m_UsedNtItems.end()) {
		*m_NtClassDefinitions << "        util::syntax_ptr<" << baseName << "> " << itemName << ";\n";
	}

	// Add to the items in the current rule
	m_CurrentRuleNames.push_back(itemName);
	m_CurrentRuleTypes.push_back(baseName);

	// Mark as used
	m_UsedRuleItems.insert(itemName);
	m_UsedNtItems.insert(itemName);
}

/// \brief Writes out an individual item in the current rule (a terminal)
///
/// Note the distinction between the item ID, which is part of the grammar, and the
/// symbol ID (which is part of the lexer and is the same as the value passed to 
/// terminal_symbol)
void output_cplusplus::rule_item_terminal(int terminalItemId, int terminalSymbolId, const contextfree::item_container& item) {
	// Generate a name for this item
	// TODO: would be nice to be able to specify aliases in the grammar
	string baseName = name_for_nonterminal(terminalItemId, item, *m_Grammar, *m_Terminals);

	// Uniquify it
	string 	itemName;
	int 	variant = 0;
	for (;;) {
		// Next variant
		variant++;

		// Generate the name for this variant
		stringstream thisName;
		thisName << "m_" << baseName;
		if (variant > 1) {
			thisName << "_" << variant;
		}

		// Should be unique within the rule
		if (m_UsedRuleItems.find(thisName.str()) == m_UsedRuleItems.end()) {
			// This item hasn't been used in this rule before
			itemName = thisName.str();
			break;
		}
	}

	// Add to the definition if we haven't declared it in this nonterminal yet
	if (m_UsedNtItems.find(itemName) == m_UsedNtItems.end()) {
		*m_NtClassDefinitions << "        util::syntax_ptr<" << baseName << "> " << itemName << ";\n";
	}

	// Add to the items in the current rule
	m_CurrentRuleNames.push_back(itemName);
	m_CurrentRuleTypes.push_back(baseName);

	// Mark as used
	m_UsedRuleItems.insert(itemName);
	m_UsedNtItems.insert(itemName);	
}

/// \brief Finished writing out 
void output_cplusplus::end_ast_rule() {
	// Generate a constructor for this rule
	*m_NtClassDefinitions << "\n    public:\n";

	// Write out the declaration and parameters
	bool first = true;

	// Write the header...
	*m_NtClassDefinitions << "        " << m_CurrentNonterminal << "(";

	// ... and the source file
	*m_SourceFile << "\n" << get_identifier(m_ClassName) << "::" << m_CurrentNonterminal << "::" << m_CurrentNonterminal << "(";

	for (size_t index = 0; index < m_CurrentRuleNames.size(); index++) {
		// Comma to seperate the items
		if (!first) {
			*m_NtClassDefinitions << ", ";
			*m_SourceFile << ", ";
		}

		// Write out this parameter (name it after the type name and the index)
		*m_NtClassDefinitions << m_CurrentRuleTypes[index] << "* " << m_CurrentRuleTypes[index][0] << "_" << index;
		*m_SourceFile << m_CurrentRuleTypes[index] << "* " << m_CurrentRuleTypes[index][0] << "_" << index;

		// No longer first
		first = false;
	}
	*m_NtClassDefinitions << ");\n";

	// Write out the constructor declaration
	first = true;
	*m_SourceFile << ")";

	for (size_t index = 0; index < m_CurrentRuleNames.size(); index++) {
		// Comma to seperate the items
		if (!first) {
			*m_SourceFile << "\n, ";
		} else {
			*m_SourceFile << "\n: ";
		}

		// Fill in the appropriate field
		*m_SourceFile << m_CurrentRuleNames[index] << "(" << m_CurrentRuleTypes[index][0] << "_" << index << ")";

		// No longer first
		first = false;
	}

	*m_SourceFile << "{\n";
	*m_SourceFile << "}\n";

	// TODO: blank out the other fields (hrm, use magic pointer type that does the right thing? Suitable place for auto_ptr?)

	// Extra newline to space things out
	*m_NtClassDefinitions << "\n";
}

/// \brief Finished writing the definitions for a nonterminal
void output_cplusplus::end_ast_nonterminal() {
	// Destructor: nothing to do here, as the syntax_ptr class will handle freeing everything up
	*m_NtClassDefinitions << "        virtual ~" << m_CurrentNonterminal << "();\n";
	*m_SourceFile << "\n" << get_identifier(m_ClassName) << "::" << m_CurrentNonterminal << "::~" << m_CurrentNonterminal << "() { }\n";

	// TODO: write out the accessors for the various items

	// End the class definition for this nonterminal
	*m_NtClassDefinitions << "    };\n";
}

/// \brief Finished writing out the AST information
void output_cplusplus::end_ast_definitions() {
	// Output the forward declarations
	(*m_HeaderFile) << m_NtForwardDeclarations->str();

	// ... then the class definitions
	(*m_HeaderFile) << m_NtClassDefinitions->str();
}
