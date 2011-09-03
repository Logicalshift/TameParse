//
//  symbol_table.h
//  Parse
//
//  Created by Andrew Hunter on 28/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_SYMBOL_TABLE_H
#define _DFA_SYMBOL_TABLE_H

#include "Dfa/symbol_map.h"

namespace dfa {
    ///
    /// \brief Structure representing a level of symbols stored in this translator
    ///
    /// A symbol level provides an action for a set of bits in a given character.
    /// A suitable class needs to provide a lookup method that will map a given symbol
    /// to the set it belongs to. This particular implementation looks up the action
    /// for the set of bits represented by (symbol & mask) >> shift and then passes
    /// the symbol on to the next type. It also provides methods for populating the
    /// table, so it is suitable for use with dynamically generated DFAs.
    ///
    /// This implementation also specifies a default symbol, so it can short-circuit
    /// cases where an entire block of items are mapped to the same symbol.
    ///
    template<typename next_level, unsigned int mask, int shift> struct symbol_level {
        /// \brief Default symbol to use at this level for items that are null
        int DefaultSymbol;
        
        /// \brief Number of entries at this level in the table
        static const int c_MaxIndex = (mask>>shift)+1;
        
        /// \brief null, or the symbols at this level
        next_level** Symbols;
        
        /// \brief Creates an empty symbol level
        symbol_level() {
            DefaultSymbol   = symbol_set::null;
            Symbols         = NULL;
        }
        
        /// \brief Creates a copy of an existing symbol level
        symbol_level(const symbol_level& copyFrom) {
            DefaultSymbol   = copyFrom.DefaultSymbol;
            Symbols         = NULL;
            
            if (copyFrom.Symbols) {
                Symbols = new next_level*[c_MaxIndex];
                
                for (int x=0; x<c_MaxIndex; x++) {
                    if (copyFrom.Symbols[x]) {
                        Symbols[x] = new next_level(*copyFrom.Symbols[x]);
                    } else {
                        Symbols[x] = NULL;
                    }
                }
            }
        }
        
        /// \brief Destructor
        ~symbol_level() {
            if (Symbols) {
                for (int x=0; x<c_MaxIndex; x++) {
                    if (Symbols[x]) delete Symbols[x];
                }
                delete[] Symbols;
            }
        }
        
        /// \brief Creates the table at the specified index
        inline void create(int index) {
            if (Symbols[index]) return;
            
            Symbols[index]      = new next_level();
        }
        
        /// \brief Sets all the symbols in the specified range to the specified symbol. The range must overlap this item.
        void add_range(int base, const range<int>& range, int symbol) {
            // Work out the initial index in this table
            int startIndex = range.lower() - base;
            if (startIndex < 0) startIndex = 0;
            int startOffset = (startIndex)&((1<<shift)-1);
            startIndex >>= shift;
            
            // Work out the final index in this table
            int endIndex = range.upper() - base;
            if (endIndex < 0) endIndex = 0;
            endIndex >>= shift;
            
            // If the range entirely covers this item, then set the default symbol and stop
            if (startOffset == 0 && startIndex == 0 && endIndex >= c_MaxIndex) {
                DefaultSymbol = symbol;
                return;
            }
            
            if (endIndex > c_MaxIndex-1) endIndex = c_MaxIndex-1;
            
            // Create the symbol set if needed
            if (!Symbols) {
                Symbols = new next_level*[c_MaxIndex];
                for (int index=0; index < c_MaxIndex; index++) {
                    Symbols[index] = NULL;
                }
            }
            
            // Otherwise, create new entries (if needed) and add the range there as well
            for (int index = startIndex; index <= endIndex; index++) {
                // Create a new entry if needed
                if (!Symbols[index]) Symbols[index] = new next_level();
                
                // Add the range there (with an updated base value)
                Symbols[index]->add_range(base + (index << shift), range, symbol);
            }
        }
        
        /// \brief Looks up a value in this table
        inline int lookup(int val) const {
            if (!Symbols) return DefaultSymbol;
            const next_level* next = Symbols[(val&mask)>>shift];
            if (!next) return DefaultSymbol;
            
            return next->lookup(val);
        }
        
        /// \brief The size of this item in bytes (used when computing the required size in memory of a particular lexer)
        inline size_t size() const {
            size_t mySize = sizeof(symbol_level<next_level, mask, shift>);
            if (Symbols) {
                mySize += sizeof(next_level*[c_MaxIndex]);
                for (int index=0; index<c_MaxIndex; index++) {
                    if (Symbols[index]) mySize += Symbols[index]->size();
                }
            }
            return mySize;
        }

    public:
        /// \brief Returns the default symbol for this level
        int get_default_symbol() const { return DefaultSymbol; }

        /// \brief Gets the next level array
        next_level** get_next_level() const { return Symbols; }
        
    private:
        /// \brief Disabled assignment
        symbol_level<next_level, mask, shift>& operator=(const symbol_level<next_level, mask, shift>& assignFrom);
    };
    
