## Process this file with automake to produce Makefile.in

## Created by Anjuta

uidir = $(datadir)/gtk-mkahawa-client/ui

ui_DATA = 	ui/info-panel-exit-dlg.ui  \
		ui/info-panel-exit-wnd.ui  \
		ui/info-panel-help.ui  \
		ui/info-panel.ui       \
		ui/blocker.ui          \
		ui/lockpix.png          \
		pixmaps/mkahawa-icon.png


AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_UI_DIR=\""$(uidir)"\" \
	$(GTK_MKAHAWA_CLIENT_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -ggdb

bin_PROGRAMS = gtk-mkahawa-client

gtk_mkahawa_client_SOURCES = \
	utils.c \
	ui_utils.c \
	mca_network_utils.c \
	mca_network.c \
	mca_info_panel.c \
	mca_cmd_utils.c \
	mca_cmd_state.c \
	mca_cmd_session.c \
	mca_cmd_message.c \
	mca_cmd_system.c \
	mca_cmd_update.c \
	mca_cmd_panel.c \
	mca_command.c \
	mca_svr_req.c \
	blocker_ui.c \
	mca_blocker.c \
	mca_status.c \
	mca_app.c \
	main.c

gtk_mkahawa_client_LDFLAGS = \
	-Wl,--export-dynamic

gtk_mkahawa_client_LDADD = $(GTK_MKAHAWA_CLIENT_LIBS)

EXTRA_DIST = $(ui_DATA)

# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(uidir)
	-rm -r $(datadir)/gtk-mkahawa-client
