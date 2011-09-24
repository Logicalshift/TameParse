//
//  boost_console.cpp
//  TameParse
//
//  Created by Andrew Hunter on 24/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <string>
#include <vector>

#include "TameParse/version.h"

#include "boost_console.h"
#include "boost/program_options.hpp"

using namespace std;
using namespace compiler;
using namespace tameparse;
namespace po = boost::program_options;

 // \brief Copies this console
boost_console::boost_console(const boost_console& bc)
: std_console(L"")
, m_VarMap(bc.m_VarMap) {
}

/// \brief Constructor
boost_console::boost_console(int argc, const char** argv) 
: std_console(L"") {
	// Declare the options supported by the tameparse utility
	po::options_description mainOptions("Parser generation options");

	mainOptions.add_options()
		("input-file,i",		po::value<string>(),			"specifies the name of the input file")
		("output-file,o",		po::value<string>(),			"specifies the base name for the output file")
		("include-path,I",		po::value< vector<string> >(),	"sets the path to search for included files")
		("target-language,t",	po::value<string>(),			"specifies the target language the parser will be generated in");

	po::options_description infoOptions("Information");
    
    infoOptions.add_options()
        ("help,h", 												"display help message")
        ("verbose,v",											"display verbose messages")
        ("silent",												"suppress informational messages")
        ("version",												"display version information")
        ("warranty",											"display warranty information")
        ("license",												"display license information");

	// Positional options
	po::positional_options_description positional;

	positional.add("input-file", 1);
	positional.add("output-file", 1);
    
    // Command line options
    po::options_description cmdLine;
    cmdLine.add(mainOptions).add(infoOptions);

	// Store the options
    try {
        po::store(po::command_line_parser(argc, argv).options(cmdLine).positional(positional).run(), m_VarMap);
        po::notify(m_VarMap);
    } catch (po::error e) {
        cerr << e.what() << endl << endl;
		cout << mainOptions << endl << infoOptions << endl;
        ::exit(1);
    }

	// Display help if needed
    bool doneSomething = false;
    
    if (m_VarMap.count("version") || m_VarMap.count("warranty") || m_VarMap.count("license")) {
        cout << "TameParse version " << version::version_string;
        if (m_VarMap.count("license") == 0) {
            cout << endl << version::copyright_string << " " << version::contact_string;
        }
        cout << endl << endl;
        doneSomething = true;
    }
    
    if (m_VarMap.count("warranty")) {
        cout << version::warranty_string << endl << endl;
        doneSomething = true;
    }
    
    if (m_VarMap.count("license")) {
        cout << version::license_string << endl << endl;
        doneSomething = true;
    }
    
	if (m_VarMap.count("help")) {
		cout << mainOptions << endl << infoOptions << endl;
	    doneSomething = true;
	}

	// Also display help if no input file is displayed
	if (!doneSomething && m_VarMap.count("input-file") == 0) {
		cerr << argv[0] << ": no input files" << endl << endl;
		cout << mainOptions << endl << infoOptions << endl;
	}
    
    // Set the value of the input file string
    m_InputFile = get_option(L"input-file");
}

/// \brief Returns true if the options are valid and the parser can start
bool boost_console::can_start() const {
	// Nothing to do if no input file is specified
	if (m_VarMap.count("input-file") == 0) {
		return false;
	}

	// Everything looks to be in order
    return true;
}

/// \brief Clones the console
console* boost_console::clone() const {
	return new boost_console(*this);
}

/// \brief Reports an error to the console
void boost_console::report_error(const compiler::error& error) {
	// Pass to the standard console
	std_console::report_error(error);
}

/// \brief Retrieves the value of the option with the specified name.
///
/// If the option is not set, then this should return an empty string
std::wstring boost_console::get_option(const std::wstring& name) const {
	// Convert to a standard string
	string asciiName;

	for (size_t x=0; x<name.size(); x++) {
		asciiName += (char) name[x];
	}

	// Return the empty string if the option is not present
	if (m_VarMap.count(asciiName) == 0) {
		return L"";
	}

	// Try to get the value as a string
	try {
		// Get as an ascii string
		string asciiValue = m_VarMap[asciiName].as<string>();

		// Convert to a wstring
		wstring value;
		for (size_t x = 0; x < asciiValue.size(); x++) {
			value += (wchar_t) asciiValue[x];
		}
        
        // Value is '1' if the value is empty but the option is present
        if (value.empty()) {
            value += L'1';
        }

		return value;
	} catch (boost::bad_any_cast) {
		// The option is present but doesn't have a string value
		// Assume it's a boolean on/off option and return the name
		return name;
	}
}

/// \brief The name of the initial input file
const std::wstring& boost_console::input_file() const {
	return m_InputFile;
}
