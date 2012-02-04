AC_PREREQ([2.68])
AC_CONFIG_MACRO_DIR([m4])
AC_INIT([TameParse], [1.0.0], [andrew@logicalshift.co.uk])
AC_CONFIG_SRCDIR([parsetool/main.cpp])
AC_CONFIG_HEADERS([config.h])

# Use -Os by default for C++ programs (-O2 is really astoundingly bad for some reason)
# (It's generally bad to change the default options, but if you're using gcc or clang,
# you really actually don't want them)
if test -z $CXXFLAGS; then
    CXXFLAGS='-Os -g'
fi

# Checks for programs.
AM_INIT_AUTOMAKE
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_CHECK_PROG(HAVE_XXD, xxd, yes)

if test -z $HAVE_XXD; then
	AC_MSG_ERROR([The xxd utility is required to compile the bootstrap language])
fi

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([getcwd])

# Check for boost
AX_BOOST_BASE([1.45])

AC_CONFIG_FILES([Makefile
                 Test/Makefile
                 TameParse/Makefile
                 bootstrap/Makefile
                 parsetool/Makefile
                 Examples/Makefile
                 Examples/Test/Makefile
                 TextEditors/Makefile
                 doxy/Makefile])
AC_OUTPUT
