//
//  test_stage.cpp
//  TameParse
//
//  Created by Andrew Hunter on 22/10/2011.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/Compiler/test_stage.h"

using namespace language;
using namespace compiler;

/// \brief Creates a new test stage that will run the tests in the specified definition file
test_stage::test_stage(console_container& console, const std::wstring& filename, const language::definition_file_container& definition, const import_stage* import)
: compilation_stage(console, filename)
, m_Definition(definition)
, m_Import(import) {
}

/// \brief Performs the actions associated with this compilation stage
void test_stage::compile() {
	// TODO: implement me
}
