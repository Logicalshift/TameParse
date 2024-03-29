//
//  language_stage.h
//  Parse
//
//  Created by Andrew Hunter on 30/07/2011.
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

#ifndef _COMPILER_LANGUAGE_STAGE_H
#define _COMPILER_LANGUAGE_STAGE_H

#include <set>

#include "TameParse/Dfa/ndfa_regex.h"
#include "TameParse/ContextFree/grammar.h"
#include "TameParse/ContextFree/terminal_dictionary.h"
#include "TameParse/Language/language_block.h"
#include "TameParse/Language/language_unit.h"
#include "TameParse/Compiler/compilation_stage.h"
#include "TameParse/Compiler/Data/lexer_data.h"
#include "TameParse/Compiler/Data/rule_item_data.h"
#include "TameParse/Lr/action_rewriter.h"

#ifndef TAMEPARSE_BOOTSTRAP
#include "TameParse/Compiler/import_stage.h"
#endif

namespace compiler {
#ifdef TAMEPARSE_BOOTSTRAP
    class import_stage {
        // Import stage is not available in the bootstrap parser
    };
#endif

    ///
    /// \brief Class that handles compiling a language block into a lexer and a grammar
    ///
    /// This class handles the steps necessary to compile a language block with no inheritance into a grammar and a lexer.
    /// These can in turn be used to build a finished parser.
    ///
    class language_stage : public compilation_stage {
    public:
        /// \brief Maps a language block to the file it is defined it
        typedef std::pair<yy_language::block*, std::wstring*> block_file;

        /// \brief A position and the file it is in
        typedef std::pair<dfa::position, std::wstring*> position_file;
        
        /// \brief Maps a symbol ID to the language block and file where it is defined
        typedef std::map<int, block_file> symbol_map;

        /// \brief A list of LR action rewriters
        typedef std::vector<lr::action_rewriter_container> rewriter_list;
        
    private:
        /// \brief The language block that this will compile
        const yy_language::language_block* m_Language;

        /// \brief The import stage used to find and compile any inherited languages
        const import_stage* m_Import;
        
        /// \brief Null, or the language stage that this inherits from
        language_stage* m_InheritsFrom;
        
        /// \brief The dictionary of terminals defined by the language
        contextfree::terminal_dictionary m_Terminals;
        
        /// \brief The lexer defined by the language
        lexer_data m_Lexer;
        
        /// \brief The grammar defined by the language
        contextfree::grammar m_Grammar;

        /// \brief The guards defined by the language (and where they're defined)
        std::map<contextfree::item_container, position_file> m_Guards;
        
        /// \brief The IDs of symbols defined as being 'weak'
        std::set<int> m_WeakSymbols;
        
        /// \brief The IDs of symbols defined as being 'ignored'
        std::set<int> m_IgnoredSymbols;

        /// \brief The IDs of (usually) ignored symbols that are used as part of the grammar
        std::set<int> m_UsedIgnoredSymbols;
        
        /// \brief Symbols defined in the lexer that are marked as 'unused'
        std::set<int> m_UnusedSymbols;
        
        /// \brief The type of the definition for each terminal symbol
        std::map<int, yy_language::language_unit::unit_type> m_TypeForTerminal;
        
        /// \brief Maps terminal IDs to the block where they are declared (used for reporting errors later on)
        symbol_map m_TerminalDefinition;
        
        /// \brief Maps nonterminal IDs to the point where they were first used
        symbol_map m_FirstNonterminalUsage;

        /// \brief The data associated with each rule
        rule_item_data m_RuleItemData;

        /// \brief Any LR action rewriters that are defined by the language
        ///
        /// These are used to implement things like operator precedence. This design
        /// is slightly suboptimal as it assumes that the output of this stage will
        /// be used to build a LALR parser.
        rewriter_list m_ActionRewriters;

        /// \brief Maps strings to string pointers (stores the filenames we know about)
        ///
        /// This is used to avoid relentlessly copying the filename alongside the blocks.
        std::map<std::wstring, std::wstring*> m_Filenames;
        
        /// \brief Maps rule IDs to the block where they are declared (used for reporting errors later on)
        symbol_map m_RuleDefinition;
        
    public:
        /// \brief Creates a compiler that will compile the specified language block
        language_stage(console_container& console, const std::wstring& filename, const yy_language::language_block* block, const import_stage* importStage);
        
        /// \brief Destructor
        virtual ~language_stage();
        
        /// \brief Compiles the language, creating the dictionary of terminals, the lexer and the grammar
        void compile();

        /// \brief Reports which terminal symbols are unused in this language (and any languages that it inherits from)
        void report_unused_symbols();
        
    private:
        /// \brief Adds any lexer items that are defined by a specific EBNF item to this object
        ///
        /// Returns the number of new items that were defined
        int add_ebnf_lexer_items(yy_language::ebnf_item* item);

