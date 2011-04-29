//
//  ndfa_regex.h
//  Parse
//
//  Created by Andrew Hunter on 27/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_NDFA_REGEX_H
#define _DFA_NDFA_REGEX_H

#include <string>
#include <map>

#include "Dfa/ndfa.h"

namespace dfa {
    ///
    /// \brief String of symbols that can act as input to a regular expression
    ///
    /// Internally, these state machines use positive 32-bit integers as input, so we express the regular expressions this way as well.
    ///
    typedef std::basic_string<int> symbol_string;
    
    ///
    /// \brief NDFA subclass used for building NDFAs out of regular expressions
    ///
    class ndfa_regex : public ndfa {
    private:
        
    public:
        /// \brief Constructs an empty NDFA
        ndfa_regex();
        
        /// \brief Conversion constructor
        ndfa_regex(const ndfa& copyFrom);
        
        /// \brief Copy constructor
        ndfa_regex(const ndfa_regex& copyFrom);
        
    public:
        /// \brief Converts a string to a symbol_string
        static symbol_string convert(std::string source);
        
        /// \brief Converts a wstring to a symbol_string
        static symbol_string convert(std::wstring source);

        /// \brief Converts a null-terminated string to a symbol_string
        static symbol_string convert(char* source);
        
        /// \brief Converts a null-terminated to a symbol_string
        static symbol_string convert(wchar_t* source);

    public:
        /// \brief Compiles a regular expression starting at the specified state, returning the final state
        int add_regex(int initialState, const symbol_string& regex);
        
        /// \brief Compiles a regular expression starting at the specified state, returning the final state and adding an accepting action
        inline int add_regex(int initialState, const symbol_string& regex, const accept_action& action) {
            int finalState = add_regex(initialState, regex);
            accept(finalState, action);
            return finalState;
        }
        
        /// \brief Compiles a regular expression starting at the specified state, returning the final state
        inline int add_regex(int initialState, std::string regex) {
            return add_regex(initialState, convert(regex));
        }

        /// \brief Compiles a regular expression starting at the specified state, returning the final state
        inline int add_regex(int initialState, std::wstring regex) {
            return add_regex(initialState, convert(regex));
        }

        /// \brief Compiles a regular expression starting at the specified state, returning the final state
        inline int add_regex(int initialState, std::string regex, const accept_action& action) {
            return add_regex(initialState, convert(regex), action);
        }
        
        /// \brief Compiles a regular expression starting at the specified state, returning the final state
        inline int add_regex(int initialState, std::wstring regex, const accept_action& action) {
            return add_regex(initialState, convert(regex), action);
        }

    protected:
        ///
        /// \brief Compiles a single symbol from a regular expression
        ///
        /// Subclasses can override this to extend the grammar accepted by the regular expression.
        /// This class should update the supplied iterator and NDFA constructor object with the position of the next item.
        ///
        /// Note that this will be called with pos == end at the end of the regular expression. Implementations should not
        /// inspect the contents of pos in this case, but should instead perform any tidying up that's required at the end
        /// of a regular expression.
        ///
        virtual void compile(symbol_string::const_iterator& pos, const symbol_string::const_iterator& end, constructor& cons);
        
        ///
        /// \brief Returns the symbol at the specified position. Updates pos to point to the last character that makes up this symbol
        ///
        /// This translates quoted symbols like '\n' to their symbolic equivalent (0x0a in this case). Subclasses can override this to
        /// add more quoting behaviour to the default.
        ///
        virtual int symbol_for_sequence(symbol_string::const_iterator& pos, const symbol_string::const_iterator& end);
    };
}

#endif
