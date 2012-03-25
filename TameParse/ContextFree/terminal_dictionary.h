//
//  terminal_dictionary.h
//  Parse
//
//  Created by Andrew Hunter on 07/05/2011.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy 
//  of this software and associated documentation files (the \"Software\"), to 
//  deal in the Software without restriction, including without limitation the 
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
//  sell copies of the Software, and to permit persons to whom the Software is 
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
//  IN THE SOFTWARE.
//

#ifndef _CONTEXTFREE_TERMINAL_DICTIONARY_H
#define _CONTEXTFREE_TERMINAL_DICTIONARY_H

#include <string>
#include <map>
#include <set>

namespace contextfree {
    ///
    /// \brief Class representing a dictionary of terminal values
    ///
    /// This performs two purposes: it assigns symbol values to terminals identified by strings, and it makes it possible to
    /// reassign terminal values to possibly many different meanings.
    ///
    class terminal_dictionary {
    public:
        /// \brief Class mapping strings to the associated terminal identifiers
        typedef std::map<std::wstring, int> name_to_symbol;
        
        /// \brief Class mapping symbol identifiers to their names
        typedef std::map<int, std::wstring> symbol_to_name;
        
        /// \brief Class mapping symbols to a 'parent' symbol
        ///
        /// Sometimes it's necessary to map a symbol to multiple meanings. For instance when combining lexers, it's possible
        /// that different symbols will appear in the accepting states. To map symbols from both lexers, it's necessary to
        /// split the individual symbols, into 'A-or-B' and 'B-or-C' symbols.
        ///
        /// Any given symbol can have one parent, but multiple child symbols
        typedef std::map<int, int> symbol_to_parent;
        
        /// \brief Set of symbols
        typedef std::set<int> symbol_set;
        
        /// \brief Maps a symbol to its child symbols
        typedef std::map<int, symbol_set> symbol_to_children;

        /// \brief Iterator used to access all of the symbols defined in this object
        typedef symbol_to_name::const_iterator iterator;
        
    private:
        /// \brief Maximum used symbol
        int m_MaxSymbol;
        
        /// \brief Table of symbol names
        name_to_symbol m_NameToSymbol;
        
        /// \brief Table of symbol names
        symbol_to_name m_SymbolToName;
        
        /// \brief Maps symbols to their parent symbols
        symbol_to_parent m_ParentFor;
        
        /// \brief Maps symbols to their child symbols
        symbol_to_children m_ChildrenFor;
        
    public:
        /// \brief Creates a new terminal dictionary
        terminal_dictionary();
        
    public:
        /// \brief Adds a new anonymous symbol and returns its identifier
        int add_symbol();
        
        /// \brief Adds a named symbol and returns its identifier
        int add_symbol(const std::wstring& name);
        
        /// \brief Adds a new symbol with the specified identifier (which must be unused)
        void add_symbol(const std::wstring& name, int value);
        
        /// \brief Splits the symbol with the specified identifier, adding a new symbol and marking the original as its parent
        ///
        /// If the symbol was already split off from another, then the split is actually added to the 'parent' symbol rather
        /// than the supplied symbol
        int split(int symbol);
        
        /// \brief Total number of symbols defined in this dictionary
        inline int count_symbols() const { return (int)m_SymbolToName.size(); }
        
    public:
        /// \brief Returns the parent of the specified symbol (identical to the symbol if it wasn't split off from another symbol)
        inline int parent_of(int symbol) const {
            symbol_to_parent::const_iterator found = m_ParentFor.find(symbol);
            if (found == m_ParentFor.end()) return symbol;
            return found->second;
        }
        
        /// \brief Returns the set of symbols that are children of the specified symbol (or the empty set if it has no children)
        inline const symbol_set& children_of(int symbol) const {
            static symbol_set empty;
            symbol_to_children::const_iterator found = m_ChildrenFor.find(symbol);
            if (found == m_ChildrenFor.end()) return empty;
            return found->second;
        }

        /// \brief Retrieves the name for the specified symbol (or the empty string if it has no associated name)
        inline const std::wstring& name_for_symbol(int symbol) const {
            static std::wstring emptyString;
            
            symbol_to_name::const_iterator found = m_SymbolToName.find(symbol);
            if (found == m_SymbolToName.end()) {
                int parentId = parent_of(symbol);
                if (parentId != symbol) {
                    return name_for_symbol(parentId);
                }
                
                return emptyString;
            }
            
            return found->second;
        }
        
        /// \brief Retrieves the symbol with the specified name, or -1 if there is no symbol with this name
        inline int symbol_for_name(const std::wstring& name) const {
            name_to_symbol::const_iterator found = m_NameToSymbol.find(name);
            if (found == m_NameToSymbol.end()) return -1;
            
            return found->second;
        }

    public:
        /// \brief Returns an iterator pointing to the first symbol defined in this object
        ///
        /// Symbols are ordered by ID (which usually begins from 0 and increases)
        inline iterator begin() const { return m_SymbolToName.begin(); }

        /// \brief Returns an iterator pointing to the symbol after the last symbol defined in this object
        ///
        /// Symbols are ordered by ID (which usually begins from 0 and increases)
        inline iterator end() const { return m_SymbolToName.end(); }
    };
}

#endif
