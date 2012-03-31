//
//  terminal_dictionary.cpp
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

#include "TameParse/ContextFree/terminal_dictionary.h"

using namespace contextfree;

/// \brief Creates a new terminal dictionary
terminal_dictionary::terminal_dictionary()
: m_MaxSymbol(0) {
}

/// \brief Adds a new anonymous symbol and returns its identifier
int terminal_dictionary::add_symbol() {
    // Create a new maximum symbol
    int newSymbol = m_MaxSymbol;
    ++m_MaxSymbol;
    return newSymbol;
}

/// \brief Adds a named symbol and returns its identifier
int terminal_dictionary::add_symbol(const std::wstring& name) {
    // Add a new symbol
    int newSymbol = add_symbol();
    
    // Associate the name with this symbol
    m_NameToSymbol[name]        = newSymbol;
    m_SymbolToName[newSymbol]   = name;
    
    return newSymbol;
}

/// \brief Adds a new symbol with the specified identifier (which must be unused)
void terminal_dictionary::add_symbol(const std::wstring& name, int value) {
    // Update the max symbol if needed
    if (value >= m_MaxSymbol) m_MaxSymbol = value + 1;
    
    // Associate this name with this symbol
    m_NameToSymbol[name]    = value;
    m_SymbolToName[value]   = name;
}

/// \brief Splits the symbol with the specified identifier, adding a new symbol and marking the original as its parent
///
/// If the symbol was already split off from another, then the split is actually added to the 'parent' symbol rather
/// than the supplied symbol
int terminal_dictionary::split(int symbol) {
    // Get the parent ID for this symbol (we always split off of the parent, so there is only ever one level of 'split' symbols)
    int parentId = parent_of(symbol);

    // Create a new symbol to represent the newly split symbol
    int newSymbol = add_symbol();
    
    // Add to the split, if the parent isn't -1
    if (parentId >= 0) {
        m_ParentFor[newSymbol] = parentId;
        m_ChildrenFor[parentId].insert(newSymbol);
    }
    
    // Return the new symbol
    return newSymbol;
}
