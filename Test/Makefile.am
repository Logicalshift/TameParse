noinst_PROGRAMS		= test
EXTRA_DIST 			= Test.1

test_CFLAGS			= -I$(top_srcdir) -I../TameParse
test_CXXFLAGS		= -I$(top_srcdir) -I../TameParse
test_LDADD			= $(top_builddir)/TameParse/libTameParse.la $(top_builddir)/Runtimes/C/libTameParseC.la

definition_tp.h: $(top_srcdir)/TameParse/Language/definition.tp
	ln -sf $(top_srcdir)/TameParse/Language/definition.tp ./definition.tp
	xxd -i "definition.tp" >definition_tp.h

ansic.tpdat: $(top_srcdir)/Examples/AnsiC.tp
	$(top_builddir)/parsetool/tameparse -T binary -o ansic -S "<Translation-Unit>" $(top_srcdir)/Examples/AnsiC.tp

../TameParse/Language/bootstrap.cpp: definition_tp.h

runtime_c.cpp: ansic.tpdat

test_SOURCES		= \
					  contextfree_firstset.h \
					  contextfree_followset.h \
					  dfa_multi_regex.h \
					  dfa_ndfa.h \
					  dfa_range.h \
					  dfa_single_regex.h \
					  dfa_symbol_deduplicate.h \
					  dfa_symbol_set.h \
					  dfa_symbol_translator.h \
					  language_bootstrap.h \
					  language_primary.h \
					  lr_lalr_general.h \
					  lr_weaksymbols.h \
					  runtime_c.h \
					  test_fixture.h \
					  ../TameParse/Language/bootstrap.h \
 					  \
					  contextfree_firstset.cpp \
					  contextfree_followset.cpp \
					  dfa_multi_regex.cpp \
					  dfa_ndfa.cpp \
					  dfa_range.cpp \
					  dfa_single_regex.cpp \
					  dfa_symbol_deduplicate.cpp \
					  dfa_symbol_set.cpp \
					  dfa_symbol_translator.cpp \
					  language_bootstrap.cpp \
					  language_primary.cpp \
					  lr_lalr_general.cpp \
					  lr_weaksymbols.cpp \
					  runtime_c.cpp \
					  ../TameParse/Language/bootstrap.cpp \
					  main.cpp \
					  test_fixture.cpp

TESTS 				= ./test
