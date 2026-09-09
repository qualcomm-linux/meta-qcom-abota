AUTOMAKE_OPTIONS = foreign subdir-objects

AM_CPPFLAGS = -I$(srcdir)/inc @EFIVAR_CFLAGS@

lib_LTLIBRARIES = libabctl.la

libabctl_la_SOURCES = src/libabctl.cpp src/efiabctl.cpp
libabctl_la_LIBADD = @EFIVAR_LIBS@
libabctl_la_LDFLAGS = -version-info 1:0:0 -no-undefined

libabctlincludedir = $(includedir)
libabctlinclude_HEADERS = inc/libabctl.h

sbin_PROGRAMS = abctl

abctl_SOURCES = src/abctl.cpp
abctl_LDADD = libabctl.la

sbin_SCRIPTS = scripts/abctl-helper.sh
