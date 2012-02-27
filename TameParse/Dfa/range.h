//
//  range.h
//  Parse
//
//  Created by Andrew Hunter on 13/03/2011.
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

#ifndef _DFA_RANGE_H
#define _DFA_RANGE_H

namespace dfa {
    ///
    /// \brief Class representing a range of values
    ///
    template<class T> class range {
    private:
        /// \brief The inclusive lower bound of this range
        T m_Lower;
        
        /// \brief The exclusive upper bound of this range
        T m_Upper;
        
    public:
        /// \brief Comparator functor for the symbol store set. 
        /// 
        /// We compare only by the lower bounds (so any range with the same lower bounds is considered equal; this works as we store a set of 
        /// non-overlapping ranges)
        class sort_by_lower {
        public:
            inline bool operator()(const range<T>& a, const range<T>& b) const {
                return a.lower() < b.lower();
            }
        };
        
    public:
        /// \brief Constructs a range containing a single value
        inline range(T value) {
            m_Lower = value;
            m_Upper = value + 1;
        }
        
        /// \brief Constructs a range from one value to another (the upper value is excluded)
        inline range(T from, T to) {
            m_Lower = from;
            m_Upper = to;
        }
        
        inline range(const range<T>& copyFrom) {
            m_Lower = copyFrom.m_Lower;
            m_Upper = copyFrom.m_Upper;
        }
        
        inline range<T>& operator=(const range<T>& copyFrom) {
            m_Lower = copyFrom.m_Lower;
            m_Upper = copyFrom.m_Upper;
            
            return *this;
        }
        
    public:
        /// \brief The first item included in this range
        inline T lower() const { return m_Lower; }
        
        /// \brief The first item not included in this range
        inline T upper() const { return m_Upper; }
        
    public:
        /// \brief Determines if this range contains a particular value
        inline bool operator[](T value) const {
            if (value < m_Lower) return false;
            if (value >= m_Upper) return false;
            return true;
        }
        
        /// \brief Determines if two ranges refer to the same region
        inline bool operator==(const range<T>& compareTo) const {
            return compareTo.m_Lower == m_Lower && compareTo.m_Upper == m_Upper; 
        }
        
        /// \brief Inverse of operator==
        inline bool operator!=(const range<T>& compareTo) const { return !operator==(compareTo); }
        
        /// \brief Orders this range relative to another
        inline bool operator<(const range<T>& compareTo) const {
            // Order by the lower bound first
            if (m_Lower < compareTo.m_Lower)    return true;
            if (m_Lower > compareTo.m_Lower)    return false;
            
            return m_Upper < compareTo.m_Upper;
        }
        
        /// \brief Orders this range relative to another
        inline bool operator<=(const range<T>& compareTo) const {
            return m_Lower < compareTo.m_Lower || (m_Lower == compareTo.m_Lower && m_Upper <= compareTo.m_Upper);
        }
        
        /// \brief Orders this range relative to another
        inline bool operator>(const range<T>& compareTo) const { return !operator<=(compareTo); }
        
        /// \brief Orders this range relative to another
        inline bool operator>=(const range<T>& compareTo) const { return !operator<(compareTo); }
        
        /// \brief Determines if this range overlaps another
        inline bool overlaps(const range<T>& compareTo) const {
            if (compareTo.m_Upper <= m_Lower) return false;
            if (compareTo.m_Lower >= m_Upper) return false;
            return true;
        }
        
        /// \brief Determines if the specified range is a neighbour of this one
        inline bool neighbours(const range<T>& compareTo) const {
            if (compareTo.m_Upper == m_Lower) return true;
            if (compareTo.m_Lower == m_Upper) return true;
            return false;
        }
        
        /// \brief Determines if this range can be merged with another to make a larger range
        inline bool can_merge(const range<T>& compareTo) const {
            if (compareTo.m_Upper < m_Lower) return false;
            if (compareTo.m_Lower > m_Upper) return false;
            return true;            
        }
        
        /// \brief Merges this range with another
        inline range<T> merge(const range<T>& mergeWith) const {
            T lower = m_Lower;
            T upper = m_Upper;
            
            if (mergeWith.m_Lower < m_Lower) lower = mergeWith.m_Lower;
            if (mergeWith.m_Upper > m_Upper) upper = mergeWith.m_Upper;
            
            return range<T>(lower, upper);
        }
    };
}

#endif
