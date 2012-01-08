noinst_PROGRAMS		= bootstrap
EXTRA_DIST 			= boostrap.1

definition_tp.h: $(top_srcdir)/TameParse/Language/definition.tp
	ln -sf $(top_srcdir)/TameParse/Language/definition.tp ./definition.tp
	xxd -i "definition.tp" >definition_tp.h

../TameParse/Language/bootstrap.cpp: definition_tp.h

bootstrap_CFLAGS	= -I$(top_srcdir) -DTAMEPARSE_BOOTSTRAP=1
bootstrap_CXXFLAGS	= -I$(top_srcdir) -DTAMEPARSE_BOOTSTRAP=1
bootstrap_SOURCES	= main.cpp \
					  definition_tp.h \
					  ../TameParse/Compiler/compilation_stage.cpp \
					  ../TameParse/Compiler/conflict_attribute_rewriter.cpp \
					  ../TameParse/Compiler/console.cpp \
					  ../TameParse/Compiler/error.cpp \
					  ../TameParse/Compiler/language_stage.cpp \
					  ../TameParse/Compiler/lexer_stage.cpp \
					  ../TameParse/Compiler/lr_parser_stage.cpp \
					  ../TameParse/Compiler/output_stage.cpp \
					  ../TameParse/Compiler/std_console.cpp \
					  ../TameParse/Compiler/OutputStages/cplusplus.cpp \
					  ../TameParse/Compiler/Data/lexer_data.cpp \
					  ../TameParse/Compiler/Data/lexer_item.cpp \
					  ../TameParse/Compiler/Data/rule_item_data.cpp \
					  ../TameParse/ContextFree/ebnf_items.cpp \
					  ../TameParse/ContextFree/grammar.cpp \
					  ../TameParse/ContextFree/guard.cpp \
					  ../TameParse/ContextFree/item.cpp \
					  ../TameParse/ContextFree/item_set.cpp \
					  ../TameParse/ContextFree/rule.cpp \
					  ../TameParse/ContextFree/standard_items.cpp \
					  ../TameParse/ContextFree/terminal_dictionary.cpp \
					  ../TameParse/Dfa/accept_action.cpp \
					  ../TameParse/Dfa/basic_lexer.cpp \
					  ../TameParse/Dfa/character_lexer.cpp \
					  ../TameParse/Dfa/epsilon.cpp \
					  ../TameParse/Dfa/hard_coded_symbol_table.cpp \
					  ../TameParse/Dfa/lexeme.cpp \
					  ../TameParse/Dfa/lexer.cpp \
					  ../TameParse/Dfa/ndfa.cpp \
					  ../TameParse/Dfa/ndfa_regex.cpp \
					  ../TameParse/Dfa/ndfa_transformations.cpp \
					  ../TameParse/Dfa/position.cpp \
					  ../TameParse/Dfa/range.cpp \
					  ../TameParse/Dfa/remapped_symbol_map.cpp \
					  ../TameParse/Dfa/regex_error.cpp \
					  ../TameParse/Dfa/state.cpp \
					  ../TameParse/Dfa/state_machine.cpp \
					  ../TameParse/Dfa/symbol_map.cpp \
					  ../TameParse/Dfa/symbol_set.cpp \
					  ../TameParse/Dfa/symbol_table.cpp \
					  ../TameParse/Dfa/symbol_translator.cpp \
					  ../TameParse/Dfa/transition.cpp \
					  ../TameParse/Language/block.cpp \
					  ../TameParse/Language/bootstrap.cpp \
					  ../TameParse/Language/definition_file.cpp \
					  ../TameParse/Language/ebnf_item.cpp \
					  ../TameParse/Language/ebnf_item_attributes.cpp \
					  ../TameParse/Language/formatter.cpp \
					  ../TameParse/Language/grammar_block.cpp \
					  ../TameParse/Language/import_block.cpp \
					  ../TameParse/Language/language_block.cpp \
					  ../TameParse/Language/language_unit.cpp \
					  ../TameParse/Language/lexeme_definition.cpp \
					  ../TameParse/Language/lexer_block.cpp \
					  ../TameParse/Language/nonterminal_definition.cpp \
					  ../TameParse/Language/parser_block.cpp \
					  ../TameParse/Language/process.cpp \
					  ../TameParse/Language/production_definition.cpp \
					  ../TameParse/Language/test_block.cpp \
					  ../TameParse/Language/test_definition.cpp \
					  ../TameParse/Language/toplevel_block.cpp \
					  ../TameParse/Lr/action_rewriter.cpp \
					  ../TameParse/Lr/ast_parser.cpp \
					  ../TameParse/Lr/conflict.cpp \
					  ../TameParse/Lr/ignored_symbols.cpp \
					  ../TameParse/Lr/lalr_builder.cpp \
					  ../TameParse/Lr/lalr_machine.cpp \
					  ../TameParse/Lr/lalr_state.cpp \
					  ../TameParse/Lr/lr1_item_set.cpp \
					  ../TameParse/Lr/lr_action.cpp \
					  ../TameParse/Lr/lr_item.cpp \
					  ../TameParse/Lr/lr_state.cpp \
					  ../TameParse/Lr/parser.cpp \
					  ../TameParse/Lr/parser_stack.cpp \
					  ../TameParse/Lr/parser_tables.cpp \
					  ../TameParse/Lr/weak_symbols.cpp \
					  ../TameParse/Util/astnode.cpp \
					  ../TameParse/Util/container.cpp \
					  ../TameParse/Util/stringreader.cpp \
					  ../TameParse/Util/syntax_ptr.cpp \
					  ../TameParse/Util/unicode.cpp \
					  ../TameParse/Util/utf8reader.cpp \
					  ../TameParse/version.cpp
