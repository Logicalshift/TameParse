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

#include "Compiler/error.h"
#include "Util/container.h"

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
        /// \brief Creates a copy of this console
        virtual console* clone() const;
        
        /// \brief Orders this console with respect to another (default operation is just a pointer compare)
        virtual bool operator<(const console& compareTo) const;

        /// \brief Comparison function so we can use consoles in a container
        inline static bool compare(const console* a, const console* b) {
            if (a == b)     return true;
            if (a == NULL)  return true;
            if (b == NULL)  return false;
            
            return a->operator<(*b);
        }
        
    public:
        /// \brief Reports an error to the console
        virtual void report_error(const error& error) = 0;
        
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
        virtual std::wstring& get_option(const std::wstring& name) = 0;
        
        /// \brief The name of the initial input file
        virtual std::wstring& input_file() = 0;
        
    public:
        /// \brief Opens a file with the specified name for reading
        ///
        /// The caller should delete the stream once it has finished with it
        virtual std::wistream* open_file(const std::wstring& filename) = 0;
        
        /// \brief Opens an output file with the specified name for writing
        ///
        /// The caller should delete the stream once it has finished writing to it
        virtual std::wostream* open_file_for_writing(const std::wstring& filename) = 0;
    };
    
    typedef util::container<console> console_container;
}

#endif
