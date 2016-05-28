AUTOMAKE_OPTIONS = gnu

MAINTAINFERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libcclc.la

libcclc_la_SOURCES = \
cclc.c \
cclc.h \
util.c \
cclc_private.h

libcclc_la_DEPENDENCIES = $(top_builddir)/config.h
libcclc_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
include_HEADERS = cclc.h

AM_CFLAGS = -Wall -ansi -pedantic
