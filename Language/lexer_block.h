//
//  lexer_block.h
//  Parse
//
//  Created by Andrew Hunter on 17/07/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LANGUAGE_LEXER_BLOCK_H
#define _LANGUAGE_LEXER_BLOCK_H

namespace language {
    ///
    /// \brief Class representing the definition of a block that defines lexical symbols
    ///
    /// A lexical symbol consists of an identifier and a definition which can be a string, a character, a regular expression
    /// or a reference to an existing lexical symbol in another language.
    ///
    /// In keyword blocks, the definition can additionally be empty (indicating that the lexical symbol is identical to 
    /// the identifier used for the keyword)
    ///
    class lexer_block {
        
    };
}

#endif
