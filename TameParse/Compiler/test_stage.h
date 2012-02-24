//
//  test_stage.h
//  TameParse
//
//  Created by Andrew Hunter on 22/10/2011.
//  Copyright (c) 2011 Andrew Hunter. All rights reserved.
//

#ifndef _COMPILER_TEST_STAGE_H
#define _COMPILER_TEST_STAGE_H

#include <map>

#include "TameParse/Compiler/compilation_stage.h"
#include "TameParse/Compiler/import_stage.h"
#include "TameParse/Compiler/lexer_stage.h"
#include "TameParse/Compiler/lr_parser_stage.h"
#include "TameParse/Language/definition_file.h"

namespace compiler {
    ///
    /// \brief Compilation stage that runs the tests specified in a language definition block
    /// 
    class test_stage : public compilation_stage {
    public:
        /// \brief Maps language names to their grammar/NDFAs
        typedef std::map<std::wstring, language_stage*> language_map;

        /// \brief Maps language names to lexer stages
        typedef std::map<std::wstring, lexer_stage*> lexer_map;

        /// \brief Maps language names and nonterminal names to
        typedef std::map<std::pair<std::wstring, std::wstring>, lr_parser_stage*> parser_map;

    private:
        /// \brief The definition file that contains the tests that this
        language::definition_file_container m_Definition;

        /// \brief The import stage where languages should be loaded from
        const import_stage* m_Import;

        /// \brief Grammars, NDFAs for the languages that have tests
        language_map m_Languages;

        /// \brief Lexers for the languages that have tests
        lexer_map m_Lexers;

        /// \brief Parsers for the nonterminals and languages that have tests
        parser_map m_Parsers;

     private:
        /// \brief Retrieves the language stage for the language with the specified name
        language_stage* get_language(const std::wstring& languageName, const dfa::position& pos);

        /// \brief Retrieves the lexer for the language with the specified name
        lexer_stage* get_lexer(const std::wstring& languageName, const dfa::position& pos);
        
        /// \brief Retrieves the parser for the language with the specified name
        lr_parser_stage* get_parser(const std::wstring& languageName, const std::wstring& nonterminalName, const dfa::position& pos);

    public:
        /// \brief Creates a new test stage that will run the tests in the specified definition file
        test_stage(console_container& console, const std::wstring& filename, const language::definition_file_container& definition, const import_stage* import);

        /// \brief Destructor
        virtual ~test_stage();

        /// \brief Performs the actions associated with this compilation stage
        virtual void compile();
    };
}

#endif
