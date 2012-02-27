//
//  version.h
//  TameParse
//
//  Created by Andrew Hunter on 24/09/2011.
//  
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

#ifndef _TAMEPARSE_VERSION_H
#define _TAMEPARSE_VERSION_H

#include <string>

namespace tameparse {
    /// \brief Class containing version information for the TameParse library
    class version {
    public:
        /// \brief The major version number of the library
        static const int major_version;
        
        /// \brief The minor version of the library
        static const int minor_version;
        
        /// \brief The revision of the library
        static const int revision;
        
        /// \brief A string representing the version of this library
        static const std::string version_string;
        
        /// \brief A string containing the date that this library was compiled
        static const std::string compile_date;
        
        /// \brief A string containing the release date for this library
        static const std::string release_date;
        
        /// \brief A string describing the copyright for this version of the library
        static const std::string copyright_string;
        
        /// \brief A string describing how to contact the author
        static const std::string contact_string;
        
        /// \brief A string describing the license for this version of the library
        static const std::string license_string;
        
        /// \brief A string describing the warranty for this version of the library
        static const std::string warranty_string;
    };
}

#endif
