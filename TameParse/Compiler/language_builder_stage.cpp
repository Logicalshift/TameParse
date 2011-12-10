//
//  language_builder_stage.cpp
//  TameParse
//
//  Created by Andrew Hunter on 25/09/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
//

#include "TameParse/Compiler/language_builder_stage.h"

using namespace dfa;
using namespace language;
using namespace compiler;

/// \brief Creates a new language builder stage
language_builder_stage::language_builder_stage(console_container& console, const std::wstring& filename, import_stage* importStage) 
: compilation_stage(console, filename)
, m_ImportStage(importStage) {
}

/// \brief Destructor
language_builder_stage::~language_builder_stage() {
	// Delete the language stages
	for (language_map::iterator language = m_Languages.begin(); language != m_Languages.end(); ++language) {
		delete language->second;
	}

	// Finished with the languages
	m_Languages.clear();

	// The import stage doesn't belong to this object, so we don't free it here
}

/// \brief Performs the actions associated with this compilation stage
void language_builder_stage::compile() {
	// Sanity check
	if (!m_ImportStage) {
		cons().report_error(error(error::sev_bug, filename(), L"BUG_NO_IMPORT_STAGE", L"Import stage missing", position(-1, -1, -1)));
	}

	console_container consContainer(cons_container());

	// Run through all of the language blocks in the import stage
	for (import_stage::language_iterator language = m_ImportStage->begin_language(); language != m_ImportStage->end_language(); ++language) {
		// Fetch the parts of this name
		std::wstring            languageName        = language->first;
		const language_block*   languageBlock       = language->second;
		std::wstring            languageFileName    = m_ImportStage->file_with_language(languageName);

		// Nothing to do if this language has already been compiled
		if (m_Languages.find(languageName) != m_Languages.end()) {
			// Can happen if one language inherits from this one and was compiled earlier
			continue;
		}

		// Create a language stage to compile this language
		language_stage* stage 		= new language_stage(consContainer, languageFileName, languageBlock, m_ImportStage);
		m_Languages[languageName] 	= stage;

		// Compile it
		stage->compile();
	}
    
    // It's an error for no languages to be defined (as we won't be able to define anything)
    if (m_Languages.size() == 0) {
        cons().report_error(error(error::sev_error, filename(), L"NO_LANGUAGES_DEFINED", L"No languages are defined", position(-1, -1, -1)));
    }
}
