## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src po

gtk_mkahawa_clientdocdir = ${prefix}/doc/gtk_mkahawa_client
gtk_mkahawa_clientdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS


INTLTOOL_FILES = intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

EXTRA_DIST = $(gtk_mkahawa_clientdoc_DATA) \
	$(INTLTOOL_FILES)

DISTCLEANFILES = intltool-extract \
	intltool-merge \
	intltool-update \
	po/.intltool-merge-cache


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(gtk_mkahawa_clientdocdir)
