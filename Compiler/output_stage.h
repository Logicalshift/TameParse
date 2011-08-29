//
//  output_stage.h
//  Parse
//
//  Created by Andrew Hunter on 29/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _COMPILER_OUTPUT_STAGE_H
#define _COMPILER_OUTPUT_STAGE_H

#include "Compiler/compilation_stage.h"
#include "Compiler/language_stage.h"
#include "Compiler/lexer_stage.h"

namespace compiler {
	///
	/// \brief Base class for a compilation stage that produces output in a given language
	///
	class output_stage : public compilation_stage {
	private:
		
	public:
		/// \brief Creates a new output stage
		output_stage(console_container& console, const std::wstring& filename, lexer_stage* lexer, language_stage* language);

		/// \brief Destructor
		virtual ~output_stage();

		/// \brief Compiles the parser specified by this stage
		///
		/// Subclasses can override this if they want to substantially change the way that the
		/// compiler is generated.
		void compile();

	protected:
	};
}

#endif
