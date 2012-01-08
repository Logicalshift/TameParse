//
//  position.h
//  Parse
//
//  Created by Andrew Hunter on 28/04/2011.
//  Copyright 2011-2012 Andrew Hunter. All rights reserved.
//

#ifndef _DFA_POSITION_H
#define _DFA_POSITION_H

namespace dfa {
    ///
    /// \brief Representation of a position in an input stream
    ///
    class position {
    private:
        /// \brief The offset in symbols from the beginning of the stream of this position
        int m_Offset;
        
        /// \brief The line number of this position (number of newline sequences encountered from offset 0)
        int m_Line;
        
        /// \brief The column number of this position (number of symbols encountered since the last newline sequence)
        int m_Column;
        
    public:
        inline position()
        : m_Offset(0)
        , m_Line(0)
        , m_Column(0) {
        }
        
        inline position(int offset, int line, int column)
        : m_Offset(offset)
        , m_Line(line)
        , m_Column(column) {
        }
        
        inline position(const position& copyFrom)
        : m_Offset(copyFrom.m_Offset)
        , m_Line(copyFrom.m_Line)
        , m_Column(copyFrom.m_Column) {
        }
        
        /// \brief The offset in symbols from the beginning of the stream of this position
        inline int offset() const { return m_Offset; }

        /// \brief The line number of this position (number of newline sequences encountered from offset 0)
        inline int line() const { return m_Line; }

        /// \brief The column number of this position (number of symbols encountered since the last newline sequence)
        inline int column() const { return m_Column; }
        
        /// \brief Moves on by a single symbol
        inline void increment() {
            ++m_Offset;
            ++m_Column;
        }
        
        /// \brief Increases the offset without changing the column
        inline void increment_offset() {
            ++m_Offset;
        }
        
        /// \brief Adds a new line to this position
        inline void newline() {
            m_Column = 0;
            ++m_Line;
        }
        
        /// \brief Compares two positions
        inline bool operator==(const position& compareTo) const {
            if (m_Line != compareTo.m_Line)     return false;
            if (m_Column != compareTo.m_Column) return false;
            
            return m_Offset == compareTo.m_Offset;
        }
        
        /// \brief Compares two positions
        inline bool operator!=(const position& compareTo) const { return !operator==(compareTo); }
        
        /// \brief Compares two positions
        inline bool operator<(const position& compareTo) const {
            if (m_Line < compareTo.m_Line)      return true;
            if (m_Line > compareTo.m_Line)      return false;
            
            if (m_Column < compareTo.m_Column)  return true;
            if (m_Column > compareTo.m_Column)  return false;
            
            return m_Offset < compareTo.m_Offset;
        }
        
        /// \brief Compares two positions
        
        /// \brief Compares two positions
        inline bool operator>(const position& compareTo) const { return compareTo.operator<(*this); }
        
        /// \brief Compares two positions
        inline bool operator>=(const position& compareTo) const { return !operator<(compareTo); }
        
        /// \brief Compares two positions
        inline bool operator<=(const position& compareTo) const { return !operator>(compareTo); }
    };
    
    ///
    /// \brief Class used to track position during lexing
    ///
    class position_tracker {
    private:
        /// \brief The current position
        position m_CurrentPosition;
        
        /// \brief Set to true if the last character is '\r'
        bool m_SeenReturn;
        
    public:
        /// \brief Creates a new position tracker at position 0
        inline position_tracker()
        : m_CurrentPosition()
        , m_SeenReturn(false) {
        }
        
        /// \brief Creates a new position tracker by copying an existing position
        ///
        /// This may record an extra newline in the case where the position represents a location immediately after a carriage return character
        inline position_tracker(const position& copyFrom)
        : m_CurrentPosition(copyFrom)
        , m_SeenReturn(false) {
        }
        
        /// \brief Copies this position tracker
        inline position_tracker(const position_tracker& copyFrom) 
        : m_CurrentPosition(copyFrom.m_CurrentPosition)
        , m_SeenReturn(copyFrom.m_SeenReturn) {
        }
        
        /// \brief Returns a copy of the current position
        inline position current_position() const { return m_CurrentPosition; }
        
        /// \brief Moves the position on by a single symbol
        inline void update_position(int symbol) {
            switch (symbol) {
                case 0x0a:              // LF: Line Feed
                    if (!m_SeenReturn) {
                        // Previous symbol was not 0x0d
                        m_CurrentPosition.increment();
                        m_CurrentPosition.newline();
                    } else {
                        // Sequence is 0x0d 0x0a (ie, a CR+LF sequence)
                        m_CurrentPosition.increment_offset();
                    }
                    
                    m_SeenReturn = false;
                    break;
                    
                case 0x0d:              // CR: Carriage Return
                    m_CurrentPosition.newline();
                    m_SeenReturn = true;
                    break;
                    
                case 0x0b:              // VT: Vertical Tab
                case 0x0c:              // FF: Form Feed
                case 0x85:              // NEL: NExt Line
                case 0x2028:            // LS: Line Separator
                case 0x2029:            // PS: Paragraph Separator
                    m_CurrentPosition.newline();
                    m_SeenReturn = false;
                    break;
                    
                default:
                    // Not a newline: just increment the position
                    m_SeenReturn = false;
                    m_CurrentPosition.increment();
                    break;
            }
        }
        
        /// \brief Processes a set of symbols (a class with an iterator) and updates the position
        template<typename iterator> inline void update_position(iterator begin, iterator end) {
            for (iterator symbol=begin; symbol != end; ++symbol) {
                update_position((int)(unsigned)*symbol);
            }
        }
    };
}

#endif
