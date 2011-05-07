//
//  parser.h
//  Parse
//
//  Created by Andrew Hunter on 07/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_PARSER_H
#define _LR_PARSER_H

#include "Lr/lalr_builder.h"
#include "Dfa/lexer.h"

namespace lr {
    ///
    /// \brief A LR(1) parser
    ///
    class parser {
    public:
        
    private:
        
    public:
        /// \brief Creates a parser from the result of the specified builder class
        parser(const lalr_builder& builder);
        
        /// \brief Destructor
        virtual ~parser();
        
        /// \brief Parses the specified lexeme string
        void parse(dfa::lexeme_stream& input);
    };
}

#endif
