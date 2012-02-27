//
//  ebnf_item_attributes.h
//  Parse
//
//  Created by Andrew Hunter on 06/01/2012.
//  Copyright 2012 Andrew Hunter. All rights reserved.
//

#ifndef _LANGUAGE_EBNF_ITEM_ATTRIBUTES_H
#define _LANGUAGE_EBNF_ITEM_ATTRIBUTES_H

#include <vector>
#include <string>

namespace language {
    ///
    /// \brief Class representing the attributes associated with an EBNF item in a rule
    ///
    class ebnf_item_attributes {
    public:
        /// \brief The possible actions to take on an item when it is part of a shift/reduce conflict
        enum conflict_action {
            /// \brief No action specified (report a shift/reduce conflict as a warning)
            conflict_notspecified,

            /// \brief Shift the symbol
            conflict_shift,

            /// \brief Reduce the rule (even if it will result in a parser error)
            conflict_reduce,

            /// \brief Reduce the rule provided that the symbol will be subsequently shifted
            conflict_weakreduce
        };

    public:
        /// \brief The name of this item
        std::wstring name;

        /// \brief The action to take when this item is part of a shift/reduce conflict
        conflict_action conflict_action;

        /// \brief Set to true if this item is a guard that is allowed to clash (have the same start symbol) with other guards
        ///
        /// We require that the user supplies this attributes to guards that they
        /// know can begin with the same start symbol as other guards because clashing
        /// guards can make it harder to maintain the language, particularly if two
        /// guard symbols can match a common phrase: this will make the meaning of
        /// the language arbitrary depending on which guard is checked first.
        bool guard_can_clash;

    public:
        /// \brief Creates an empty attributes object
        ebnf_item_attributes();

        /// \brief Creates an attribute for an object with just a name
        explicit ebnf_item_attributes(std::wstring& name);
    };
}

#endif
