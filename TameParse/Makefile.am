EXTRA_DIST					= Unicode/unicode.pl \
					 	  	  Language/definition.tp

definition_tp.h: $(top_srcdir)/TameParse/Language/definition.tp
	$(LN_S) -f $(top_srcdir)/TameParse/Language/definition.tp ./definition.tp
	xxd -i "definition.tp" >definition_tp.h

tameparse_language.h tameparse_language.cpp: ../bootstrap/bootstrap
	../bootstrap/bootstrap

Language/bootstrap.cpp: definition_tp.h

Language/language_parser.cpp: tameparse_language.h

libTameParse_VERSION		= 1:0:0
lib_LTLIBRARIES				= libTameParse.la
libTameParse_la_CFLAGS		= -I$(top_srcdir)
libTameParse_la_CXXFLAGS	= -I$(top_srcdir)
libTameParse_la_LDFLAGS		= -version-info $(libTameParse_VERSION)

libTameParse_la_SOURCES		= tameparse_language.h \
							  tameparse_language.cpp \
							  \
							  Compiler/compilation_stage.h \
							  Compiler/console.h \
							  Compiler/error.h \
							  Compiler/import_stage.h \
							  Compiler/language_builder_stage.h \
							  Compiler/language_stage.h \
							  Compiler/lexer_stage.h \
							  Compiler/lr_parser_stage.h \
							  Compiler/output_stage.h \
							  Compiler/parser_stage.h \
							  Compiler/std_console.h \
							  Compiler/test_stage.h \
							  Compiler/OutputStages/cplusplus.h \
							  Compiler/Data/lexer_data.h \
							  ContextFree/ebnf_items.h \
							  ContextFree/grammar.h \
							  ContextFree/guard.h \
							  ContextFree/item.h \
							  ContextFree/item_set.h \
							  ContextFree/rule.h \
							  ContextFree/standard_items.h \
							  ContextFree/terminal_dictionary.h \
							  Dfa/accept_action.h \
							  Dfa/basic_lexer.h \
							  Dfa/character_lexer.h \
							  Dfa/epsilon.h \
							  Dfa/hard_coded_symbol_table.h \
							  Dfa/lexeme.h \
							  Dfa/lexer.h \
							  Dfa/ndfa.h \
							  Dfa/ndfa_regex.h \
							  Dfa/position.h \
							  Dfa/range.h \
							  Dfa/remapped_symbol_map.h \
							  Dfa/state.h \
							  Dfa/state_machine.h \
							  Dfa/symbol_map.h \
							  Dfa/symbol_set.h \
							  Dfa/symbol_table.h \
							  Dfa/symbol_translator.h \
							  Dfa/transition.h \
							  Language/block.h \
							  Language/bootstrap.h \
							  Language/definition_file.h \
							  Language/ebnf_item.h \
							  Language/formatter.h \
							  Language/grammar_block.h \
							  Language/import_block.h \
							  Language/language_block.h \
							  Language/language_parser.h \
							  Language/language_unit.h \
							  Language/lexeme_definition.h \
							  Language/lexer_block.h \
							  Language/nonterminal_definition.h \
							  Language/parser_block.h \
							  Language/process.h \
							  Language/production_definition.h \
					  		  Language/test_block.h \
					  		  Language/test_definition.h \
							  Language/toplevel_block.h \
							  Lr/action_rewriter.h \
							  Lr/ast_parser.h \
							  Lr/conflict.h \
							  Lr/ignored_symbols.h \
							  Lr/lalr_builder.h \
							  Lr/lalr_machine.h \
							  Lr/lalr_state.h \
							  Lr/lr1_item_set.h \
							  Lr/lr_action.h \
							  Lr/lr_item.h \
							  Lr/lr_state.h \
							  Lr/parser.h \
							  Lr/parser_stack.h \
							  Lr/parser_state.h \
							  Lr/parser_tables.h \
							  Lr/weak_symbols.h \
							  TameParse.h \
							  Unicode/unicode_data.h \
							  Util/astnode.h \
							  Util/container.h \
							  Util/stringreader.h \
							  Util/syntax_ptr.h \
							  Util/unicode.h \
							  Util/utf8reader.h \
							  version.h \
							  \
							  Compiler/compilation_stage.cpp \
							  Compiler/console.cpp \
							  Compiler/error.cpp \
							  Compiler/import_stage.cpp \
							  Compiler/language_builder_stage.cpp \
							  Compiler/language_stage.cpp \
							  Compiler/lexer_stage.cpp \
							  Compiler/lr_parser_stage.cpp \
							  Compiler/output_stage.cpp \
							  Compiler/parser_stage.cpp \
							  Compiler/std_console.cpp \
							  Compiler/test_stage.cpp \
							  Compiler/OutputStages/cplusplus.cpp \
							  Compiler/Data/lexer_data.cpp \
							  ContextFree/ebnf_items.cpp \
							  ContextFree/grammar.cpp \
							  ContextFree/guard.cpp \
							  ContextFree/item.cpp \
							  ContextFree/item_set.cpp \
							  ContextFree/rule.cpp \
							  ContextFree/standard_items.cpp \
							  ContextFree/terminal_dictionary.cpp \
							  Dfa/accept_action.cpp \
							  Dfa/basic_lexer.cpp \
							  Dfa/character_lexer.cpp \
							  Dfa/epsilon.cpp \
							  Dfa/hard_coded_symbol_table.cpp \
							  Dfa/lexeme.cpp \
							  Dfa/lexer.cpp \
							  Dfa/ndfa.cpp \
							  Dfa/ndfa_regex.cpp \
							  Dfa/ndfa_transformations.cpp \
							  Dfa/position.cpp \
							  Dfa/range.cpp \
							  Dfa/remapped_symbol_map.cpp \
							  Dfa/state.cpp \
							  Dfa/state_machine.cpp \
							  Dfa/symbol_map.cpp \
							  Dfa/symbol_set.cpp \
							  Dfa/symbol_table.cpp \
							  Dfa/symbol_translator.cpp \
							  Dfa/transition.cpp \
							  Language/block.cpp \
							  Language/bootstrap.cpp \
							  Language/definition_file.cpp \
							  Language/ebnf_item.cpp \
							  Language/formatter.cpp \
							  Language/grammar_block.cpp \
							  Language/import_block.cpp \
							  Language/language_block.cpp \
							  Language/language_parser.cpp \
							  Language/language_unit.cpp \
							  Language/lexeme_definition.cpp \
							  Language/lexer_block.cpp \
							  Language/nonterminal_definition.cpp \
							  Language/parser_block.cpp \
							  Language/process.cpp \
							  Language/production_definition.cpp \
							  Language/test_block.cpp \
							  Language/test_definition.cpp \
							  Language/toplevel_block.cpp \
							  Lr/action_rewriter.cpp \
							  Lr/ast_parser.cpp \
							  Lr/conflict.cpp \
							  Lr/ignored_symbols.cpp \
							  Lr/lalr_builder.cpp \
							  Lr/lalr_machine.cpp \
							  Lr/lalr_state.cpp \
							  Lr/lr1_item_set.cpp \
							  Lr/lr_action.cpp \
							  Lr/lr_item.cpp \
							  Lr/lr_state.cpp \
							  Lr/parser.cpp \
							  Lr/parser_stack.cpp \
							  Lr/parser_tables.cpp \
							  Lr/weak_symbols.cpp \
							  Util/astnode.cpp \
							  Util/container.cpp \
							  Util/stringreader.cpp \
							  Util/syntax_ptr.cpp \
							  Util/unicode.cpp \
							  Util/utf8reader.cpp \
							  version.cpp

