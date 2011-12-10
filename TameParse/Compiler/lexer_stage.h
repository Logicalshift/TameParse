//
//  lexer_stage.h
//  Parse
//
//  Created by Andrew Hunter on 21/08/2011.
//  Copyright 2011 Andrew Hunter. All rights reserved.
//

#ifndef _COMPILER_LEXER_STAGE_H
#define _COMPILER_LEXER_STAGE_H

#include <set>

#include "TameParse/Compiler/language_stage.h"
#include "TameParse/Dfa/ndfa_regex.h"
#include "TameParse/Dfa/lexer.h"
#include "TameParse/ContextFree/terminal_dictionary.h"
#include "TameParse/Compiler/compilation_stage.h"
#include "TameParse/Lr/weak_symbols.h"

namespace compiler {
    ///
    /// \brief Class that can compile the DFA associated with a language
    ///
    class lexer_stage : public compilation_stage {
    private:
        /// \brief The language compiler stage that generates the input for this stage
        language_stage* m_Language;
        
        /// \brief The DFA that was generated by this compilation stage
        dfa::ndfa* m_Dfa;
        
        /// \brief The compiled lexer
        dfa::lexer* m_Lexer;
        
        /// \brief The weak symbols object
        lr::weak_symbols m_WeakSymbols;
        
    public:
        /// \brief Creates a new lexer compiler
        ///
        /// The compiler will not 'own' the objects passed in to this constructor; however, they must have a lifespan
        /// that is at least as long as the compiler itself (it's safe to call the destructor but no other call if they
        /// have been destroyed)
        lexer_stage(console_container& console, const std::wstring& filename, language_stage* languageCompiler);
        
        /// \brief Destroys the lexer compiler
        virtual ~lexer_stage();
        
        /// \brief Compiles the lexer (the language compiler must have completed its work by this point)
        void compile();

    private:
        /// \brief Reports any errors that might have occurred in the specified regular expression
        void check_regex(dfa::ndfa_regex* ndfa, const std::wstring& regex, const std::wstring* filename, const dfa::position& pos);

    public:
        /// \brief The DFA generated by this stage
        inline const dfa::ndfa* dfa() const { return m_Dfa; }

        /// \brief The weak symbols action rewriter generated by this stage
        inline const lr::weak_symbols* weak_symbols() const { return &m_WeakSymbols; }
        
        /// \brief The weak symbols action rewriter generated by this stage
        inline lr::weak_symbols* weak_symbols() { return &m_WeakSymbols; }
        
        /// \brief The lexer that was compiled by this stage
        inline dfa::lexer* get_lexer() { return m_Lexer; }
    };
}

#endif
