//
//  parser_block.h
//  Parse
//
//  Created by Andrew Hunter on 25/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _LANGUAGE_PARSER_BLOCK_H
#define _LANGUAGE_PARSER_BLOCK_H

#include <string>
#include <vector>

#include "TameParse/Language/block.h"

namespace language {
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
