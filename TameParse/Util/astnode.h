//
//  astnode.h
//  Parse
//
//  Created by Andrew Hunter on 15/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _UTIL_ASTNODE_H
#define _UTIL_ASTNODE_H

#include <vector>

#include "TameParse/Dfa/lexeme.h"
#include "TameParse/Util/container.h"

namespace util {
    class astnode;
    typedef container<astnode> astnode_container;
    
    ///
    /// \brief Class representing an abstract syntax tree
    ///
    class astnode {
    public:
        /// \brief List of AST nodes
        typedef std::vector<astnode_container> node_list;
        
    private:
        /// \brief The identifier of the grammar item associated with this node (or -1 for a terminal node)
        int m_ItemIdentifier;
        
        /// \brief The identifier of the rule that was matched for this node (or -1 for a terminal node)
        int m_Rule;
        
        /// \brief The lexeme associated with this node (will be NULL if this node doesn't contain a lexeme)
        dfa::lexeme_container m_Lexeme;
        
        /// \brief The child nodes of this node
        node_list m_Children;
        
    public:
        /// \brief Creates an empty AST node (not recommended you do this, it doesn't make much sense)
        astnode();
        
        /// \brief Creates an AST node with the specified rule and item identifier
        astnode(int itemIdentifier, int rule = -1);
        
        /// \brief Creates an AST node from a lexeme
        astnode(const dfa::lexeme_container& terminal);
        
        /// \brief Adds a new child node to this item
        void add_child(const astnode_container& newChild);
        
        /// \brief Adds a series of children to this item
        template<typename iterator> void add_children(iterator begin, iterator end) {
            for (iterator cur = begin; cur != end; ++cur) {
                m_Children.push_back(*cur);
            }
        }
        
        /// \brief The ID of the rule for this node
        inline int rule() const { return m_Rule; }
        
        /// \brief The identifier of the item representing the nonterminal for this node
        inline int item_identifier() const { return m_ItemIdentifier; }
        
        /// \brief The lexeme associated with this node (will be NULL if this node doesn't contain a lexeme)
        inline const dfa::lexeme_container& lexeme() const { return m_Lexeme; }
        
        /// \brief The child nodes of this node
        inline const node_list& children() const { return m_Children; }
        
        /// \brief Returns the child at the specified index
        inline const astnode_container& operator[](int x) const { return children()[x]; }
        
    public:
        /// \brief Clones this AST node
        virtual astnode* clone() const;
        
        /// \brief Orders this node relative to another
        virtual bool operator<(const astnode& compareTo) const;
        
        inline static bool compare(const astnode* a, const astnode* b) {
            if (a == b) return false;
            if (!a) return true;
            if (!b) return false;
            
            return *a < *b;
        }
    };
}

#endif
