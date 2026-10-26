#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([mirror_copy], [1.0], [])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([mirror_copy.cpp])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_INSTALL

# Checks for libraries.
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([OPENSSL], [libcrypto])
AC_SUBST([OPENSSL_CFLAGS])
AC_SUBST([OPENSSL_LIBS])

# libabctl provides the A/B slot control API (libabctl.h / -labctl).
# It ships no pkg-config file, so link it directly.
ABCTL_CFLAGS=""
ABCTL_LIBS="-labctl"
AC_SUBST([ABCTL_CFLAGS])
AC_SUBST([ABCTL_LIBS])

AC_LANG_PUSH([C++])
AC_CHECK_HEADER([libabctl.h], [],
    [AC_MSG_ERROR([libabctl.h not found; qcom-efiabctl-abota is required])])
AC_LANG_POP([C++])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
