//
//  boost_console.h
//  TameParse
//
//  Created by Andrew Hunter on 24/09/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#ifndef TameParse_boost_console_h
#define TameParse_boost_console_h

#include <string>

#include "TameParse/Compiler/std_console.h"
#include "boost/program_options.hpp"

///
/// \brief Console class that reads settings using the boost program_options classes
///
/// This is the default console for the tameparse utility program.
///
class boost_console : public compiler::std_console {
private:
    /// \brief Boost options object
    typedef boost::program_options::variables_map variables_map;

    /// \brief The variables map for this object
    variables_map m_VarMap;
    
    /// \brief The name of the input file
    std::wstring m_InputFile;

protected:
    // \brief Copies this console
    boost_console(const boost_console& bc);

public:
    /// \brief Constructor
    boost_console(int argc, const char** argv);

    /// \brief Clones the console
    virtual console* clone() const;

    /// \brief Reports an error to the console
    virtual void report_error(const compiler::error& error);

    /// \brief Returns true if the options are valid and the parser can start
    virtual bool can_start() const;

public:
    /// \brief Converts a wstring filename to whatever is the preferred format for the current system
    virtual std::string convert_filename(const std::wstring& filename);

    /// \brief Given a pathname, returns the 'real', absolute pathname
    ///
    /// The default implementation just returns the current path
    virtual std::wstring real_path(const std::wstring& pathname);

    /// \brief Splits a path into its components
    ///
    /// The default implementation assumes UNIX-style paths.
    virtual std::vector<std::wstring> split_path(const std::wstring& pathname);
    
    /// \brief Opens a text file with the specified name for reading
    ///
    /// The caller should delete the stream once it has finished with it. Streams are generally expected to contain UTF-8
    /// data, so console classes should usually open streams in binary mode.
    virtual std::istream* open_file(const std::wstring& filename);
    
    /// \brief Opens an output file with the specified name for writing binary data
    ///
    /// The caller should delete the stream once it has finished writing to it. The parser primarily writes binary files
    /// to ensure that the output is consistent between locales.
    ///
    /// (This is an annoying compromise added to deal with C++'s completely useless support for locales and unicode)
    virtual std::ostream* open_binary_file_for_writing(const std::wstring& filename);
    
public:
    /// \brief Retrieves the value of the option with the specified name.
    ///
    /// If the option is not set, then this should return an empty string
    virtual std::wstring get_option(const std::wstring& name) const;

    /// \brief Returns a list of values for a particular option
    ///
    /// For some options it is possible to specify more than one value: in this
    /// case, this will return all of the possible values. This can also be used
    /// to retrieve the values of options that can have an empty value.
    virtual std::vector<std::wstring> get_option_list(const std::wstring& name);
    
    /// \brief The name of the initial input file
    virtual const std::wstring& input_file() const;
};

#endif
