bin_PROGRAMS            = json_format

json_format_CFLAGS      = -I$(top_srcdir)
json_format_CXXFLAGS    = -I$(top_srcdir) $(BOOST_CPPFLAGS)
json_format_LDFLAGS     = $(BOOST_LDFLAGS) -lboost_system -lboost_filesystem -lboost_program_options
json_format_LDADD       = ../../TameParse/libTameParse.la

json_format_SOURCES     = \
						  json_format.cpp \
						  json.h \
						  json.cpp

json.h json.cpp: json.tp ../../parsetool/tameparse
	../../parsetool/tameparse --run-tests -o json -T cplusplus -S "<Object>" $(srcdir)/json.tp
