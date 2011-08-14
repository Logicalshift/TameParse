//
//  language_compiler.cpp
//  Parse
//
//  Created by Andrew Hunter on 30/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "Compiler/language_compiler.h"

using namespace compiler;

/// \brief Creates a compiler that will compile the specified language block
language_compiler::language_compiler(console_container& console, language::language_block* block)
: compilation_stage(console) {
}

/// \brief Destructor
language_compiler::~language_compiler() {
    
}

/// \brief Compiles the language, creating the dictionary of terminals, the lexer and the grammar
void language_compiler::compile() {
    
}
