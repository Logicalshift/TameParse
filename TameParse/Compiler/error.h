//
//  error.h
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

#ifndef _COMPILER_ERROR_H
#define _COMPILER_ERROR_H

#include <string>

#include "TameParse/Dfa/position.h"

namespace compiler {
    // TODO: add a way to add parameters to the error so we can use the identifier to internationalise the parser
    
    ///
    /// \brief Class representing a compiler error
    ///
    class error {
    public:
        /// \brief Severity of a particular error
        enum severity {
            /// \brief Just a message
            sev_message,
            
            /// \brief Some information about a particular file
            sev_info,
            
            /// \brief Detail about the previous warning or error
            sev_detail,
            
            /// \brief A warning about something that could be wrong with the file (but which does not prevent the generator from producing results)
            sev_warning,
            
            /// \brief An error that prevents the generator from producing results
            sev_error,
            
            /// \brief A more serious error that prevents the generator from proceeding
            sev_fatal,
            
            /// \brief An error that should never occur and indicates that the generator has malfunctioned in some way
            sev_bug
        } ;
        
        /// \brief Position within a file
        typedef dfa::position position;
        
    private:
        /// \brief An identifier for this error
        std::wstring m_Identifier;
        
        /// \brief The standard description for this error
        std::wstring m_Description;
        
        /// \brief The name of the file where this error occurred
        std::wstring m_Filename;
        
        /// \brief The position in the file where this error should appear
        position m_Position;
        
        /// \brief The severity of this error
        severity m_Severity;
        
    public:
        /// \brief Creates a new error
        error(severity severity, const std::wstring& filename, const std::wstring& identifier, const std::wstring& description, const position& pos);
        
        /// \brief The identifier for this error
        inline const std::wstring& identifier() const { return m_Identifier; }
        
        /// \brief The description for this error
        inline const std::wstring& description() const { return m_Description; }
        
        /// \brief The file where this error occurred
        inline const std::wstring& filename() const { return m_Filename; }
        
        /// \brief The position in the file where the errors occurred
        inline const position& pos() const { return m_Position; }
        
        /// \brief The severity of this error
        inline severity sev() const { return m_Severity; }
    };
}

#endif