        /// \brief Compiles an EBNF item from the language into a context-free grammar item onto the end of the specified rule
        ///
        /// The lexer items should already be compiled before this call is made; it's a bug if any terminal items are found
        /// to be missing from the terminal dictionary.
        void compile_item(contextfree::rule& target, yy_language::ebnf_item* item, std::wstring* ourFilename);

        /// \brief Attaches attributes to the last item in the specified rule
        void append_attribute(contextfree::rule& target, const rule_item_data::rule_attributes& attributes);

        /// \brief In a final pass, process the symbols in a particular rule
        ///
        /// This will ensure that the m_UnusedSymbols set is up to date, as well as
        /// the m_UsedIgnoredSymbols set.
        void process_rule_symbols(const contextfree::rule& rule);
        
    public:
        /// \brief The grammar generated by this stage
        inline const contextfree::grammar* grammar() const                  { return &m_Grammar; }

        /// \brief The grammar generated by this stage
        inline contextfree::grammar* grammar()                              { return &m_Grammar; }

        /// \brief The lexer data for this stage
        inline const lexer_data* lexer() const                              { return &m_Lexer; }
        
        /// \brief The terminal dictionary generated by this stage
        inline const contextfree::terminal_dictionary* terminals() const    { return &m_Terminals; }
        
        /// \brief The terminal dictionary generated by this stage
        inline contextfree::terminal_dictionary* terminals()                { return &m_Terminals; }
        
        /// \brief The set of symbols identifiers that should be treated as being 'weak'
        inline const std::set<int>* weak_symbols() const                    { return &m_WeakSymbols; }

        /// \brief The set of symbol identifiers that should be ignored by the parser
        inline const std::set<int>* ignored_symbols() const                 { return &m_IgnoredSymbols; }

        /// \brief The symbols that are usually ignored but occasionally have syntactic meaning
        inline const std::set<int>* used_ignored_symbols() const            { return &m_UsedIgnoredSymbols; }

        /// \brief A list of the action rewriters defined by this language
        inline const rewriter_list* action_rewriters() const                { return &m_ActionRewriters; }
        
        /// \brief The position in the file where the terminal symbol with the given ID was defined
        inline dfa::position terminal_definition_pos(int id) const {
            symbol_map::const_iterator found = m_TerminalDefinition.find(id);
            if (found == m_TerminalDefinition.end() || !found->second.first) return dfa::position(-1, -1, -1);
            return found->second.first->start_pos();
        }

        /// \brief Returns the object that is used to store attribute data about each rule
        inline const rule_item_data& get_rule_item_data() const { return m_RuleItemData; }

        /// \brief The position in the file where the rule with the given ID was defined
        inline dfa::position rule_definition_pos(int id) const {
            // Try finding the rule in the list
            symbol_map::const_iterator found = m_RuleDefinition.find(id);

            if (found == m_RuleDefinition.end() || !found->second.first) {
                // Rule not found. It's possible we know the position of the nonterminal, though
                // This will apply to EBNF items when they are expanded, but it's not ideal if
                // a particular item appears in multiple places in the grammar, as we might
                // indiate an incorrect position.
                int ntId = grammar()->identifier_for_item(grammar()->rule_with_identifier(id)->nonterminal());

                found = m_FirstNonterminalUsage.find(ntId);

                if (found == m_FirstNonterminalUsage.end() || !found->second.first) {
                    // Rule not found
                    return dfa::position(-1, -1, -1);
                }
            }

            // Rule was found
            return found->second.first->start_pos();            
        }
        
        /// \brief The name of the file where a particular terminal symbol is defined
        inline const std::wstring& terminal_definition_file(int id) const {
            static const std::wstring empty_string;
            
            symbol_map::const_iterator found = m_TerminalDefinition.find(id);
            if (found == m_TerminalDefinition.end() || !found->second.first) return empty_string;
            return *found->second.second;
        }
        
        /// \brief The name of the file where a particular rule is defined
        inline const std::wstring& rule_definition_file(int id) const {
            static const std::wstring empty_string;
            
            // Try finding the rule directly
            symbol_map::const_iterator found = m_RuleDefinition.find(id);

            if (found == m_RuleDefinition.end() || !found->second.first) {
                // Rule not found. It's possible we know the position of the nonterminal, though
                // This will apply to EBNF items when they are expanded, but it's not ideal if
                // a particular item appears in multiple places in the grammar, as we might
                // indiate an incorrect position.
                int ntId = grammar()->identifier_for_item(grammar()->rule_with_identifier(id)->nonterminal());

                found = m_FirstNonterminalUsage.find(ntId);

                if (found == m_FirstNonterminalUsage.end() || !found->second.first) {
                    // Rule not found
                    return empty_string;
                }
            }

            return *found->second.second;
        }

    private:
        /// \brief Exports the results of this language stage into another
        ///
        /// This is used to deal with inherited languages: it will copy the contents
        /// of this object into the target object (making them identical). The 
        /// target object can then compile the differences to create the final 
        /// language.
        void export_to(language_stage* target);
    };
}

#endif
