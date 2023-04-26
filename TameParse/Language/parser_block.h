//
//  parser_block.h
//  Parse
//
//  Created by Andrew Hunter on 25/08/2011.
//  
//  Copyright (c) 2011-2012 Andrew Hunter
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

#ifndef _LANGUAGE_PARSER_BLOCK_H
#define _LANGUAGE_PARSER_BLOCK_H

#include <string>
#include <vector>

#include "TameParse/Language/block.h"

namespace yy_language {
    class parser_block : public block {
    public:
        /// \brief List of start symbols for this language
        typedef std::vector<std::wstring> start_symbol_list;
        
    private:
        /// \brief The name of the parser that should be generated
        std::wstring m_ParserName;
        
        /// \brief The name of the language that the parser should be generated for
        std::wstring m_LanguageName;
        
        /// \brief The names of the start symbols for the language
        start_symbol_list m_StartSymbols;
        
    public:
        /// \brief Creates a new parser block
        parser_block(const std::wstring& name, const std::wstring& languageName, const std::vector<std::wstring>& startSymbols, position startPos, position endPos);
        
    public:
        /// \brief The name of the parser class that should be generated
        inline const std::wstring& parser_name() const { return m_ParserName; }
        
        /// \brief The name of the language that the parser should be generated from
        inline const std::wstring& language_name() const { return m_LanguageName; }
        
        /// \brief The names of the start symbols for this language
        inline const start_symbol_list& start_symbols() const { return m_StartSymbols; }
    };
}

#endif
