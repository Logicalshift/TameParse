//
//  symbol_set.h
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _DFA_SYMBOL_SET_H
#define _DFA_SYMBOL_SET_H

#include <set>

#include "Dfa/range.h"

namespace dfa {
    ///
    /// \brief Class representing a set of symbols
    ///
    /// A symbol is represented as an integer value.
    ///
    class symbol_set {
    public:
        /// \brief Range of symbols
        typedef range<int> symbol_range;
        
    private:
        /// \brief Maximum possible symbol
        static const int c_MaxSymbol = 0x7fffffff;
        
        /// \brief Comparator functor for the symbol store set. 
        /// 
        /// We compare only by the lower bounds (so any range with the same lower bounds is considered equal; this works as we store a set of 
        /// non-overlapping ranges)
        class sort_by_lower {
        public:
            inline bool operator()(const symbol_range& a, const symbol_range& b) {
                return a.lower() < b.lower();
            }
        };

        /// \brief Type of a set of symbols
        typedef std::set<symbol_range, sort_by_lower> symbol_store;
        
        /// \brief The symbols in this set
        symbol_store m_Symbols;
        
    public:
        /// \brief Creates an empty symbol set
        symbol_set();
        
        /// \brief Creates set containing a range of symbols
        symbol_set(const symbol_range& symbol);
        
        /// \brief Creates a new symbol set by copying an old one
        symbol_set(const symbol_set& copyFrom);
        
        /// \brief Destructor
        virtual ~symbol_set();
        
    public:
        /// \brief Merges this symbol set with another
        symbol_set& operator|=(const symbol_set& mergeWith);
        
        /// \brief Merges this symbol set with a range of symbols
        symbol_set& operator|=(const symbol_range& mergeWith);
        
        /// \brief Restricts this set to the symbols common between two sets
        inline symbol_set& operator&=(const symbol_set& andWith) {
            exclude(~andWith);
            return *this;
        }
        
        /// \brief Restricts this set to the symbols common between two sets
        inline symbol_set& operator&=(const symbol_range& andWith) {
            symbol_set notRange(andWith);
            notRange.invert();
            exclude(notRange);
            return *this;
        }
        
        /// \brief Excludes a range of symbols from this set
        void exclude(const symbol_set& exclude);

        /// \brief Excludes a range of symbols from this set
        void exclude(const symbol_range& exclude);
        
        /// \brief Inverts this set
        void invert();
        
        /// \brief Returns an inverted set
        inline symbol_set operator~() const { symbol_set result = *this; result.invert(); return result; }
        
        /// \brief Merges this symbol set with another
        inline symbol_set operator|(const symbol_set& mergeWith) const { symbol_set result = *this; result |= mergeWith; return result; }
        
        /// \brief Merges this symbol set with another
        inline symbol_set operator|(const symbol_range& mergeWith) const { symbol_set result = *this; result |= mergeWith; return result; }
        
        /// \brief Restricts this set to the symbols common between two sets
        inline symbol_set operator&(const symbol_set& andWith) const { symbol_set result = *this; result &= andWith; return result; }

        /// \brief Restricts this set to the symbols common between two sets
        inline symbol_set operator&(const symbol_range& andWith) const { symbol_set result = *this; result &= andWith; return result; }
        
        /// \brief Excludes a range of symbols from this set
        inline symbol_set excluding(const symbol_set& toExclude) const { symbol_set result = *this; result.exclude(toExclude); return result; }

        /// \brief Excludes a range of symbols from this set
        inline symbol_set excluding(const symbol_range& toExclude) const { symbol_set result = *this; result.exclude(toExclude); return result; }
        
    public:
        /// \brief True if the specified symbol is in this set
        bool operator[](int symbol);
        
        /// \brief Determines if this set represents the same as another set
        bool operator==(const symbol_set& compareTo) const;
        
        /// \brief Orders this symbol set
        bool operator<(const symbol_set& compareTo) const;
        
        /// \brief Orders this symbol set
        bool operator<=(const symbol_set& compareTo) const;
        
        /// \brief Orders this symbol set
        inline bool operator!=(const symbol_set& compareTo) const { return !operator==(compareTo); }
        
        /// \brief Orders this symbol set
        inline bool operator>(const symbol_set& compareTo) const { return !operator<=(compareTo); }
        
        /// \brief Orders this symbol set
        inline bool operator>=(const symbol_set& compareTo) const { return !operator<(compareTo); }
    };
}

#endif
