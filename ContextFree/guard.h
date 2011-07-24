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
#include "ContextFree/rule.h"

namespace contextfree {
    // TODO: support guard conditions which call arbitrary function, we only support context-free guards for the moment.
    
    ///
    /// \brief Item representing a guard symbol
    ///
    /// A guard symbol is used to provide the parser with a means of disambiguating a grammar that is not LALR(1). These symbols
    /// act like terminals that are spontaneously generated when the lookahead matches a particular pattern. They do not appear
    /// in the final AST, however.
    ///
    /// In a state where guard symbols are present, the parser will evaluate them (in order) for any symbol that is in their
    /// first set. If a guard is matched, then the action associated with it will be performed; if there are multiple possible
    /// guards, only the first match is followed.
    ///
    /// Guard symbols that match the lookaehad against a series of symbols work to increase the lookahead supported by the parser.
    /// These can be used to parse LALR(k) grammars.
    ///
    /// Guard symbols that match the lookahead against a context-free grammar effectively enable the parser to perform back-tracking.
    /// There are a couple of advantages of using guards rather than a back-tracking parser: firstly, guards explicitly define the
    /// parser behaviour for a given input. Secondly, guards do not affect the ability of the parser generator to report ambiguities
    /// in the grammar.
    ///
    /// Guards can also be used to specify how ambiguous grammars should be interpreted. For example, they can be used to resolve the
    /// ambiguities associated with templates in languages like C++. They can also make expressing grammars for unambiguous languages
    /// more natural.
    ///
    class guard : public item {
    private:
        /// \brief The rule that should be matched for this guard to be successful
        rule_container m_Rule;
        
        /// \brief The priority of this guard compared to other guards
        int m_Priority;

        /// \brief Disabled assignment
        guard& operator=(const guard& assignFrom);
        
    public:
        /// \brief Creates a new guard item, with a specified priority
        guard(int priority = -1);
        
        /// \brief Creates a new guard item by copying a rule
        guard(const rule& copyFrom, int priority = -1);
        
        /// \brief Creates a new guard by copying an old one
        guard(const guard& copyFrom);
        
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
        
        /// \brief Orders this item relative to another item
        virtual bool operator<(const item& compareTo) const;
        
        /// \brief Computes the set of symbols that can form the initial symbol of a lookahead that matches this guard
        virtual item_set initial(const grammar& gram) const;
        
        /// \brief The priority of this guard relative to other guards (higher values = higher priorities, -1 by default)
        int priority() const;
        
        /// \brief The rule that should be matched for this guard to be successful
        rule_container& get_rule();
        
        /// \brief The rule that should be matched for this guard to be successful
        const rule_container& get_rule() const;
    };
}

#endif
