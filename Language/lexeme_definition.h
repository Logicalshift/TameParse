//
//  lexeme_definition.h
//  Parse
//
//  Created by Andrew Hunter on 24/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LANGUAGE_LEXEME_DEFINITION_H
#define _LANGUAGE_LEXEME_DEFINITION_H

#include <string>

#include "Language/block.h"

namespace language {
    ///
    /// \brief Class that represents the ways that an individual lexeme can be defined
    ///
    class lexeme_definition : public block {
    public:
        /// \brief The possible types of a lexeme definition
        enum type {
            /// \brief The identifier is the literal value of the block (eg, the lexeme is a keyword)
            literal,
            
            /// \brief A literal string
            string,
            
            /// \brief A single character
            character,
            
            /// \brief A regular expression
            regex
        };

    private:
        /// \brief The type of this definition
        type m_Type;
        
        /// \brief The identifier assigned to this lexeme
        std::wstring m_Identifier;
        
        /// \brief The definition assigned to this lexeme (how it is interpreted depends on type)
        ///
        /// Note that for strings, characters and regular expressions quoting is left intact for this field
        std::wstring m_Definition;
        
    public:
        /// \brief Creates a new lexeme definition
        lexeme_definition(type typ, std::wstring identifier, std::wstring definition, position start = position(), position end = position());
        
        /// \brief The type of this lexeme definition (determines how the definition field is interpreted)
        inline type get_type() const { return m_Type; }
        
        /// \brief The identifier assigned to this lexeme definition
        inline const std::wstring& identifier() const { return m_Identifier; }
        
        /// \brief How the lexeme is defined
        ///
        /// If get_type returns literal, this will be empty.
        /// If string, this will be a string of the form "Quoted string"
        /// If character, this will be a character of the form 'c'
        /// If regex, this will be a regular expression of the form /(regular expression)+/
        inline const std::wstring& definition() const { return m_Definition; }
    };
}

#endif
