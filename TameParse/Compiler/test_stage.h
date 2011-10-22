//
//  test_stage.h
//  TameParse
//
//  Created by Andrew Hunter on 22/10/2011.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _COMPILER_TEST_STAGE_H
#define _COMPILER_TEST_STAGE_H

#include "TameParse/Compiler/compilation_stage.h"
#include "TameParse/Compiler/import_stage.h"
#include "TameParse/Language/definition_file.h"

namespace compiler {
    ///
    /// \brief Compilation stage that runs the tests specified in a language definition block
    /// 
    class test_stage : public compilation_stage {
    private:
    	/// \brief The definition file that contains the tests that this
     	language::definition_file_container m_Definition;

     	/// \brief The import stage where languages should be loaded from
     	const import_stage* m_Import;
        
    public:
        /// \brief Creates a new test stage that will run the tests in the specified definition file
        test_stage(console_container& console, const std::wstring& filename, const language::definition_file_container& definition, const import_stage* import);

        /// \brief Performs the actions associated with this compilation stage
        virtual void compile();
    };
}

#endif
