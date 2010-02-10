#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])
AC_INIT([opdis], [0.1], [dev@thoughtgang.org])
AM_INIT_AUTOMAKE([1.9 gnu])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([opdis/opdis.c])
#AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB([libbfd.so], [bfd_init])
AC_CHECK_LIB([libopcodes.so], [init_disassemble_info])

# Checks for header files.
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([bfd.h])
AC_CHECK_HEADERS([dis-asm.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strdup])

AC_OUTPUT
