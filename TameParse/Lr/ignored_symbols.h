//
//  ignored_symbols.h
//  Parse
//
//  Created by Andrew Hunter on 21/05/2011.
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

#ifndef _LR_IGNORED_SYMBOLS_H
#define _LR_IGNORED_SYMBOLS_H

#include <set>

#include "TameParse/Lr/action_rewriter.h"
#include "TameParse/ContextFree/item.h"

namespace lr {
    ///
    /// \brief Action rewriter that adds 'ignored' symbols to a set of actions
    ///
    /// It is very common for a language to have lexical items that have no syntactic meaning. For example, most
    /// languages do not attribute any syntactic meaning to whitespace or comments.
    ///
    /// This class makes it possible to specify which lexical items should be ignored by the parser. These items
    /// can appear anywhere in the source file and will not affect the outcome of the parse.
    ///
    /// An extension to the usual way that these symbols are treated by the parser is that parser states that have
    /// actions referencing these symbols will *not* ignore them (hence, they will have syntactic relevance only
    /// in these states). This makes it much simpler to design languages where certain lexical items are usually
    /// ignored, but have meaning in specific contexts. Examples of this might be literate languages where
    /// comments in particular situations are interesting, or languages that use newlines to end statements.
    ///
    /// If an 'ignored' symbol is used in a situation where it will cause a reduction, the reduction is changed to
    /// a weak reduce operation, so that the reduction will not take place in situations where it would make the
    /// result is invalid. (A possible future enhancement would be to detect the situations where this can't occur
    /// and use a strong reduce instead)
    ///
    class ignored_symbols : public action_rewriter {
    public:
        /// \brief A set of symbols
        typedef std::set<contextfree::item_container> item_set;

    private:
        /// \brief The set of items that should be considered as being ignored
        ///
        /// These should generally only be terminal items, it doesn't make sense to ignore a nonterminal in the
        /// current parser design.
        item_set m_IgnoredItems;
        
    public:
        /// \brief Creates a new ignored symbols rewriter
        ignored_symbols();
        
        /// \brief Copy constructor
        ignored_symbols(const ignored_symbols& copyFrom);
        
        /// \brief Destructor
        virtual ~ignored_symbols();
        
        /// \brief Adds a new ignored item to this object
        ///
        /// The new item should be a terminal, as it doesn't make sense in the current parser design to ignore
        /// other item types.
        void add_item(const contextfree::item_container& newItem);
        
        /// \brief Adds a new item to this object
        inline void add_item(const contextfree::terminal& newItem) { add_item(contextfree::item_container(newItem)); }
        
        /// \brief Modifies the specified set of actions according to the rules in this rewriter
        virtual void rewrite_actions(int state, lr_action_set& actions, const lalr_builder& builder) const;
        
        /// \brief Creates a clone of this rewriter
        virtual action_rewriter* clone() const;
    };
}

#endif
