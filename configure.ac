#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])
AC_INIT([opdis], [0.1], [community@thoughtgang.org])
AM_INIT_AUTOMAKE([1.9 gnu subdir-objects])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([opdis/opdis.c])
#AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

# Checks for programs.
AM_PROG_CC_C_O
AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB([libbfd.so], [bfd_init])
AC_CHECK_LIB([libopcodes.so], [init_disassemble_info])

# Checks for header files.
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([stdint.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([argp.h])
AC_CHECK_HEADERS([bfd.h])
AC_CHECK_HEADERS([dis-asm.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strdup])

# Enable debug
AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug],
        		               [build debug symbols (def=no)])],
    	      [debug_syms="$enableval"], [debug_syms=no])

if test x"$debug_syms" = x"yes"; then
    AM_CXXFLAGS="$AM_CXXFLAGS -ggdb -Wall -Werror -O0"
else
    AM_CXXFLAGS="$AM_CXXFLAGS -O2"
fi


AC_OUTPUT
