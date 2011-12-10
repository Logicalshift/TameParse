//
//  version.h
//  TameParse
//
//  Created by Andrew Hunter on 24/09/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
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
