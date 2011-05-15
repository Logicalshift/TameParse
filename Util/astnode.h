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

#include "dfa/lexeme.h"
#include "util/container.h"

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
        /// \brief The identifier of the nonterminal associated with this node (or -1 for a terminal node)
        int m_Nonterminal;
        
        /// \brief The lexeme associated with this node (will be NULL if this node doesn't contain a lexeme)
        dfa::lexeme_container m_Lexeme;
        
        /// \brief The child nodes of this node
        node_list m_Children;
        
    public:
        /// \brief Creates an AST node with a nonterminal ID
        astnode(int nonterminal);
        
        /// \brief Creates an AST node from a lexeme
        astnode(const dfa::lexeme_container& terminal);
        
        /// \brief Adds a new child node to this item
        void add_child(const astnode_container& newChild);
        
        /// \brief The nonterminal ID for this node
        inline int nonterminal() const { return m_Nonterminal; }
        
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
};

#endif
