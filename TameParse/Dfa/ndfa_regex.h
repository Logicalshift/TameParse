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

#include "TameParse/Dfa/ndfa.h"

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
        /// \brief Set to true if the compiler should construct unicode surrogate sequences for characters >0xffff
        bool m_ConstructSurrogates;

        /// \brief If true, then any regexes are added in a case insensitive manner
        bool m_CaseInsensitive;

        /// \brief Maps expressions to regular expressions
        std::map<symbol_string, symbol_string> m_ExpressionMap;

        /// \brief Maps expressions to literal strings
        std::map<symbol_string, symbol_string> m_LiteralExpressionMap;
        
    public:
        /// \brief Constructs an empty NDFA
        ndfa_regex();
        
        /// \brief Conversion constructor
        ndfa_regex(const ndfa& copyFrom);
        
        /// \brief Copy constructor
        ndfa_regex(const ndfa_regex& copyFrom);
        
    public:
        /// \brief Converts a string to a symbol_string
        static symbol_string convert(const std::string& source);
        
        /// \brief Converts a wstring to a symbol_string
        static symbol_string convert(const std::wstring& source);

        /// \brief Converts a null-terminated string to a symbol_string
        static symbol_string convert(char* source);
        
        /// \brief Converts a null-terminated to a symbol_string
        static symbol_string convert(wchar_t* source);

        /// \brief Converts a symbol string into a wstring
        static std::wstring convert_syms(const symbol_string& source);

    public:
        /// \brief Compiles a regular expression starting at the specified state, returning the final state
        int add_regex(builder& cons, const symbol_string& regex);

        /// \brief Compiles a regular expression starting at the specified state, returning the final state
        int add_regex(int initialState, const symbol_string& regex);
        
        /// \brief Compiles a regular expression starting at the specified state, returning the final state and adding an accepting action
        inline int add_regex(int initialState, const symbol_string& regex, const accept_action& action) {
            int finalState = add_regex(initialState, regex);
            accept(finalState, action);
            return finalState;
        }
        
        /// \brief Compiles a regular expression starting at the specified state, returning the final state
        inline int add_regex(int initialState, const std::string& regex) {
            return add_regex(initialState, convert(regex));
        }

        /// \brief Compiles a regular expression starting at the specified state, returning the final state
        inline int add_regex(int initialState, std::wstring regex) {
            return add_regex(initialState, convert(regex));
        }

        /// \brief Compiles a regular expression starting at the specified state, returning the final state
        inline int add_regex(int initialState, const std::string& regex, const accept_action& action) {
            return add_regex(initialState, convert(regex), action);
        }
        
        /// \brief Compiles a regular expression starting at the specified state, returning the final state
        inline int add_regex(int initialState, const std::wstring& regex, const accept_action& action) {
            return add_regex(initialState, convert(regex), action);
        }
        
        /// \brief Compiles an NDFA that matches a literal string starting at the specified state, returning the final state
        int add_literal(int initialState, const symbol_string& literal);

        /// \brief Compiles a regular expression starting at the specified state, returning the final state and adding an accepting action
        inline int add_literal(int initialState, const symbol_string& regex, const accept_action& action) {
            int finalState = add_literal(initialState, regex);
            accept(finalState, action);
            return finalState;
        }
        
        /// \brief Compiles a regular expression starting at the specified state, returning the final state
        inline int add_literal(int initialState, const std::string& regex) {
            return add_literal(initialState, convert(regex));
        }
        
        /// \brief Compiles a regular expression starting at the specified state, returning the final state
        inline int add_literal(int initialState, const std::wstring& regex) {
            return add_literal(initialState, convert(regex));
        }
        
        /// \brief Compiles a regular expression starting at the specified state, returning the final state
        inline int add_literal(int initialState, const std::string& regex, const accept_action& action) {
            return add_literal(initialState, convert(regex), action);
        }
        
        /// \brief Compiles a regular expression starting at the specified state, returning the final state
        inline int add_literal(int initialState, const std::wstring& regex, const accept_action& action) {
            return add_literal(initialState, convert(regex), action);
        }

    public:
        /// \brief Sets whether or not this regular expression builder should generate unicode surrogate characters
        ///
        /// If this is set to true then characters >0xffff will be substituted with their unicode surrogate equivalent.
        /// This is useful if the parser is expected to deal with UTF-16 characters. For cases where UCS-32 is in use,
        /// this should be set to false.
        ///
        /// By default, this is turned on, as 16-bit unicode characters are far more common.
        inline void set_use_surrogates(bool useSurrogates) { m_ConstructSurrogates = useSurrogates; }

        /// \brief Sets whether or not the regular expressions should be treated as case-insensitive
        inline void set_case_insensitive(bool caseInsensitive) { m_CaseInsensitive = caseInsensitive; }

        /// \brief Defines a new expression as a regular expression
        ///
        /// When a regular expression contains {name}, it will be substituted for the supplied value. Subclasses can 
        /// change this behaviour by overriding the compile_expression function.
        void define_expression(const symbol_string& name, const symbol_string& value);

        /// \brief Defines a new expression as a literal string
        ///
        /// When a regular expression contains {name}, it will be substituted for the supplied value. Subclasses can 
        /// change this behaviour by overriding the compile_expression function.
        ///
        /// Unlike the define_expression call, the value given here is literal rather than a regular expression.
        void define_expression_literal(const symbol_string& name, const symbol_string& value);

        /// \brief Defines a new expression
        ///
        /// When a regular expression contains {name}, it will be substituted for the supplied value. Subclasses can 
        /// change this behaviour by overriding the compile_expression funciton.
        inline void define_expression(const std::wstring& name, const std::wstring& value) {
            define_expression(convert(name), convert(value));
        }

        /// \brief Defines a new expression
        ///
        /// When a regular expression contains {name}, it will be substituted for the supplied value. Subclasses can 
        /// change this behaviour by overriding the compile_expression funciton.
        inline void define_expression(const std::string& name, const std::string& value) {
            define_expression(convert(name), convert(value));
        }

        /// \brief Defines a new expression as a literal string
        ///
        /// When a regular expression contains {name}, it will be substituted for the supplied value. Subclasses can 
        /// change this behaviour by overriding the compile_expression function.
        ///
        /// Unlike the define_expression call, the value given here is literal rather than a regular expression.
        inline void define_expression_literal(const std::wstring& name, const std::wstring& value) {
            define_expression_literal(convert(name), convert(value));
        }

        /// \brief Defines a new expression as a literal string
        ///
        /// When a regular expression contains {name}, it will be substituted for the supplied value. Subclasses can 
        /// change this behaviour by overriding the compile_expression function.
        ///
        /// Unlike the define_expression call, the value given here is literal rather than a regular expression.
        inline void define_expression_literal(const std::string& name, const std::string& value) {
            define_expression_literal(convert(name), convert(value));
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
        virtual void compile(symbol_string::const_iterator& pos, const symbol_string::const_iterator& end, builder& cons);

        ///
        /// \brief Compiles the value of a {} expression
        ///
        /// The '{}' operator in a regular expression generally indicates a subexpression macro. The default implementation
        /// of this will first look in the expression map for a regular expression, and compile that at the current position
        /// if it finds one. If it does not find an expression, it will look up the expression as a unicode character range.
        ///
        /// Returns true if the expression was successfully compiled.
        ///
        virtual bool compile_expression(const symbol_string& expression, builder& cons);
        
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
