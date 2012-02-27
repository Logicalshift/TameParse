//
//  TameParse.h
//  TameParse
//
//  Created by Andrew Hunter on 24/09/2011.
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

#ifndef _TAMEPARSE_H
#define _TAMEPARSE_H

#include "TameParse/version.h"

#include "TameParse/Util/astnode.h"
#include "TameParse/Util/container.h"
#include "TameParse/Util/stringreader.h"
#include "TameParse/Util/syntax_ptr.h"
#include "TameParse/Util/unicode.h"
#include "TameParse/Util/utf8reader.h"

#include "TameParse/Dfa/accept_action.h"
#include "TameParse/Dfa/basic_lexer.h"
#include "TameParse/Dfa/character_lexer.h"
#include "TameParse/Dfa/epsilon.h"
#include "TameParse/Dfa/hard_coded_symbol_table.h"
#include "TameParse/Dfa/lexeme.h"
#include "TameParse/Dfa/lexer.h"
#include "TameParse/Dfa/ndfa.h"
#include "TameParse/Dfa/ndfa_regex.h"
#include "TameParse/Dfa/position.h"
#include "TameParse/Dfa/range.h"
#include "TameParse/Dfa/regex_error.h"
#include "TameParse/Dfa/remapped_symbol_map.h"
#include "TameParse/Dfa/state.h"
#include "TameParse/Dfa/state_machine.h"
#include "TameParse/Dfa/symbol_map.h"
#include "TameParse/Dfa/symbol_set.h"
#include "TameParse/Dfa/symbol_table.h"
#include "TameParse/Dfa/symbol_translator.h"
#include "TameParse/Dfa/transition.h"

#include "TameParse/ContextFree/ebnf_items.h"
#include "TameParse/ContextFree/grammar.h"
#include "TameParse/ContextFree/guard.h"
#include "TameParse/ContextFree/item.h"
#include "TameParse/ContextFree/item_set.h"
#include "TameParse/ContextFree/rule.h"
#include "TameParse/ContextFree/standard_items.h"
#include "TameParse/ContextFree/terminal_dictionary.h"

#include "TameParse/Lr/action_rewriter.h"
#include "TameParse/Lr/ast_parser.h"
#include "TameParse/Lr/conflict.h"
#include "TameParse/Lr/ignored_symbols.h"
#include "TameParse/Lr/lalr_builder.h"
#include "TameParse/Lr/lalr_machine.h"
#include "TameParse/Lr/lalr_state.h"
#include "TameParse/Lr/lr1_item_set.h"
#include "TameParse/Lr/lr1_rewriter.h"
#include "TameParse/Lr/lr_action.h"
#include "TameParse/Lr/lr_item.h"
#include "TameParse/Lr/lr_state.h"
#include "TameParse/Lr/parser.h"
#include "TameParse/Lr/parser_stack.h"
#include "TameParse/Lr/parser_state.h"
#include "TameParse/Lr/parser_tables.h"
#include "TameParse/Lr/precedence_rewriter.h"
#include "TameParse/Lr/weak_symbols.h"

#include "TameParse/Language/block.h"
#include "TameParse/Language/bootstrap.h"
#include "TameParse/Language/definition_file.h"
#include "TameParse/Language/ebnf_item.h"
#include "TameParse/Language/ebnf_item_attributes.h"
#include "TameParse/Language/formatter.h"
#include "TameParse/Language/grammar_block.h"
#include "TameParse/Language/import_block.h"
#include "TameParse/Language/language_block.h"
#include "TameParse/Language/language_parser.h"
#include "TameParse/Language/language_unit.h"
#include "TameParse/Language/lexeme_definition.h"
#include "TameParse/Language/lexer_block.h"
#include "TameParse/Language/nonterminal_definition.h"
#include "TameParse/Language/parser_block.h"
#include "TameParse/Language/precedence_block.h"
#include "TameParse/Language/process.h"
#include "TameParse/Language/production_definition.h"
#include "TameParse/Language/test_block.h"
#include "TameParse/Language/test_definition.h"
#include "TameParse/Language/toplevel_block.h"

#include "TameParse/Compiler/compilation_stage.h"
#include "TameParse/Compiler/conflict_attribute_rewriter.h"
#include "TameParse/Compiler/console.h"
#include "TameParse/Compiler/Data/lexer_data.h"
#include "TameParse/Compiler/Data/lexer_item.h"
#include "TameParse/Compiler/Data/rule_item_data.h"
#include "TameParse/Compiler/error.h"
#include "TameParse/Compiler/import_stage.h"
#include "TameParse/Compiler/language_builder_stage.h"
#include "TameParse/Compiler/language_stage.h"
#include "TameParse/Compiler/lexer_stage.h"
#include "TameParse/Compiler/lr_parser_stage.h"
#include "TameParse/Compiler/output_stage.h"
#include "TameParse/Compiler/output_stage_data.h"
#include "TameParse/Compiler/OutputStages/binary.h"
#include "TameParse/Compiler/OutputStages/cplusplus.h"
#include "TameParse/Compiler/parser_stage.h"
#include "TameParse/Compiler/precedence_block_rewriter.h"
#include "TameParse/Compiler/std_console.h"
#include "TameParse/Compiler/test_stage.h"

#endif
