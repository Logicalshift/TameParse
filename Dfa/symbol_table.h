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
    template<class T, unsigned int mask, int shift> struct symbol_level {
        /// \brief Default symbol to use at this level for items that are null
        int DefaultSymbol;
        
        /// \brief Number of entries at this level in the table
        static const int c_MaxIndex = (mask>>shift)+1;
        
        /// \brief null, or the symbols at this level
        T** Symbols;
        
        symbol_level() {
            DefaultSymbol   = symbol_set::null;
            Symbols         = NULL;
        }
        
        symbol_level(const symbol_level& copyFrom) {
            DefaultSymbol   = copyFrom.DefaultSymbol;
            Symbols         = NULL;
            
            if (copyFrom.Symbols) {
                Symbols = new T*[c_MaxIndex];
                
                for (int x=0; x<c_MaxIndex; x++) {
                    if (copyFrom.Symbols[x]) {
                        Symbols[x] = new T(*copyFrom.Symbols[x]);
                    } else {
                        Symbols[x] = NULL;
                    }
                }
            }
        }
        
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
            
            Symbols[index]      = new T();
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
                Symbols = new T*[c_MaxIndex];
                for (int index=0; index < c_MaxIndex; index++) {
                    Symbols[index] = NULL;
                }
            }
            
            // Otherwise, create new entries (if needed) and add the range there as well
            for (int index = startIndex; index <= endIndex; index++) {
                // Create a new entry if needed
                if (!Symbols[index]) Symbols[index] = new T();
                
                // Add the range there (with an updated base value)
                Symbols[index]->add_range(base + (index << shift), range, symbol);
            }
        }
        
        /// \brief Looks up a value in this table
        inline int Lookup(int val) const {
            if (!Symbols) return DefaultSymbol;
            const T* next = Symbols[(val&mask)>>shift];
            if (!next) return DefaultSymbol;
            
            return next->Lookup(val);
        }
        
        /// \brief The size of this item in bytes (used when computing the required size in memory of a particular lexer)
        inline size_t size() const {
            size_t mySize = sizeof(symbol_level<T, mask, shift>);
            if (Symbols) {
                mySize += sizeof(T*[c_MaxIndex]);
                for (int index=0; index<c_MaxIndex; index++) {
                    if (Symbols[index]) mySize += Symbols[index]->size();
                }
            }
            return mySize;
        }
    };
    
    /// \brief Structure representing a level of symbols stored in this translator
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
        inline int Lookup(int val) const {
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
    };
    
    ///
    /// \brief Default format of a symbol table
    ///
    /// (Single, level, can handle 255 symbols of a generic type)
    template<class T> struct symbol_table {
        symbol_level<int, 0xff, 0> Table;
        
        inline int Lookup(T val) const {
            return Table.Lookup(val);
        }
        
        inline void add_range(const range<int>& range, int symbol) {
            Table.add_range(0, range, symbol);
        }
    };
    
    /// \brief Character table
    template<> struct symbol_table<unsigned char> {
        symbol_level<int, 0xff, 0> Table;

        inline int Lookup(unsigned char val) const {
            return Table.Lookup((int)val);
        }
        
        inline void add_range(const range<int>& range, int symbol) {
            Table.add_range(0, range, symbol);
        }
    };
    
    /// \brief Character table
    template<> struct symbol_table<char> {
        symbol_level<int, 0xff, 0> Table;
        
        inline int Lookup(char val) const {
            return Table.Lookup((int)(unsigned char)val);
        }
        
        inline void add_range(const range<int>& range, int symbol) {
            Table.add_range(0, range, symbol);
        }
    };
    
    /// \brief Unicode character table
    template<> struct symbol_table<wchar_t> {
        symbol_level<symbol_level<int, 0xff, 0>, 0xff00, 8> Table;
        
        inline int Lookup(wchar_t val) const {
            return Table.Lookup((int)(unsigned)val);
        }
        
        inline void add_range(const range<int>& range, int symbol) {
            Table.add_range(0, range, symbol);
        }
    };
    
    /// \brief Integer character table
    template<> struct symbol_table<int> {
        symbol_level<symbol_level<symbol_level<symbol_level<int, 0xff, 0>, 0xff00, 8>, 0xff0000, 16>, 0x7f000000, 24> Table;
        
        inline int Lookup(int val) const {
            return Table.Lookup(val);
        }
        
        inline void add_range(const range<int>& range, int symbol) {
            Table.add_range(0, range, symbol);
        }
    };
}

#endif
