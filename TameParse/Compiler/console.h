//
//  console.h
//  Parse
//
//  Created by Andrew Hunter on 06/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _COMPILER_CONSOLE_H
#define _COMPILER_CONSOLE_H

#include <iostream>
#include <string>
#include <vector>

#include "TameParse/Compiler/error.h"
#include "TameParse/Util/container.h"

namespace compiler {
    ///
    /// \brief Class representing a 'compiler console'
    ///
    /// This is used by compiler stages to supply messages to the user, perform file I/O and retrieve command-line options.
    /// This is an abstract class: subclasses should implement the functions here to provide a suitable I/O layer. Basic I/O
    /// is provided by the std_console class.
    ///
    class console {
    public:
        
        /// \brief Orders this console with respect to another (default operation is just a pointer compare)
        virtual bool operator<(const console& compareTo) const;

        /// \brief Compares this console to another (default operation is to compare pointers)
        virtual bool operator==(const console& compareTo) const;

        /// \brief Comparison function so we can use consoles in a container
        inline static bool compare(const console* a, const console* b) {
            if (a == b)     return true;
            if (a == NULL)  return true;
            if (b == NULL)  return false;
            
            return a->operator<(*b);
        }

        /// \brief Creates a copy of this console
        virtual console* clone() const = 0;
        
    public:
        /// \brief Reports an error to the console
        virtual void report_error(const error& error) = 0;
        
        /// \brief Returns the exit code that the application should use (if there's an error, this will be non-zero)
        virtual int exit_code() = 0;
        
        /// \brief Retrieves a stream where log messages can be sent to (these are generally always displayed, but may be
        /// suppressed if the console has a 'silent' mode)
        virtual std::wostream& message_stream() = 0;
        
        /// \brief Retrieves a stream where verbose messages can be sent to (these are generally not displayed unless the
        /// console is configured to)
        virtual std::wostream& verbose_stream() = 0;
        
    public:
        /// \brief Retrieves the value of the option with the specified name.
        ///
        /// If the option is not set, then this should return an empty string
        virtual std::wstring get_option(const std::wstring& name) const = 0;

        /// \brief Returns a list of values for a particular option
        ///
        /// For some options it is possible to specify more than one value: in this
        /// case, this will return all of the possible values. This can also be used
        /// to retrieve the values of options that can have an empty value.
        ///
        /// The default implementation just calls get_option() and creates a list of
        /// size one if it contains a non-empty string. Subclasses should generally
        /// override this behaviour if they truly support empty options or options
        /// with multiple values
        virtual std::vector<std::wstring> get_option_list(const std::wstring& name);
        
        /// \brief The name of the initial input file
        virtual const std::wstring& input_file() const = 0;

        /// \brief Returns true if the options are valid and the parser can start
        virtual bool can_start() const;
        
    public:
        /// \brief Converts a wstring filename to whatever is the preferred format for the current system
        virtual std::string convert_filename(const std::wstring& filename);

        /// \brief Given a pathname, returns the 'real' pathname, that is, the unique
        /// absolute path for the supplied relative path.
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
        virtual std::istream* open_file(const std::wstring& filename) = 0;
        
        /// \brief Opens an output file with the specified name for writing binary data
        ///
        /// The caller should delete the stream once it has finished writing to it. The parser primarily writes binary files
        /// to ensure that the output is consistent between locales.
        ///
        /// (This is an annoying compromise added to deal with C++'s completely useless support for locales and unicode)
        virtual std::ostream* open_binary_file_for_writing(const std::wstring& filename) = 0;
    };
    
    // Console container class
    typedef util::container<console> console_container;
}

#endif
