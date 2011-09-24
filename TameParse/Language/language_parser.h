//
//  language_parser.h
//  TameParse
//
//  Created by Andrew Hunter on 19/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LANGUAGE_LANGUAGE_PARSER_H
#define _LANGUAGE_LANGUAGE_PARSER_H

#include "TameParse/Language/definition_file.h"

#include <string>

namespace language {
    ///
    /// \brief Class used to process parser language files
    ///
    /// This is the 'primary' language class: it accepts the 'real' parser language. The bootstrap class also accepts a
    /// parser language, but it is only intended to be comprehensive enough to read the language definition and build the
    /// parser used by this class.
    ///
    /// The generated parser can be found in the tameparse_language class: however, it is not recommended that you depend
    /// on this, as it is much more changable between versions than the definition_file classes.
    ///
    class language_parser {
    private:
        /// \brief NULL, or the definition file described by this object
        definition_file_container m_FileDefinition;
        
    public:
        /// \brief Creates a new language object
        language_parser();
        
        /// \brief Parses the language file specified in the given string and stores it in this object.
        ///
        /// This will return true if the file parsed correctly. If this is the case, then the file_definition() function
        /// will return the result. If there is any existing definition, this will be replaced by this call.
        bool parse(const std::wstring& language);
        
        /// \brief Parses the language file specified in the given string and stores it in this object.
        ///
        /// This will return true if the file parsed correctly. If this is the case, then the file_definition() function
        /// will return the result. If there is any existing definition, this will be replaced by this call.
        bool parse(const std::string& language);
        
        /// \brief Parses the language file specified by the given stream and stores it in this object
        bool parse(std::istream& language);
        
        /// \brief The file definition that was last accepted by this object
        inline definition_file_container file_definition() { return m_FileDefinition; }
    };
}

#endif