//
//  utf8reader.h
//  TameParse
//
//  Created by Andrew Hunter on 24/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _UTIL_UTF8READER_H
#define _UTIL_UTF8READER_H

#include <iostream>

namespace util {
    ///
    /// \brief Reader class that can convert an istream encoded using UTF-8 to a unicode 
    /// character stream.
    ///
    /// This provides the get() and good() methods required to use the stream as input 
    /// to a lexer. We don't provide an implementation of istream as C++'s architecture
    /// makes creating stream implementations with characters of variable size very
    /// difficult, and this is sufficient to provide input to the parser.
    ///
    /// This will generate surrogate characters; the default implementation of the
    /// parser will expect a language as a UTF-16 sequence, so unicode characters
    /// greater than 0xffff should be supported. This will make this unsuitable for
    /// systems where wchar_t is expected to be in UCS-4.
    ///
    class utf8reader {
    private:
    	/// \brief The input stream
        std::istream* m_InputStream;

        /// \brief True if this object owns the input stream and should free it when done
        bool m_OwnsStream;

        /// \brief True if a bad UTF-8 character has been encountered
        bool m_BadUTF8;

        /// \brief 0, or the next pair character in a surrogate pair
        wchar_t m_PairChar;

    public:
    	/// \brief Creates a new UTF-8 reader
    	///
    	/// Set ownsReader to true to specify that this object owns its stream and should
    	/// dispose of it when it is freed.
    	explicit utf8reader(std::istream* inputStream, bool ownsReader = false);

    	/// \brief Destructor for this object
    	~utf8reader();

    	/// \brief Places the next unicode character in the target
    	///
    	/// This will read multiple characters from the source stream until an entire
    	/// unicode character has been constructed. In the case where there is a problem,
    	/// the target will be set to 0 and good() will return false;
    	utf8reader& get(wchar_t& target);

    	/// \brief True if the stream is good
   		bool good() const;
    };
}

#endif
