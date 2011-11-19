//
//  ast_parser.cpp
//  Parse
//
//  Created by Andrew Hunter on 21/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/common.h"

#include "TameParse/Lr/ast_parser.h"

template class lr::parser<util::astnode_container, lr::ast_parser_actions>;
