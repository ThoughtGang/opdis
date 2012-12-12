#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([opdis], [1.0.5], [community@thoughtgang.org])
AM_INIT_AUTOMAKE([1.9 gnu subdir-objects])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([opdis/opdis.c])
AC_CONFIG_FILES([Makefile])

# Checks for programs.
AM_PROG_CC_C_O
AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB([bfd], [bfd_init], [], [AC_MSG_ERROR([Missing GNU binutils])])
AC_CHECK_LIB([opcodes], [init_disassemble_info], [], [AC_MSG_ERROR([Missing GNU binutils])])

# Checks for header files.
AC_CHECK_HEADERS([sys/types.h], [], [AC_MSG_ERROR([Missing UNIX libc headers])])
AC_CHECK_HEADERS([stdint.h], [], [AC_MSG_ERROR([Missing GNU libc headers])])
AC_CHECK_HEADERS([stdlib.h], [], [AC_MSG_ERROR([Missing libc headers])])
AC_CHECK_HEADERS([string.h], [], [AC_MSG_ERROR([Missing libc headers])])
AC_CHECK_HEADERS([bfd.h], [], [AC_MSG_ERROR([Missing GNU binutils headers])])
AC_CHECK_HEADERS([dis-asm.h], [], [AC_MSG_ERROR([Missing GNU binutils headers])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strdup])

# Allow disabling of CLI utility
AC_ARG_ENABLE([cli], [AS_HELP_STRING([--enable-cli],
	      			     [build opdis cli utility [default=yes]])],
	      [cli_build="$enableval"], [cli_build=yes])
AM_CONDITIONAL([BUILD_CLI], [test  x"$cli_build" = x"yes"])

# Only check for argp.h if CLI is being built
if test x"$cli_build" != x"no"; then
	AC_CHECK_HEADERS([argp.h], [], 
			 [AC_MSG_ERROR([Missing GNU libc headers])])
fi

# Enable debug
AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug],
        		               [build debug symbols [default=no]])],
    	      [debug_syms="$enableval"], [debug_syms=no])

if test x"$debug_syms" = x"yes"; then
    AM_CXXFLAGS="$AM_CXXFLAGS -ggdb -Wall -Werror -O0"
else
    AM_CXXFLAGS="$AM_CXXFLAGS -O2"
fi

AC_OUTPUT
