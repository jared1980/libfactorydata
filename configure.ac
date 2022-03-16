#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libfactorydata], [0.1])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([lib/operations.c])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CC
AM_PROG_AR

# Checks for libraries.
LT_INIT

# Checks for header files.
AC_CHECK_HEADERS([stdint.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([strdup strstr])

AC_CONFIG_FILES([Makefile
                 lib/Makefile])
AC_OUTPUT
