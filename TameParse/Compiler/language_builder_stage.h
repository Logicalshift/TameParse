//
//  language_builder_stage.h
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

#ifndef _COMPILER_LANGUAGE_BUILDER_STAGE_H
#define _COMPILER_LANGUAGE_BUILDER_STAGE_H

#include <map>
#include <string>

#include "TameParse/Compiler/compilation_stage.h"
#include "TameParse/Compiler/language_stage.h"
#include "TameParse/Compiler/import_stage.h"

namespace compiler {
    ///
    /// \brief Stage that creates grammars and NDFAs from all of the languages that were
    /// imported by an import_stage
    ///
    class language_builder_stage : public compilation_stage {
    public:
        /// \brief Maps language names to the stages that compiled them
        typedef std::map<std::wstring, language_stage*> language_map;

    private:
        /// \brief The import stage
        ///
        /// This contains all of the languages loaded by this parser generator. This
        /// stage is not freed by this object
        import_stage* m_ImportStage;

        /// \brief Maps languages to the grammars and NDFAs that are built for them
        ///
        /// These stages are destroyed when this object is destroyed
        language_map m_Languages;

    public:
        /// \brief Creates a new language builder stage
        language_builder_stage(console_container& console, const std::wstring& filename, import_stage* importStage);

        /// \brief Destructor
        virtual ~language_builder_stage();

        /// \brief Performs the actions associated with this compilation stage
        virtual void compile();
        
        /// \brief Returns the language stage for the language with the specified name, or NULL if it was not compiled by this object
        inline language_stage* language_with_name(const std::wstring& languageName) const { 
            language_map::const_iterator found = m_Languages.find(languageName);
            if (found != m_Languages.end()) {
                return found->second;
            }
            return NULL;
        }
    };
}


#endif
