dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(gtk-mkahawa-client, 0.1)

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC


dnl ***************************************************************************
dnl Internatinalization
dnl ***************************************************************************
GETTEXT_PACKAGE=gtk-mkahawa-client
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.35.0])


AM_PROG_LIBTOOL

PKG_CHECK_MODULES(GTK_MKAHAWA_CLIENT, [gtk+-2.0 >= 2.12, libnotify >= 0.4, gthread-2.0])

AC_CHECK_LIB(ssl, SSL_new,,AC_MSG_ERROR([please install openssl]))
AC_CHECK_HEADER(openssl/ssl.h,,AC_MSG_ERROR([please install openssl headers]))


AC_OUTPUT([
Makefile
src/Makefile
po/Makefile.in
])
