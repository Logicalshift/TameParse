//
//  symbol_translator.h
//  Parse
//
//  Created by Andrew Hunter on 27/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_SYMBOL_TRANSLATOR_H
#define _DFA_SYMBOL_TRANSLATOR_H

#include "Dfa/symbol_map.h"

namespace dfa {
    ///
    /// \brief Class used in a DFA to map an input symbol to a symbol set ID
    ///
    class symbol_translator {
    private:
        /// \brief Structure representing a level of symbols stored in this translator
        template<class T, unsigned int mask, int shift> struct symbol_level {
            /// \brief Default symbol to use at this level for items that are null
            int DefaultSymbol;
            
            /// \brief Number of entries at this level in the table
            static const int c_MaxIndex = mask>>shift;
            
            /// \brief null, or the symbols at this level
            T* Symbols[c_MaxIndex];
            
            symbol_level() {
                DefaultSymbol   = symbol_set::null;
                
                for (int x=0; x<(mask>>shift); x++) {
                    Symbols[x] = NULL;
                }
            }
            
            symbol_level(const symbol_level& copyFrom) {
                for (int x=0; x<c_MaxIndex; x++) {
                    if (copyFrom.Symbols[x]) {
                        Symbols[x] = new T(*copyFrom.Symbols[x]);
                    } else {
                        Symbols[x] = NULL;
                    }
                }
            }
            
            ~symbol_level() {
                for (int x=0; x<c_MaxIndex; x++) {
                    if (Symbols[x]) delete Symbols[x];
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
                startIndex >>= shift;
                
                // Work out the final index in this table
                int endIndex = range.upper() - base;
                if (endIndex < 0) endIndex = 0;
                endIndex >>= shift;
                
                // If the range entirely covers this item, then set the default symbol and stop
                if (startIndex == 0 && endIndex >= c_MaxIndex) {
                    DefaultSymbol = symbol;
                    return;
                }
                
                if (endIndex >= c_MaxIndex) endIndex = c_MaxIndex;
                
                // Otherwise, create new entries (if needed) and add the range there as well
                for (int index = startIndex; index <= endIndex; index++) {
                    // Create a new entry if needed
                    if (!Symbols[index]) Symbols[index] = new T();
                    
                    // Add the range there (with an updated base value)
                    Symbols[index]->add_range(base + (index << shift), range, symbol);
                }
            }
            
            /// \brief The range of this item
            inline range<int> table_range(int base) {
                return range<int>(base, ((mask>>shift)+1)<<shift);
            }
            
            /// \brief Looks up a value in this table
            inline int Lookup(int val) const {
                T* next = Symbols[(val&mask)>>shift];
                if (!next) return DefaultSymbol;
                
                return next->Lookup(val);
            }
            
            /// \brief The size of this item in bytes (used when computing the required size in memory of a particular lexer)
            inline size_t size() const {
                size_t mySize = sizeof(symbol_level<T, mask, shift>);
                for (int index=0; index<c_MaxIndex; index++) {
                    if (Symbols[index]) mySize += Symbols[index]->size();
                }
                return mySize;
            }
        };

        /// \brief Structure representing a level of symbols stored in this translator
        template<unsigned int mask, int shift> struct symbol_level<int, mask, shift> {
            /// \brief Number of entries at this level in the table
            static const int c_MaxIndex = mask>>shift;

            /// \brief null, or the symbols at this level
            int Symbols[c_MaxIndex];
            
            symbol_level() {
                for (int x=0; x<(mask>>shift); x++) {
                    Symbols[x] = symbol_set::null;
                }                
            }
            
            /// \brief Sets all the symbols in the specified range to the specified symbol. The range must overlap this item.
            void add_range(int base, const range<int>& range, int symbol) {
                // Work out the initial index in this table
                int startIndex = range.lower() - base;
                if (startIndex < 0) startIndex = 0;
                startIndex >>= shift;
                
                // Work out the final index in this table
                int endIndex = range.upper() - base;
                if (endIndex < 0) endIndex = 0;
                endIndex >>= shift;
                                
                if (endIndex >= c_MaxIndex) endIndex = c_MaxIndex;

                // Fill in the symbols between these indexes
                for (int index=startIndex; index < endIndex; index++) {
                    Symbols[index] = symbol;
                }
            }
            
            /// \brief Looks up a value in this table
            inline int Lookup(int val) {
                return Symbols[(val&mask)>>shift];
            }
            
            /// \brief The size of this item
            inline size_t size() const { return sizeof(symbol_level<int, mask, shift>); }
        };

        /// \brief The table for this item
        symbol_level<symbol_level<symbol_level<symbol_level<int, 0xff, 0>, 0xff00, 8>, 0xff0000, 16>, 0x7f000000, 24> m_Table;
        
    public:
        /// \brief Copy constructor
        symbol_translator(const symbol_translator& copyFrom);
        
        /// \brief Creates a translator from a map containing non-overlapping symbol sets
        ///
        /// Use remapped_symbol_map::deduplicate() to create a symbol map that is appropriate for this call, or ndfa::to_ndfa_with_unique_symbols()
        /// to remap an NDFA so that it uses unique symbols.
        symbol_translator(const symbol_map& map);
        
        /// \brief Sets the symbol for the specified range
        inline void add_range(const range<int>& range, int symbol) {
            m_Table.add_range(0, range, symbol);
        }
        
        /// \brief Returns the ID of the symbol set of the specified symbol
        inline int set_for_symbol(int symbol) const {
            return m_Table.Lookup(symbol);
        }
        
        // \brief Returns the number of bytes required by the table
        inline size_t size() const {
            return m_Table.size();
        }
    };
}

#endif