    ///
    /// \brief Structure representing a level of symbols stored in this translator
    ///
    /// This specialization represents a 'base' symbol table, which maps items
    /// represented as (symbol & mask) >> shift to symbol sets.
    ///
    template<unsigned int mask, int shift> struct symbol_level<int, mask, shift> {
        /// \brief Number of entries at this level in the table
        static const int c_MaxIndex = (mask>>shift)+1;
        
        /// \brief The symbol to use if no symbols are defined for this object
        int DefaultSymbol;
        
        /// \brief null, or the symbols at this level
        int* Symbols;
        
        symbol_level() {
            DefaultSymbol   = symbol_set::null;
            Symbols         = NULL;
        }
        
        /// \brief Sets all the symbols in the specified range to the specified symbol. The range must overlap this item.
        void add_range(int base, const range<int>& range, int symbol) {
            // Work out the initial index in this table
            int startIndex = range.lower() - base;
            if (startIndex < 0) startIndex = 0;
            int startOffset = (startIndex)&((1<<shift)-1);
            startIndex >>= shift;
            
            // Work out the final index in this table
            int endIndex = range.upper() - base;
            if (endIndex < 0) endIndex = 0;
            endIndex >>= shift;
            
            // If the range entirely covers this item, then set the default symbol and stop
            if (startOffset == 0 && startIndex == 0 && endIndex >= c_MaxIndex) {
                DefaultSymbol = symbol;
                return;
            }
            
            if (endIndex > c_MaxIndex) endIndex = c_MaxIndex;
            
            // Create the symbol array if needed
            if (!Symbols) {
                Symbols = new int[c_MaxIndex];
                for (int index=0; index< c_MaxIndex; index++) Symbols[index] = symbol_set::null;
            }
            
            // Fill in the symbols between these indexes
            for (int index=startIndex; index < endIndex; index++) {
                Symbols[index] = symbol;
            }
        }
        
        /// \brief Looks up a value in this table
        inline int lookup(int val) const {
            if (!Symbols) return DefaultSymbol;
            return Symbols[(val&mask)>>shift];
        }
        
        /// \brief The size of this item
        inline size_t size() const { 
            size_t mySize = sizeof(symbol_level<int, mask, shift>); 
            if (Symbols) {
                mySize += sizeof(int[c_MaxIndex]);
            }
            return mySize;
        }

    public:
        /// \brief Returns the default symbol for this level
        int get_default_symbol() const { return DefaultSymbol; }

        /// \brief Gets the symbol sets for this level
        int* get_next_level() const { return Symbols; }
    };
    
    ///
    /// \brief Template class representing the default symbol_level definition to use for a specific character type
    ///
    /// Default is to accept 8-bit characters
    ///
    template<class Char> class symbol_level_for : public symbol_level<int, 0xff, 0> {
    };

    ///
    /// \brief Default symbol level class for parsers accepting 16-bit unicode languages
    ///
    template<> class symbol_level_for<wchar_t> : public symbol_level<symbol_level<int, 0xff, 0>, 0xff00, 8> {
    };
    
    ///
    /// \brief Default symbol level class for parsers accepting 32-bit integer languages
    ///
    template<> class symbol_level_for<int> : public symbol_level<symbol_level<symbol_level<symbol_level<int, 0xff, 0>, 0xff00, 8>, 0xff0000, 16>, 0x7f000000, 24> {
    };
    
    ///
    /// \brief Default format of a symbol table
    ///
    template<class symbol_type, class table_type = symbol_level_for<symbol_type> > struct symbol_table {
        table_type Table;
        
        inline int lookup(symbol_type val) const {
            return Table.lookup(val);
        }
        
        inline void add_range(const range<int>& range, int symbol) {
            Table.add_range(0, range, symbol);
        }
    };
}

#endif
