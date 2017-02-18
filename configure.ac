#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([src/dev_alarm.c])
AC_CONFIG_HEADERS([config.h])

AC_PREFIX_DEFAULT(~)

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([memset munmap])

AC_CONFIG_FILES([src/Makefile libuser/Makefile pterm/Makefile Makefile libdisk/Makefile config.mk])
AC_OUTPUT
