//
//  symbol_table.h
//  Parse
//
//  Created by Andrew Hunter on 28/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_SYMBOL_TABLE_H
#define _DFA_SYMBOL_TABLE_H

#include <cstdlib>
#include "TameParse/Dfa/symbol_map.h"

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
        /// \brief Converts this table to a table suitable for use with the hard_coded_symbol_table class
        inline int* to_hard_coded_table(size_t& size) {
            // If no symbols are defined at this level, then generate an empty table
            if (!Symbols) {
                size = 2;
                int* result = new int[2];
                result[0] = DefaultSymbol;
                result[1] = 0;
                return result;
            }

            // Create the set of subtables
            size_t  tableSizes[c_MaxIndex];
            int*    subTables[c_MaxIndex];

            int lowestUsed  = c_MaxIndex;
            int highestUsed = 0;
            int totalSize   = 0;

            for (int subtableId = 0; subtableId < c_MaxIndex; subtableId++) {
                // Add a null table if no symbols are defined for this set
                if (!Symbols[subtableId]) {
                    subTables[subtableId] = NULL;
                    continue;
                }

                // Update the lowest and highest used tables
                if (subtableId < lowestUsed)    lowestUsed  = subtableId;
                if (subtableId >= highestUsed)  highestUsed = subtableId+1;

                // Add the subtable for this symbol ID
                subTables[subtableId] = Symbols[subtableId]->to_hard_coded_table(tableSizes[subtableId]);

                // Work out the size of this entry and add it to the total size
                // (table size is 2 + the number)
                totalSize += tableSizes[subtableId];
            }

            // Allocate space for this table
            size = 2 + (highestUsed - lowestUsed) + totalSize;
            int*    newTable    = new int[size];

            // Position for the next subtable in the result
            size_t  copyPos     = 2 + (highestUsed - lowestUsed);

            // Fill in the header
            newTable[0] = DefaultSymbol;
            newTable[1] = lowestUsed | (highestUsed << 8);

            // Fill in the table list (and delete the subtables as we go)
            for (int subtableId = lowestUsed; subtableId < highestUsed; subtableId++) {
                // Position for this table offset
                int pos = 2 + (subtableId - lowestUsed);

                if (!subTables[subtableId]) {
                    // Default value
                    newTable[pos] = -1;
                    continue;
                }

                // Position is the current copyPos
                newTable[pos] = (int) copyPos;

                // Copy the table into the result
                memcpy(newTable + copyPos, subTables[subtableId], sizeof(int) * tableSizes[subtableId]);

                // Move on
                copyPos += tableSizes[subtableId];

                // No longer need this table
                delete[] subTables[subtableId];
                subTables[subtableId] = NULL;
            }

            // Return the new table
            return newTable;
        }
        
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
                mySize += sizeof(int)*c_MaxIndex;
            }
            return mySize;
        }

    public:
        /// \brief Converts this table to a table suitable for use with the hard_coded_symbol_table class
        inline int* to_hard_coded_table(size_t& size) {
            // If no symbols are defined at this level, then generate an empty table
            if (!Symbols) {
                size = 2;
                int* result = new int[2];
                result[0] = DefaultSymbol;
                result[1] = 0;
                return result;
            }

            // Work out the highest and lowest used symbols
            int lowestUsed  = c_MaxIndex;
            int highestUsed = 0;

            for (int symbolId = 0; symbolId < c_MaxIndex; symbolId++) {
                // Counts as not used if the symbol is the same as the default symbol
                if (Symbols[symbolId] == DefaultSymbol) {
                    continue;
                }

                // Update the lowest and highest used tables
                if (symbolId < lowestUsed)    lowestUsed  = symbolId;
                if (symbolId >= highestUsed)  highestUsed = symbolId+1;
            }

            // Allocate space for this table
            size = 2 + (highestUsed - lowestUsed);
            int*    newTable    = new int[size];

            // Fill in the header
            newTable[0] = DefaultSymbol;
            newTable[1] = lowestUsed | (highestUsed << 8);

            // Fill in the symbols
            for (int symbolId = lowestUsed; symbolId < highestUsed; symbolId++) {
                // Position for this table offset
                int pos = 2 + (symbolId - lowestUsed);

                // Copy in the symbol ID
                newTable[pos] = Symbols[symbolId];
            }

            // Return the new table
            return newTable;
        }
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
        /// \brief The internal table
        table_type Table;
        
        /// \brief Returns the set that the specified symbol is in
        inline int lookup(symbol_type val) const {
            return Table.lookup(val);
        }
        
        /// \brief Adds a new symbol to this table
        inline void add_range(const range<int>& range, int symbol) {
            Table.add_range(0, range, symbol);
        }
    };
}

#endif
