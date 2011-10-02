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
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"

using namespace std;
using namespace compiler;
using namespace tameparse;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

 // \brief Copies this console
boost_console::boost_console(const boost_console& bc)
: std_console(L"")
, m_VarMap(bc.m_VarMap) {
}

/// \brief Constructor
boost_console::boost_console(int argc, const char** argv) 
: std_console(L"") {
	// Declare the options supported by the tameparse utility
	po::options_description inputOptions("Input options");

	inputOptions.add_options()
		("input-file,i",		po::value<string>(),			"specifies the name of the input file.")
		("include-path,I",		po::value< vector<string> >(),	"sets the path to search for included files.");

	po::options_description outputOptions("Output options");
	
	outputOptions.add_options()
		("output-file,o",		po::value<string>(),			"specifies the base name for the output file. For languages that need to generate multiple files (for example, C++), the appropriate extensions will be added to this filename. If this is not specified, the output filenames will be derived from the input file.")
		("output-language,T",	po::value<string>(),			"specifies the output language the parser will be generated in.")
		("start-symbol,S",		po::value< vector<string> >(),	"specifies the name of the start symbol (overriding anything defined in the parser block of the input file)")
		("compile-language,L",	po::value<string>(),			"specifies the name of the language block to compile (overriding anything defined in the parser block of the input file)")
		("class-name,C",		po::value<string>(),			"specifies the name of the class to generate (overriding anything defined in the parser block of the input file)")
		("namespace-name,N",	po::value<string>(),			"specifies the namespace to put the target class into.")
		("test",												"specifies that no output should be generated. This tool will instead try to read from stdin and indicate whether or not it can be accepted.")
        ("show-parser",                                         "writes the generated parser to standard out");

	po::options_description infoOptions("Information");
    
    infoOptions.add_options()
        ("help,h", 												"display help message.")
        ("verbose,v",											"display verbose messages.")
        ("silent",												"suppress informational messages.")
        ("version",												"display version information.")
        ("warranty",											"display warranty information.")
        ("license",												"display license information.");
    
    po::options_description errorOptions("Error reporting");
    
    errorOptions.add_options()
        ("suppress-warnings",                                   "do not display any warning messages.")
        ("show-error-codes",                                    "display error codes alongside the error messages.")
        ("show-conflict-details",                               "show details about the context that conflicts occur in.")
        ("allow-reduce-conflicts",                              "reduce/reduce conflicts will produce a warning instead of an error.")
        ("no-conflicts",                                        "all parser conflicts count as an error and not a warning.");

	// Positional options
	po::positional_options_description positional;

	positional.add("input-file", 1);
	positional.add("output-file", 1);
    
    // Command line options
    po::options_description cmdLine;
    cmdLine.add(inputOptions).add(outputOptions).add(infoOptions).add(errorOptions);

	// Store the options
    try {
        po::store(po::command_line_parser(argc, argv).options(cmdLine).positional(positional).run(), m_VarMap);
        po::notify(m_VarMap);
    } catch (po::error e) {
        cerr << e.what() << endl << endl;
		cout << inputOptions << endl << outputOptions << endl << errorOptions << endl << infoOptions << endl;
        ::exit(1);
    }

	// Display help if needed
    bool doneSomething = false;
    
    if (m_VarMap.count("version") || m_VarMap.count("warranty") || m_VarMap.count("license") || m_VarMap.count("help")) {
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
		cout << inputOptions << endl << outputOptions << endl << errorOptions << endl << infoOptions << endl;
	    doneSomething = true;
	}

	// Also display help if no input file is displayed
	if (!doneSomething && m_VarMap.count("input-file") == 0) {
		cerr << argv[0] << ": no input files" << endl << endl;
		cout << inputOptions << endl << outputOptions << endl << errorOptions << endl << infoOptions << endl;
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

/// \brief Simple conversion from a string to wstring
static wstring convert(string asciiValue) {
	// Convert to a wstring by assuming that the string is latin-1
	// TODO: could take account of the locale here, somehow
	wstring value;
	for (size_t x = 0; x < asciiValue.size(); x++) {
		value += (wchar_t) asciiValue[x];
	}

	return value;
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
		// Convert to a wstring
		wstring value = convert(m_VarMap[asciiName].as<string>());
        
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

/// \brief Returns a list of values for a particular option
///
/// For some options it is possible to specify more than one value: in this
/// case, this will return all of the possible values. This can also be used
/// to retrieve the values of options that can have an empty value.
std::vector<std::wstring> boost_console::get_option_list(const std::wstring& name) {
	// Convert to a standard string
	string asciiName;

	for (size_t x=0; x<name.size(); x++) {
		asciiName += (char) name[x];
	}

	// Return the empty string if the option is not present
	if (m_VarMap.count(asciiName) == 0) {
		return vector<wstring>();
	}

	// Try to get the value as a string
	try {
		// Get as an ascii string
		vector<string> asciiValue = m_VarMap[asciiName].as< vector<string> >();

		// Convert to a wstring
		vector<wstring> value;
		for (size_t x = 0; x < asciiValue.size(); x++) {
			value.push_back(convert(asciiValue[x]));
		}

		return value;
	} catch (boost::bad_any_cast) {
		// Option is not a vector of string: try getting as a simple value
		wstring simpleValue = get_option(name);

		// Place in the result if it's non-empty
		vector<wstring> res;
		if (!simpleValue.empty()) {
			res.push_back(simpleValue);
		}
		return res;
	}	
}

/// \brief The name of the initial input file
const std::wstring& boost_console::input_file() const {
	return m_InputFile;
}

/// \brief Converts a wstring filename to whatever is the preferred format for the current system
std::string boost_console::convert_filename(const std::wstring& filename) {
	fs::wpath path(filename);
	return path.generic_string();
}

/// \brief Given a pathname, returns the 'real', absolute pathname
///
/// The default implementation just returns the current path
std::wstring boost_console::real_path(const std::wstring& pathname) {
	fs::wpath path(pathname);
	fs::wpath absolute;
    
    try {
        // Try to get the absolute path for this file
        absolute = fs::absolute(path);
        return absolute.generic_wstring();
    } catch (fs::filesystem_error e) {
        // Report as an error
        report_error(error(error::sev_error, pathname, L"CANT_GET_ABSOLUTE_PATH", L"Unable to get absolute path for file", dfa::position(-1, -1, -1)));
        
        // Return the path unchanged
        return pathname;
    }
}

/// \brief Splits a path into its components
///
/// The default implementation assumes UNIX-style paths.
std::vector<std::wstring> boost_console::split_path(const std::wstring& pathname) {
	fs::wpath path(pathname);
	vector<wstring> res;

	for (fs::wpath::iterator component = path.begin(); component != path.end(); component++) {
		res.push_back(component->generic_wstring());
	}

	return res;
}

/// \brief Opens a text file with the specified name for reading
///
/// The caller should delete the stream once it has finished with it. Streams are generally expected to contain UTF-8
/// data, so console classes should usually open streams in binary mode.
std::istream* boost_console::open_file(const std::wstring& filename) {
	// Get the path
	fs::wpath path(filename);

	// Create the stream
    fs::fstream* readFile = new fs::fstream();

    readFile->open(path, fstream::in | fstream::binary);
    
    // Return NULL if the file failed to open
    if (readFile->fail()) {
        delete readFile;
        return NULL;
    }
    
    // Return the stream
    return readFile;
}

/// \brief Opens an output file with the specified name for writing binary data
///
/// The caller should delete the stream once it has finished writing to it. The parser primarily writes binary files
/// to ensure that the output is consistent between locales.
///
/// (This is an annoying compromise added to deal with C++'s completely useless support for locales and unicode)
std::ostream* boost_console::open_binary_file_for_writing(const std::wstring& filename) {
    // Get the path
	fs::wpath path(filename);

    // Create the resulting stream
    fs::fstream* result = new fs::fstream();
    
    result->open(path, fstream::out | fstream::binary | fstream::trunc);
    
    // Return as the result
    return result;
}
