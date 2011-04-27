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
            
            /// \brief null, or the symbols at this level
            T Symbols[mask>>shift];
            
            /// \brief Looks up a value in this table
            inline int Lookup(int val) {
                T next = Symbols[(val&mask)>>shift];
                if (!next) return DefaultSymbol;
                
                return next->Lookup(val);
            }
        };

        /// \brief Structure representing a level of symbols stored in this translator
        template<unsigned int mask, int shift> struct symbol_level<int, mask, shift> {
            /// \brief Default symbol to use at this level for items that are null
            int DefaultSymbol;
            
            /// \brief null, or the symbols at this level
            int Symbols[mask>>shift];
            
            /// \brief Looks up a value in this table
            inline int Lookup(int val) {
                if (!Symbols) return DefaultSymbol;
                
                return Symbols[(val&mask)>>shift];
            }
        };

        /// \brief The table for this item
        symbol_level<symbol_level<symbol_level<symbol_level<int, 0xff, 0>*, 0xff00, 8>*, 0xff0000, 16>*, 0xff000000, 24> m_Table;
        
    public:
        /// \brief Copy constructor
        symbol_translator(const symbol_translator& copyFrom);
        
        /// \brief Creates a translator from a map containing non-overlapping symbol sets
        ///
        /// Use remapped_symbol_map::deduplicate() to create a symbol map that is appropriate for this call, or ndfa::to_ndfa_with_unique_symbols()
        /// to remap an NDFA so that it uses unique symbols.
        symbol_translator(const symbol_map& map);
        
        /// \brief Returns the ID of the symbol set of the specified symbol
        inline int set_for_symbol(int symbol) {
            return m_Table.Lookup(symbol);
        }
    };
}

#endif
