bin_PROGRAMS			= tameparse

tameparse_CFLAGS		= -I$(top_srcdir)
tameparse_CXXFLAGS		= -I$(top_srcdir) $(BOOST_CPPFLAGS)
tameparse_LDFLAGS		= $(BOOST_LDFLAGS) -lboost_system -lboost_filesystem -lboost_program_options
tameparse_LDADD			= ../TameParse/libTameParse.la

tameparse_SOURCES		= \
						  boost_console.h \
						  \
						  boost_console.cpp \
						  main.cpp
