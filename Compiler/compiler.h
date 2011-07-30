//
//  compiler.h
//  Parse
//
//  Created by Andrew Hunter on 30/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _COMPILER_COMPILER_H
#define _COMPILER_COMPILER_H

#include "Language/definition_file.h"

namespace compiler {
    ///
    /// \brief Class that handles compiling a definition file into a parser in a particular language
    ///
    class compiler {
    private:
        /// \brief The definition file that this compiler will handle
        language::definition_file_container m_DefinitionFile;
    };
}

#endif
