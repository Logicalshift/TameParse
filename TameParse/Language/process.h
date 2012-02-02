//
//  process.h
//  Parse
//
//  Created by Andrew Hunter on 23/07/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#ifndef _LANGUAGE_PROCESS_H
#define _LANGUAGE_PROCESS_H

#include <string>

namespace language {
    ///
    /// \brief Routines to help with processing items from a language file
    ///
    class process {
    public:
        /// \brief Given a string with quotation marks and '\' quoting, returns the literal string
        static std::wstring dequote_string(const std::wstring& string);
    };
}

#endif
