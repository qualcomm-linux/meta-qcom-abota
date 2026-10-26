AUTOMAKE_OPTIONS = foreign subdir-objects

AM_CPPFLAGS = -D_GNU_SOURCE \
              -D_LARGEFILE64_SOURCE \
              @OPENSSL_CFLAGS@ \
              @ABCTL_CFLAGS@

bin_PROGRAMS = mirror_copy

mirror_copy_SOURCES = mirror_copy.cpp
mirror_copy_LDADD = @OPENSSL_LIBS@ @ABCTL_LIBS@
