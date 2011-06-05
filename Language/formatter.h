//
//  formatter.h
//  Parse
//
//  Created by Andrew Hunter on 30/05/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LANGUAGE_FORMATTER_H
#define _LANGUAGE_FORMATTER_H

#include <string>

namespace contextfree {
    class terminal;
    class terminal_dictionary;
    class nonterminal;
    class grammar;
    class rule;
    class item;
    class guard;
}

namespace lr {
    class lr0_item;
    class lalr_state;
    class lalr_machine;
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
        
        /// \brief Turns a rule into a string, with an optional 'dot position'
        static std::wstring to_string(const contextfree::rule& rule, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict, int dotPos = -1, bool showNonterminal = true);

        /// \brief Turns a grammar into a (large) string
        static std::wstring to_string(const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict);
        
    public:
        /// \brief Turns a LR(0) item into a string
        static std::wstring to_string(const lr::lr0_item& item, const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict);
        
        /// \brief Turns a LALR state into a string
        static std::wstring to_string(const lr::lalr_state& state,  const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict);
        
        /// \brief Turns a LALR state machine into an enormous string
        static std::wstring to_string(const lr::lalr_machine& machine,  const contextfree::grammar& gram, const contextfree::terminal_dictionary& dict);
    };
}

#endif
