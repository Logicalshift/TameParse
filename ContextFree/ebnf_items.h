//
//  ebnf_items.h
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _CONTEXTFREE_EBNF_ITEMS_H
#define _CONTEXTFREE_EBNF_ITEMS_H

#include "ContextFree/item.h"

namespace contextfree {
    /// \brief Forward declaration of a rule
    class rule;
    
    ///
    /// \brief Abstract base class of an EBNF item
    ///
    class ebnf : public item {
    private:
        /// \brief The rule that is included in this item
        rule* m_Rule;
        
    public:
        /// \brief Creates a new EBNF item
        ebnf();
        
        /// \brief Creates a new EBNF item from a rule
        ebnf(const rule& copyFrom);

        /// \brief Compares this item to another. Returns true if they are the same
        virtual bool operator==(const item& compareTo) const;
        
        /// \brief Orders this item relative to another item
        virtual bool operator<(const item& compareTo) const;

        /// \brief The rule that defines this item
        inline rule& get_rule() { return *m_Rule; }
        
        /// \brief The rule that defines this item
        inline const rule& get_rule() const { return *m_Rule; }
    };
}

#endif
