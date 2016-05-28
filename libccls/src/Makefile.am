AUTOMAKE_OPTIONS = gnu

MAINTAINFERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libccls.la

libccls_la_SOURCES = \
ccl.c \
client.c \
data.c \
member.c \
employee.c \
tarif.c \
logging.c \
util.c \
ccl_private.h

libccls_la_DEPENDENCIES = $(top_builddir)/config.h
libccls_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
include_HEADERS = ccls.h

INCLUDES = $(GLIB_CFLAGS)
libccls_la_LIBADD = -L/usr/local/lib $(GLIB_LIBS)
AM_CFLAGS = -Wall -ansi -pedantic
