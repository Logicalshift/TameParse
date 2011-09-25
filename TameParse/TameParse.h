//
//  TameParse.h
//  TameParse
//
//  Created by Andrew Hunter on 24/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
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
#include "TameParse/Lr/lr_action.h"
#include "TameParse/Lr/lr_item.h"
#include "TameParse/Lr/lr_state.h"
#include "TameParse/Lr/parser.h"
#include "TameParse/Lr/parser_stack.h"
#include "TameParse/Lr/parser_state.h"
#include "TameParse/Lr/parser_tables.h"
#include "TameParse/Lr/weak_symbols.h"

#include "TameParse/Language/block.h"
#include "TameParse/Language/bootstrap.h"
#include "TameParse/Language/definition_file.h"
#include "TameParse/Language/ebnf_item.h"
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
#include "TameParse/Language/process.h"
#include "TameParse/Language/production_definition.h"
#include "TameParse/Language/toplevel_block.h"

#include "TameParse/Compiler/compilation_stage.h"
#include "TameParse/Compiler/console.h"
#include "TameParse/Compiler/error.h"
#include "TameParse/Compiler/language_stage.h"
#include "TameParse/Compiler/lexer_stage.h"
#include "TameParse/Compiler/lr_parser_stage.h"
#include "TameParse/Compiler/output_stage.h"
#include "TameParse/Compiler/OutputStages/cplusplus.h"
#include "TameParse/Compiler/std_console.h"
#include "TameParse/Compiler/parser_stage.h"
#include "TameParse/Compiler/import_stage.h"
#include "TameParse/Compiler/language_builder_stage.h"

#endif
