//
//  lr1_rewriter.h
//  TameParse
//
//  Created by Andrew Hunter on 13/01/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef _LR_LR1_REWRITER_H
#define _LR_LR1_REWRITER_H

#include "TameParse/Lr/action_rewriter.h"

namespace lr {
    ///
    /// \brief Action rewriter that rewrites a LALR parser so that it is a LR(1) parser
    ///
    /// LALR parsers may produce reduce/reduce conflicts where a LR(1) parser would
    /// not. These conflicts can be resolved (provided the grammar is indeed LR(1)) as
    /// they will have the property that only one side will result in a reduction.
    /// Reduce/reduce conflicts of this type can be tricky to resolve as the grammar
    /// will appear not to contain any conflicts.
    ///
    /// This rewriter will detect reduce/reduce conflicts where only one side will
    /// eventually shift the symbol and replace one side with a weak reduction.
    /// The technique is to find all the places that the conflicting lookahead for
    /// both items can come from (this will be the symbol that is shifted after the
    /// the reduction takes place). Provided that the conflicted nonterminal only
    /// occurs in a single place in the rules reached by the reduction, we can
    /// replace one of the reductions with a weak reduce to resolve the conflict.
    ///
 	/// Additionally, if the conflict becomes a shift/reduce we can also resolve
 	/// it this way (this allows other rewriters to resolve the conflict, and also
 	/// makes it easier to see why the grammar is conflicted)
    ///
    class lr1_rewriter {
        
    };
}

#endif