library_includedir 			= $(includedir)/TameParse-$(PACKAGE_VERSION)/TameParse
nobase_library_include_HEADERS = Compiler/compilation_stage.h \
							  Compiler/console.h \
							  Compiler/error.h \
							  Compiler/import_stage.h \
							  Compiler/language_builder_stage.h \
							  Compiler/language_stage.h \
							  Compiler/lexer_stage.h \
							  Compiler/lr_parser_stage.h \
							  Compiler/output_stage.h \
							  Compiler/parser_stage.h \
							  Compiler/std_console.h \
							  Compiler/test_stage.h \
							  Compiler/OutputStages/cplusplus.h \
							  Compiler/Data/lexer_data.h \
							  ContextFree/ebnf_items.h \
							  ContextFree/grammar.h \
							  ContextFree/guard.h \
							  ContextFree/item.h \
							  ContextFree/item_set.h \
							  ContextFree/rule.h \
							  ContextFree/standard_items.h \
							  ContextFree/terminal_dictionary.h \
							  Dfa/accept_action.h \
							  Dfa/basic_lexer.h \
							  Dfa/character_lexer.h \
							  Dfa/epsilon.h \
							  Dfa/hard_coded_symbol_table.h \
							  Dfa/lexeme.h \
							  Dfa/lexer.h \
							  Dfa/ndfa.h \
							  Dfa/ndfa_regex.h \
							  Dfa/position.h \
							  Dfa/range.h \
							  Dfa/remapped_symbol_map.h \
							  Dfa/state.h \
							  Dfa/state_machine.h \
							  Dfa/symbol_map.h \
							  Dfa/symbol_set.h \
							  Dfa/symbol_table.h \
							  Dfa/symbol_translator.h \
							  Dfa/transition.h \
							  Language/block.h \
							  Language/bootstrap.h \
							  Language/definition_file.h \
							  Language/ebnf_item.h \
							  Language/formatter.h \
							  Language/grammar_block.h \
							  Language/import_block.h \
							  Language/language_block.h \
							  Language/language_parser.h \
							  Language/language_unit.h \
							  Language/lexeme_definition.h \
							  Language/lexer_block.h \
							  Language/nonterminal_definition.h \
							  Language/parser_block.h \
							  Language/process.h \
							  Language/production_definition.h \
							  Language/toplevel_block.h \
					  		  Language/test_block.h \
					  		  Language/test_definition.h \
							  Lr/action_rewriter.h \
							  Lr/ast_parser.h \
							  Lr/conflict.h \
							  Lr/ignored_symbols.h \
							  Lr/lalr_builder.h \
							  Lr/lalr_machine.h \
							  Lr/lalr_state.h \
							  Lr/lr1_item_set.h \
							  Lr/lr_action.h \
							  Lr/lr_item.h \
							  Lr/lr_state.h \
							  Lr/parser.h \
							  Lr/parser_stack.h \
							  Lr/parser_state.h \
							  Lr/parser_tables.h \
							  Lr/weak_symbols.h \
							  TameParse.h \
							  Util/astnode.h \
							  Util/container.h \
							  Util/stringreader.h \
							  Util/syntax_ptr.h \
							  Util/unicode.h \
							  Util/utf8reader.h \
							  version.h

install-data-hook:
	$(LN_S) $(library_includedir) $(includedir)/TameParse
