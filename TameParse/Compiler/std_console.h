//
//  std_console.h
//  Parse
//
//  Created by Andrew Hunter on 06/08/2011.
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

#ifndef _COMPILER_STD_CONSOLE_H
#define _COMPILER_STD_CONSOLE_H

#include "TameParse/Compiler/console.h"

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
        virtual std::wstring get_option(const std::wstring& name) const;
        
        /// \brief The name of the initial input file
        virtual const std::wstring& input_file() const;
        
    public:
        /// \brief Opens a file with the specified name for reading
        ///
        /// The caller should delete the stream once it has finished with it. Streams are generally expected to contain UTF-8
        /// data, so console classes should usually open streams in binary mode.
        virtual std::istream* open_file(const std::wstring& filename);
        
        /// \brief Opens an output file with the specified name for writing binary data
        ///
        /// The caller should delete the stream once it has finished writing to it
        virtual std::ostream* open_binary_file_for_writing(const std::wstring& filename);
    };
}

#endif
