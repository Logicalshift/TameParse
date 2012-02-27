//
//  formatter.h
//  Parse
//
//  Created by Andrew Hunter on 30/05/2011.
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

#ifndef _LANGUAGE_FORMATTER_H
#define _LANGUAGE_FORMATTER_H

#include <set>
#include <string>

#include "TameParse/ContextFree/item.h"

namespace util {
    class astnode;
}

namespace contextfree {
    class terminal;
    class terminal_dictionary;
    class nonterminal;
    class grammar;
    class rule;
    class guard;
}

namespace lr {
    class lr0_item;
    class lr1_item;
    class lalr_state;
    class lalr_machine;
    class lr_action;
    class lalr_builder;
    class conflict;
}

namespace language {
    ///
    /// \brief Methods for formatting language items for display
    ///
    class formatter {
    public:
        /// \brief Turns a terminal into a string
        static std::wstring to_string(const contextfree::terminal& term, const contextfree::terminal_dictionary& dict);

        /// \brief Turns a nonterminal into a string
        static std::wstring to_string(const contextfree::nonterminal& nt, const contextfree::grammar& gram);
        
        /// \brief Turns a guard item into a string
        static std::wstring to_string(const contextfree::guard& nt, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict);
        
        /// \brief Turns an item into a string
        static std::wstring to_string(const contextfree::item& it, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict);
        
        /// \brief Turns an item set into a string
        static std::wstring to_string(const contextfree::item_set& it, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict);
        
        /// \brief Turns a rule into a string, with an optional 'dot position'
        static std::wstring to_string(const contextfree::rule& rule, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict, int dotPos = -1, bool showNonterminal = true);

        /// \brief Turns a grammar into a (large) string
        static std::wstring to_string(const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict);
        
    public:
        /// \brief Turns a LR(0) item into a string
        static std::wstring to_string(const lr::lr0_item& item, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict);

        /// \brief Turns a LR(1) item into a string
        static std::wstring to_string(const lr::lr1_item& item, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict);

        /// \brief Turns a LALR state into a string
        static std::wstring to_string(const lr::lalr_state& state,  const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict, bool showClosure = false);
        
        /// \brief Turns a LR action into a string
        static std::wstring to_string(const lr::lr_action& act, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict);
        
        /// \brief Turns a LALR state machine into an enormous string
        static std::wstring to_string(const lr::lalr_machine& machine,  const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict, bool showClosure = false);
        
        /// \brief Turns a LALR builder into an enormous string
        static std::wstring to_string(const lr::lalr_builder& builder, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict, bool showClosure = false);
        
    public:
        /// \brief Turns a LALR conflict into a string description
        static std::wstring to_string(const lr::conflict& conflict, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict);
        
    public:
        /// \brief Turns a AST node into a string description
        static std::wstring to_string(const util::astnode& node, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict);
    };
}

#endif
