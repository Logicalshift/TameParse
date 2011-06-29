//
//  guard.h
//  Parse
//
//  Created by Andrew Hunter on 29/06/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _CONTEXTFREE_GUARD_H
#define _CONTEXTFREE_GUARD_H

#include "ContextFree/item.h"

namespace contextfree {
    ///
    /// \brief Item representing a guard symbol
    ///
    /// A guard symbol is used to provide the parser with a means of disambiguating a grammar that is not LALR(1). In cases where
    /// there is no conflict, guard symbols are typically ignored. In states where there are conflicts for a particular symbol,
    /// the parser will look for guard symbols that can be applied, and will evaluate them against the lookahead.
    ///
    /// The usual way to use a guard symbol is to specify a context-free production: if the lookahead is matched against it, then
    /// the guard is matched and parsing can proceed from this point.
    ///
    /// Guards can be used to extend the parser to match unambiguous grammars that are not LALR(1), most trivially by acting to
    /// extend the lookahead, but also to match much more complicated languages where the lookahead must be checked against a
    /// context-free productions.
    ///
    /// Guards can also be used to disambiguate ambiguous grammars, as if a guard condition is matched, then no other alternatives
    /// will be considered. This can be used to produce a more natural expression of a language, or specify the 'preferred'
    /// intepretation of a truly ambiguous language. (For example, this can be used to deal with the problems inherant to using
    /// '<' and '>' to specify C++ style templates and generics)
    ///
    class guard : public item {
    public:
        /// \brief Creates a guard item that matches the rule with the specified identifier
        guard(int sym);
        
        /// \brief Creates a clone of this item
        virtual item* clone() const;
        
        /// \brief The type of this item
        virtual kind type() const;
        
        /// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
        ///
        /// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
        /// set for the rules that make them up.
        ///
        /// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
        /// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
        virtual item_set first(const grammar& gram) const;
    };
}

#endif
