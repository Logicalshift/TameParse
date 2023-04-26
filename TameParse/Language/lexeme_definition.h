//
//  lexeme_definition.h
//  Parse
//
//  Created by Andrew Hunter on 24/07/2011.
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

#ifndef _LANGUAGE_LEXEME_DEFINITION_H
#define _LANGUAGE_LEXEME_DEFINITION_H

#include <string>

#include "TameParse/Language/block.h"

namespace yy_language {
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
        
        /// \brief True if this is an alternate definition of an existing symbol
        bool m_AddToDefinition;

        /// \brief True if this is a replacement for an existing symbol
        bool m_ReplaceDefinition;
        
        /// \brief The definition assigned to this lexeme (how it is interpreted depends on type)
        ///
        /// Note that for strings, characters and regular expressions quoting is left intact for this field
        std::wstring m_Definition;

        /// \brief Position where the definition is defined
        dfa::position m_DefinitionPos;
        
    public:
        /// \brief Creates a new lexeme definition
        lexeme_definition(type typ, std::wstring identifier, std::wstring definition, bool addToDefinition, bool replaceDefinition, position start, position end, position definition_pos);
        
        /// \brief The type of this lexeme definition (determines how the definition field is interpreted)
        inline type get_type() const { return m_Type; }
        
        /// \brief The identifier assigned to this lexeme definition
        inline const std::wstring& identifier() const { return m_Identifier; }
        
        /// \brief True if this is an alternative to an existing lexer symbol definition
        inline bool add_to_definition() const { return m_AddToDefinition; }

        /// \brief True if the definition should be replaced
        inline bool replace_definition() const { return m_ReplaceDefinition; }
        
        /// \brief How the lexeme is defined
        ///
        /// If get_type returns literal, this will be empty.
        /// If string, this will be a string of the form "Quoted string"
        /// If character, this will be a character of the form 'c'
        /// If regex, this will be a regular expression of the form /(regular expression)+/
        inline const std::wstring& definition() const { return m_Definition; }

        /// \brief Position where the definition appears in the file
        inline const dfa::position& definition_pos() const { return m_DefinitionPos; }
    };
}

#endif
