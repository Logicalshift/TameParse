//
//  lexeme.h
//  Parse
//
//  Created by Andrew Hunter on 28/04/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_LEXEME_H
#define _DFA_LEXEME_H

#include <string>

#include "TameParse/Util/container.h"
#include "TameParse/Dfa/position.h"

namespace dfa {
    ///
    /// \brief Representation of a lexeme (a symbol accepted by a lexer)
    ///
    class lexeme {
    public:
        /// \brief Type representing the symbols in a lexeme (we use an integer string as the basic symbol type of our lexer is int)
        typedef std::basic_string<int> symbols;
        
    private:
        /// \brief The position that this lexeme was at in the source file
        position m_Position;
        
        /// \brief The symbols that make up this lexeme
        symbols m_Symbols;
        
        /// \brief The symbol ID that was matched by this lexeme
        int m_Matched;
        
        /// \brief Disabled assignment
        lexeme& operator=(const lexeme& assignFrom);
        
    public:
        /// \brief Creates a nonsensical empty lexeme
        lexeme();
        
        /// \brief Copy constructor
        lexeme(const lexeme& copyFrom);
        
        /// \brief Creates a new lexeme
        lexeme(const symbols& syms, const position& pos, int matched);
        
        /// \brief Creates a new lexeme from a sequence of symbols
        template<typename iterator_type> lexeme(iterator_type begin, iterator_type end, const position& pos, int matched, size_t length = 0)
        : m_Position(pos)
        , m_Matched(matched)
        , m_Symbols() {
            // Reserve space for the symbols if we can
            if (length != 0) m_Symbols.reserve(length);
            
            // Store the symbols in turn
            for (iterator_type symbol=begin; symbol != end; ++symbol) {
                m_Symbols += (int)*symbol;
            }
        }
        
        /// \brief Destructor
        virtual ~lexeme();
        
        /// \brief Clone operator (so subclasses can store extra data if they need to)
        virtual lexeme* clone() const;
        
        /// \brief Ordering operator
        virtual bool operator<(const lexeme& compareTo) const;
        
        /// \brief Returns true if lexeme a is less than lexeme b
        inline static bool compare(const lexeme* a, const lexeme* b) {
            if (a == b) return false;
            if (!a) return true;
            if (!b) return false;
            
            return *a < *b;
        }
        
    public:
        /// \brief The ID of the symbol that was matched
        inline int matched() const { return m_Matched; }
        
        /// \brief The content that makes up this lexeme
        inline const symbols& content() const { return m_Symbols; }
        
        /// \brief The initial location of this lexeme
        inline const position& pos() const { return m_Position; }
        
        /// \brief The final position of this lexeme
        ///
        /// Note that the line count will be off by 1 if the symbol preceeding this lexeme is a carriage return
        position final_pos() const;
        
        /// \brief Converts the content to a basic string with a different symbol type
        ///
        /// No encoding is done by this call, so the behaviour is just to convert ints to the symbol type
        template<typename symbol_type> inline std::basic_string<symbol_type> content() const {
            // Create the result and reserve the appropriate amount of space
            std::basic_string<symbol_type> result;
            result.reserve(m_Symbols.size());
            
            // Copy the symbols across, using a simple cast operation
            for (symbols::const_iterator symbol=m_Symbols.begin(); symbol != m_Symbols.end(); ++symbol) {
                result += (symbol_type)*symbol;
            }
            
            return result;
        }
    };
    
    /// \brief Container for a lexeme
    typedef util::container<lexeme> lexeme_container;
}

#endif
