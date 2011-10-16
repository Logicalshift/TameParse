#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([TameParse], [1.0.0], [andrew@logicalshift.co.uk])
AC_CONFIG_SRCDIR([parsetool/main.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

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
AX_BOOST_BASE([1.47])

AC_CONFIG_FILES([Makefile
                 Test/Makefile
                 bootstrap/Makefile
                 parsetool/Makefile])
AC_OUTPUT
