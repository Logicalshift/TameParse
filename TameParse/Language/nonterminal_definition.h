//
//  nonterminal_definition.h
//  Parse
//
//  Created by Andrew Hunter on 24/07/2011.
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

#ifndef _LANGUAGE_NONTERMINAL_DEFINITION_H
#define _LANGUAGE_NONTERMINAL_DEFINITION_H

#include <string>
#include <vector>

#include "TameParse/Language/block.h"
#include "TameParse/Language/production_definition.h"

namespace language {
    ///
    /// \brief A nonterminal definition block from the parser language
    ///
    class nonterminal_definition : public block {
    public:
        /// \brief The types of nonterminal definition
        enum type {
            /// \brief This is a direct assignment to a nonterminal
            ///
            /// (That is, this should declare a new nonterminal, and it is an error for this to replace an existing
            /// nonterminal in this language or any language it inherits from)
            assignment,
            
            /// \brief This is an addition to an existing nonterminal
            ///
            /// It is an error if there is no initial definition for this nonterminal in this language or a language it
            /// inherits from
            addition,
            
            /// \brief This is a replacement for a nonterminal in a language that this inherits from
            ///
            /// It is an error if the nonterminal doesn't exist in the 
            replace
        };
        
        /// \brief List of production definitions
        typedef std::vector<production_definition*> production_definition_list;
        
        /// \brief Iterator for accessing the productions in this definition
        typedef production_definition_list::const_iterator iterator;
        
    private:
        /// \brief The type of this definition
        type m_Type;
        
        /// \brief The identifier of the nonterminal that is being defined
        std::wstring m_Identifier;
        
        /// \brief The productions that make up this nonterminal definition
        production_definition_list m_Productions;
        
    public:
        /// \brief Creates a new nonterminal definition for a nonterminal with the specified identifier
        nonterminal_definition(type definitionType, const std::wstring& identifier, position start = position(), position end = position());
        
        /// \brief Creates a new nonterminal definition by copying an existing one
        nonterminal_definition(const nonterminal_definition& copyFrom);
        
        /// \brief Destructor
        virtual ~nonterminal_definition();
        
        /// \brief The type of this definition
        inline type get_type() { return m_Type; }
        
        /// \brief Assigns the value of this nonterminal by copying an existing one
        nonterminal_definition& operator=(const nonterminal_definition& copyFrom);
        
        /// \brief Adds a new production to this object
        ///
        /// This object will become responsible for destroying the production passed in to this call
        void add_production(production_definition* newProduction);
        
        /// \brief The identifier for this definition
        const std::wstring& identifier() const { return m_Identifier; }
        
        /// \brief The first production in this definition
        inline iterator begin() const { return m_Productions.begin(); }
        
        /// \brief The production after the last one in this definition
        inline iterator end() const { return m_Productions.end(); }
    };
}

#endif
