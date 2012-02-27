//
//  standard_items.h
//  Parse
//
//  Created by Andrew Hunter on 30/04/2011.
//  
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

#ifndef _CONTEXTFREE_STANDARD_ITEMS_H
#define _CONTEXTFREE_STANDARD_ITEMS_H

#include "TameParse/ContextFree/item.h"

namespace contextfree {
    ///
    /// \brief Item representing a terminal symbol
    ///
    class terminal : public item {
    public:
        /// \brief Creates a terminal that matches the specified symbol
        terminal(int sym);
        
        /// \brief Creates a clone of this item
        virtual item* clone() const;

        /// \brief The type of this item
        virtual kind type() const;
        
        /// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
        ///
        /// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
        /// set for the rules that make them up.
        ///
        /// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
        /// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
        virtual item_set first(const grammar& gram) const;
    };

    ///
    /// \brief Item representing a non-terminal symbol
    ///
    class nonterminal : public item {
    private:
        nonterminal(const item& noCopying);
        
    public:
        /// \brief Creates a non-terminal item that matches the specified symbol
        nonterminal(int sym);
        
        /// \brief Creates a clone of this item
        virtual item* clone() const;
        
        /// \brief The type of this item
        virtual kind type() const;
        
        /// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
        ///
        /// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
        /// set for the rules that make them up.
        ///
        /// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
        /// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
        virtual item_set first(const grammar& gram) const;
        
        /// \brief Computes the closure of this item
        virtual void closure(const lr::lr1_item& item, lr::lr1_item_set& state, const grammar& gram) const;
    };
    
    ///
    /// \brief The empty item
    ///
    class empty_item : public item {
    public:
        /// \brief Creates a terminal that matches the specified symbol
        empty_item();
        
        /// \brief Compares this item to another. Returns true if they are the same
        virtual bool operator==(const item& compareTo) const;
        
        /// \brief Orders this item relative to another item
        virtual bool operator<(const item& compareTo) const;
        
        /// \brief Creates a clone of this item
        virtual item* clone() const;
        
        /// \brief The type of this item
        virtual kind type() const;
        
        /// \brief Computes the closure of this item
        virtual void closure(const lr::lr1_item& item, lr::lr1_item_set& state, const grammar& gram) const;
        
        /// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
        ///
        /// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
        /// set for the rules that make them up.
        ///
        /// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
        /// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
        virtual item_set first(const grammar& gram) const;
        
        /// \brief True if a transition (new state) should be generated for this item
        virtual bool generate_transition() const;
    };
    
    ///
    /// \brief The end-of-input item
    ///
    class end_of_input : public item {
    public:
        /// \brief Creates a terminal that matches the specified symbol
        end_of_input();
        
        /// \brief Compares this item to another. Returns true if they are the same
        virtual bool operator==(const item& compareTo) const;
        
        /// \brief Orders this item relative to another item
        virtual bool operator<(const item& compareTo) const;
        
        /// \brief Creates a clone of this item
        virtual item* clone() const;
        
        /// \brief The type of this item
        virtual kind type() const;
        
        /// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
        ///
        /// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
        /// set for the rules that make them up.
        ///
        /// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
        /// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
        virtual item_set first(const grammar& gram) const;
    };
    
    ///
    /// \brief The end-of-guard item
    ///
    /// This item works similarly to the 'end of input' item except that it matches any symbol. It's used at the end
    /// of guard rules to signify that enough lookahead has been matched. Guards match the shortest possible input
    /// that can be accepted, so this will be followed as soon as the parser reaches a state that it can accept the
    /// rule. (This also makes this usable for accepting partial languages in the middle of a longer input string,
    /// which could perhaps be useful for cases where we want to mix two languages together)
    ///
    class end_of_guard : public item {
    public:
        /// \brief Creates a terminal that matches the specified symbol
        end_of_guard();
        
        /// \brief Compares this item to another. Returns true if they are the same
        virtual bool operator==(const item& compareTo) const;
        
        /// \brief Orders this item relative to another item
        virtual bool operator<(const item& compareTo) const;
        
        /// \brief Creates a clone of this item
        virtual item* clone() const;
        
        /// \brief The type of this item
        virtual kind type() const;
        
        /// \brief Computes the set FIRST(item) for this item (when used in the specified grammar)
        ///
        /// This set will always include the item itself by definition. Things like non-terminals should include themselves and the first
        /// set for the rules that make them up.
        ///
        /// The 'empty' and 'follow' items can be used to create special meaning (empty indicates the first set should be extended to include
        /// anything after in the rule, follow indicates that the first set should also contain any lookahead for the rule)
        virtual item_set first(const grammar& gram) const;
    };
}

#endif
