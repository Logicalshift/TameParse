//
//  std_console.cpp
//  Parse
//
//  Created by Andrew Hunter on 06/08/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#include <locale>
#include <fstream>
#include <sstream>

#include "TameParse/Compiler/std_console.h"

using namespace std;
using namespace compiler;

/// \brief Null stream class used to suppress output
class nullstream : public wostream {
private:
    /// \brief Null stream buffer
    class nullbuf : public wstreambuf {
        int overflow(int c) { return traits_type::not_eof(c); }
    } m_NullBuf;
    
public:
    nullstream() : wostream(&m_NullBuf) { }
};

/// \brief Static null stream object
static nullstream s_NullStream;

/// \brief Creates a standard console with the specified input filename
std_console::std_console(const std::wstring& inputFilename)
: m_InputFilename(inputFilename)
, m_ExitCode(0) {
}

/// \brief Creates a copy of this console
console* std_console::clone() const {
    std_console* res = new std_console(m_InputFilename);
    res->m_ExitCode = m_ExitCode;
    return res;
}

/// \brief Reports an error to the console
void std_console::report_error(const error& error) {
    // Errors go to wcerr by default
    wostream* out = &wcerr;
    
    // Suppress warnings if the options are set
    if (error.sev() != error::sev_detail && error.sev() <= error::sev_warning && !get_option(L"suppress-warnings").empty()) {
        // TODO: suppress sev_detail messages if the last non-detail error was a warning
        return;
    }
    
    // Use the verbose stream if this is not a warning or error message
    if (error.sev() < error::sev_detail) {
        out = &verbose_stream();
    }
    
    // Set the exit code if the error is severe enough
    if (error.sev() >= error::sev_error) {
        m_ExitCode = error.sev();
    }
    
    // Write a formatted error message, beginning with the file that suffered the failure
    if (!error.filename().empty()) {
        *out << error.filename() << L":";
    } else {
        *out << L":";
    }
    
    // Write out the line number if it's >= 0
    if (error.pos().line() >= 0) {
        *out << error.pos().line()+1 << L":";
    } else {
        *out << L":";
    }

    if (error.pos().column() >= 0) {
        *out << error.pos().column()+1 << L":";
    } else {
        *out << L":";
    }
    
    // Write out the error severity
    switch (error.sev()) {
        case error::sev_info:
        case error::sev_message:
            break;
            
        case error::sev_detail:
            *out << L"    ";
            break;
            
        case error::sev_warning:
            *out << L" warning:";
            break;
            
        case error::sev_error:
            *out << L" error:";
            break;
            
        case error::sev_fatal:
            *out << L" failure:";
            break;
            
        case error::sev_bug:
            *out << L" compiler failure:";
            break;
            
        default:
            *out << L" unknown:";
            break;
    }
    
    // If the options are set to add the error code then do so
    if (!get_option(L"show-error-codes").empty()) {
        *out << L" [" << error.identifier() << L"]";
    }

    // Write out the error description
    *out << L" " << error.description() << endl;
}

/// \brief Retrieves a stream where log messages can be sent to (these are generally always displayed, but may be
/// suppressed if the console has a 'silent' mode)
std::wostream& std_console::message_stream() {
    // Output to null if silent is set
    if (!get_option(L"silent").empty()) {
        return s_NullStream;
    }
    
    // Output messages to wcout by default
    return wcout;
}

/// \brief Retrieves a stream where verbose messages can be sent to (these are generally not displayed unless the
/// console is configured to)
std::wostream& std_console::verbose_stream() {
    // Output to null if verbose is not set
    if (get_option(L"verbose").empty()) {
        return s_NullStream;
    }
    
    // Output verbose messages to wcout by default
    return wcout;
}

/// \brief Retrieves the value of the option with the specified name.
///
/// If the option is not set, then this should return an empty string
std::wstring std_console::get_option(const std::wstring& name) const {
    // When this class is not overridden, we pretend to be in verbose mode
    if (name == L"verbose") return L"verbose";
    
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
std::istream* std_console::open_file(const std::wstring& filename) {
    // Convert the filename to one we can pass to open
    string latin1Filename = convert_filename(filename);
    
    // Open using a wfstream and the current locale
    // TODO: make this work with UTF-8!
    // This will use whatever the standard is for the C++ library that is in use
    fstream* readFile = new fstream();

    readFile->open(latin1Filename.c_str(), fstream::in | fstream::binary);
    
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

/// \brief Returns the exit code that the application should use (if there's an error, this will be non-zero)
int std_console::exit_code() {
    return m_ExitCode;
}

