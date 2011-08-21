//
//  std_console.h
//  Parse
//
//  Created by Andrew Hunter on 06/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _COMPILER_STD_CONSOLE_H
#define _COMPILER_STD_CONSOLE_H

#include "Compiler/console.h"

namespace compiler {
    ///
    /// \brief Most basic console implementation using the standard C++ I/O routines
    ///
    class std_console : public console {
    private:
        /// \brief A filename that this console should read from
        std::wstring m_InputFilename;
        
        /// \brief The exit code to use
        int m_ExitCode;
        
    public:
        /// \brief Creates a standard console with the specified input filename
        std_console(const std::wstring& inputFilename);
        
        /// \brief Creates a copy of this console
        virtual console* clone() const;
        
    public:
        /// \brief Reports an error to the console
        virtual void report_error(const error& error);
        
        /// \brief Retrieves a stream where log messages can be sent to (these are generally always displayed, but may be
        /// suppressed if the console has a 'silent' mode)
        virtual std::wostream& message_stream();
        
        /// \brief Retrieves a stream where verbose messages can be sent to (these are generally not displayed unless the
        /// console is configured to)
        virtual std::wostream& verbose_stream();
        
        /// \brief Returns the exit code that the application should use (if there's an error, this will be non-zero)
        virtual int exit_code();
        
    public:
        /// \brief Retrieves the value of the option with the specified name.
        ///
        /// If the option is not set, then this should return an empty string
        virtual const std::wstring& get_option(const std::wstring& name) const;
        
        /// \brief The name of the initial input file
        virtual const std::wstring& input_file() const;
        
    public:
        /// \brief Opens a file with the specified name for reading
        ///
        /// The caller should delete the stream once it has finished with it
        virtual std::wistream* open_file(const std::wstring& filename);
        
        /// \brief Opens an output file with the specified name for writing binary data
        ///
        /// The caller should delete the stream once it has finished writing to it
        virtual std::ostream* open_binary_file_for_writing(const std::wstring& filename);
    };
}

#endif
