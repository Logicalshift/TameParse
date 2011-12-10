//
//  production_definition.h
//  Parse
//
//  Created by Andrew Hunter on 24/07/2011.
//  Copyright 2011 _Andrew Hunter_. All rights reserved.
//

#ifndef _LANGUAGE_PRODUCTION_DEFINITION_H
#define _LANGUAGE_PRODUCTION_DEFINITION_H

#include <vector>

#include "TameParse/Language/block.h"
#include "TameParse/Language/ebnf_item.h"

namespace language {
    ///
    /// \brief Class representing a production associated with a nonterminal
    ///
    class production_definition : public block {
    public:
        /// \brief List of EBNF items
        typedef std::vector<ebnf_item*> ebnf_item_list;
        
        /// \brief Iterator for accessing the values in this production definition
        typedef ebnf_item_list::const_iterator iterator;
        
    private:
        /// \brief The EBNF items that make up this production
        ebnf_item_list m_Items;
        
    public:
        /// \brief Creates a new production definition
        production_definition(position start = position(), position end = position());
        
        /// \brief Creates a production definition by copying an existing definition
        production_definition(const production_definition& copyFrom);
        
        /// \brief Destructor
        virtual ~production_definition();
        
        /// \brief Assigns the value of this object to that of another
        production_definition& operator=(const production_definition& copyFrom);
        
        /// \brief Adds an EBNF item to this production definition
        ///
        /// This object will become responsible for destroying the item that is passed in
        void add_item(ebnf_item* newItem);
        
        /// \brief The first EBNF item in this production
        inline iterator begin() const { return m_Items.begin(); }
        
        /// \brief The item after the last EBNF item in this production
        inline iterator end() const { return m_Items.end(); }
    };
}

#endif
