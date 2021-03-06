//
//  hard_coded_symbol_table.h
//  Parse
//
//  Created by Andrew Hunter on 03/09/2011.
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

#ifndef _DFA_HARD_CODED_SYMBOL_TABLE_H
#define _DFA_HARD_CODED_SYMBOL_TABLE_H

#include <cstdlib>

namespace dfa {
    ///
    /// \brief Looks up a symbol at a particular level in the table
    ///
    template<int level>
    inline int hcst_lookup_sym(const int* table, int offset, unsigned int symbol) {
        // Get the word containing the low/high values
        int lowHigh = table[offset + 1];
        
        // Get the lowest and highest character for this point in the table
        int lowest  = lowHigh&0xff;
        int highest = lowHigh>>8;
        
        // Return the default value if the symbol is out of range
        int pos         = (((unsigned int) symbol) >> (level * 8))&0xff;
        if ((int) pos < lowest || (int) pos >= highest) return table[offset + 0];
        
        // Look up the next level
        int nextOffset  = table[offset + 2 + (pos - lowest)];
        
        // Use the default symbol if the offset is -1
        if (nextOffset == -1) return table[offset + 0];
        
        // Look up the next level
        return hcst_lookup_sym<level-1>(table, offset + nextOffset, symbol);
    }
    
    ///
    /// \brief Lowest-level lookup for the hard-coded symbol table
    ///
    template<> 
    inline int hcst_lookup_sym<0>(const int* table, int offset, unsigned int symbol) {
        // Get the word containing the low/high values
        int lowHigh = table[offset + 1];
        
        // Get the lowest and highest character for this point in the table
        int lowest  = lowHigh&0xff;
        int highest = lowHigh>>8;
        
        // Return the default value if the symbol is out of range
        if ((int) symbol < lowest || (int) symbol >= highest) return table[offset + 0];
        
        // Look up the symbol set
        int pos         = ((unsigned int) symbol&0xff);
        int symbolSet   = table[offset + 2 + (pos - lowest)];
        
        // Use the default symbol if the offset is -1
        if (symbolSet == -1) return table[offset + 0];
        
        // Otherwise use the value at this point in the table
        return symbolSet;
    }

    ///
    /// \brief Class that represents a hard-coded symbol translator table
    ///
    /// This is used with a static int array generated by the parser generator,
    /// and performs equivalent actions to the standard symbol table.
    ///
    /// The char_type parameter indicates the type of symbol that will be passed
    /// into this table. char_size indicates the number of significant bytes in
    /// the symbol.
    ///
    /// The table format is an array of integers, consisting of several layers
    /// of tables. Each layer represents how successively lower bytes of the
    /// symbol should be looked up: ie, the first layer represents how the upper
    /// 8 bits of the symbol are translated, the second the next 8 bits and so
    /// on until the entire character has been translated.
    ///
    /// The format for all layers except the bottom layer is as follows:
    ///     * 1 int  = default set for unknown characters
    ///     * 1 int  = (lowest_used | (highest_used<<8))
    ///     * n ints = -1 for the default set, or the offset of the table for the next layer, relative to this layer
    /// The bottom layer is the same, except that the values are the actual symbol set
    ///
    template<typename char_type, size_t char_size> class hard_coded_symbol_table {
    private:
        /// \brief The hard-coded symbol table
        const int* m_Table;

    public:
        /// \brief Constructs a new hard-coded symbol table with the specified table
        explicit hard_coded_symbol_table(const int* table)
        : m_Table(table) { }

        /// \brief Returns the symbol set for a particular character
        inline int lookup(char_type symbol) const {
            return hcst_lookup_sym<char_size-1>(m_Table, 0, (unsigned int) symbol);
        }
    };
}

#endif
