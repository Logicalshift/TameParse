//
//  import_stage.h
//  TameParse
//
//  Created by Andrew Hunter on 25/09/2011.
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

#ifndef _COMPILER_IMPORT_STAGE_H
#define _COMPILER_IMPORT_STAGE_H

#include <string>
#include <map>

#include "TameParse/Compiler/compilation_stage.h"
#include "TameParse/Language/definition_file.h"

namespace compiler {
    ///
    /// \brief Compiler stage that loads in any imported files and discovers where each named language is defined
    ///
    class import_stage : public compilation_stage {
    public:
        /// \brief Map of language names to their corresponding blocks
        typedef std::map<std::wstring, const yy_language::language_block*> language_block_map;

        /// \brief Maps strings to other strings
        typedef std::map<std::wstring, std::wstring> string_map;

        /// \brief Maps filenames to the definition file that they specify
        typedef std::map<std::wstring, yy_language::definition_file_container> definition_map;

        /// \brief Iterator for retrieving a list of language name, block pairs
        typedef language_block_map::const_iterator language_iterator;

    private:
        /// \brief The definition file containers that are imported by this object
        definition_map m_DefinitionForFile;
        
        /// \brief Maps the 'real' paths used in m_StageForFile to the short paths we should display in messages
        string_map m_ShortNameForFile;
        
        /// \brief Maps names to language blocks
        language_block_map m_LanguageBlock;
        
        /// \brief Maps language names to the filenames that they are defined in
        string_map m_LanguageFile;

    public:
        /// \brief Creates a new import stage
        import_stage(console_container& console, const std::wstring& filename, yy_language::definition_file_container rootFile);

        /// \brief Destructor
        virtual ~import_stage();

        /// \brief Performs the actions associated with this compilation stage
        virtual void compile();

    public:
        /// \brief Returns the language with the specified name, or NULL if it doesn't exist
        inline const yy_language::language_block* language_with_name(const std::wstring& languageName) const {
            language_block_map::const_iterator found = m_LanguageBlock.find(languageName);
            if (found == m_LanguageBlock.end()) return NULL;
            return found->second;
        }

        /// \brief Returns the display name of the file containing the language with the specified name
        inline std::wstring file_with_language(const std::wstring& languageName) const {
            // Find the language file
            string_map::const_iterator found = m_LanguageFile.find(languageName);
            if (found == m_LanguageFile.end()) return L"unknown";
            
            // Use the short name (if present, which it certainly will be)
            string_map::const_iterator shortName = m_ShortNameForFile.find(found->second);
            if (shortName != m_ShortNameForFile.end()) {
                return shortName->second;
            } else {
                // Won't happen :-/
                return found->second;
            }
        }

        /// \brief An iterator that returns the first language in this object
        inline language_iterator begin_language() const { return m_LanguageBlock.begin(); }

        /// \brief An iterator that returns the item after the last language in this object
        inline language_iterator end_language() const { return m_LanguageBlock.end(); }
    };
}

#endif
