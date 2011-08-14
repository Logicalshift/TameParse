//
//  std_console.cpp
//  Parse
//
//  Created by Andrew Hunter on 06/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <locale>
#include <fstream>
#include <sstream>

#include "std_console.h"

using namespace std;
using namespace compiler;

/// \brief Creates a standard console with the specified input filename
std_console::std_console(const std::wstring& inputFilename)
: m_InputFilename(inputFilename) {
}

/// \brief Creates a copy of this console
console* std_console::clone() const {
    return new std_console(m_InputFilename);
}

/// \brief Reports an error to the console
void std_console::report_error(const error& error) {
    
}

/// \brief Retrieves a stream where log messages can be sent to (these are generally always displayed, but may be
/// suppressed if the console has a 'silent' mode)
std::wostream& std_console::message_stream() {
    // Output messages to wcerr by default
    return wcerr;
}

/// \brief Retrieves a stream where verbose messages can be sent to (these are generally not displayed unless the
/// console is configured to)
std::wostream& std_console::verbose_stream() {
    // Output verbose messages to wcerr by default
    return wcerr;
}

/// \brief Retrieves the value of the option with the specified name.
///
/// If the option is not set, then this should return an empty string
const std::wstring& std_console::get_option(const std::wstring& name) const {
    static std::wstring emptyString;
    return emptyString;
}

/// \brief The name of the initial input file
const std::wstring& std_console::input_file() const {
    return m_InputFilename;
}

/// \brief Opens a file with the specified name for reading
///
/// The caller should delete the stream once it has finished with it
std::wistream* std_console::open_file(const std::wstring& filename) {
    // Convert the filename to one we can pass to open
    string latin1Filename = convert_filename(filename);
    
    // Open using a wfstream and the current locale
    // TODO: make this work with UTF-8!
    // Mrgh, there's no simple or standard way to do this. There's no commonly accepted library way to do this, either.
    // Welcome to the 1980s.
    wfstream    readFile;

    readFile.open(latin1Filename.c_str(), fstream::in);
    
    // Return NULL if the file failed to open
    if (readFile.fail()) {
        return NULL;
    }
    
    // Read the file into a wstringstream
    const size_t    blockSize   = 4096;
    wstringstream*  target      = new wstringstream();
    wchar_t*        data        = new wchar_t[blockSize];
    
    while (!readFile.fail() && !readFile.eof()) {
        // Read the next block from the file
        readFile.read(data, blockSize);
        
        // Copy into the target
        target->write(data, readFile.gcount());
    }
    
    // Reset the target 
    target->seekg(ios_base::beg);
    
    // Return as the result
    return target;
}

/// \brief Opens an output file with the specified name for writing binary data
///
/// The caller should delete the stream once it has finished writing to it
std::ostream* std_console::open_binary_file_for_writing(const std::wstring& filename) {
    // Convert the filename to one we can pass to open
    string latin1Filename = convert_filename(filename);

    // Create the resulting stream
    fstream* result = new fstream();
    
    result->open(latin1Filename.c_str(), fstream::out | fstream::binary | fstream::trunc);
    
    // Return as the result
    return result;
}
