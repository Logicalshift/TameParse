//
//  import_stage.h
//  TameParse
//
//  Created by Andrew Hunter on 25/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
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
	private:
		/// \brief The definition file containers that are imported by this object
		std::map<std::wstring, language::definition_file_container> m_StageForFile;
        
        /// \brief Maps the 'real' paths used in m_StageForFile to the short paths we should display in messages
        std::map<std::wstring, std::wstring> m_ShortNameForFile;
        
        /// \brief Maps names to language blocks
        std::map<std::wstring, const language::language_block*> m_LanguageBlock;
        
        /// \brief Maps language names to the filenames that they are defined in
        std::map<std::wstring, std::wstring> m_LanguageFile;

	public:
		/// \brief Creates a new import stage
		import_stage(console_container& console, const std::wstring& filename, language::definition_file_container rootFile);

		/// \brief Destructor
		virtual ~import_stage();

        /// \brief Performs the actions associated with this compilation stage
		virtual void compile();
	};
}

#endif
