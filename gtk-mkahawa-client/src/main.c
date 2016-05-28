/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * main.c
 * Copyright (C) Bernard Owuor 2010 <owuor@unwiretechnologies.net>
 * 
 * gtk-mkahawa-client is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gtk-mkahawa-client is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <config.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "ui_utils.h"
#include "mca_app.h"


//#define DEBUG
#include "mca_debug.h"


/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif


cnxn_status_t
mca_net_status_report(gint status, gpointer data);
cnxn_status_t
mca_panel_status_report(gint status, gpointer data);

MCA_App            app;
MCA_App           *mca;

gint
mca_init (int argc, char **argv)
{
#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif
	gtk_set_locale ();

	mca = &app;
	memset(mca, 0, sizeof(app));

	parse_args(mca, argc, argv);

	//	parse_conf_file(mca, confname);
	init_dir_files(mca);

	//init command execution - is this necessary?
	mca_command_init();
	//init blocker
	mca_blocker_init(&mca->blocker);
	//init networking  
	mca_network_init(&mca->cnxn, (cmd_callback_t)mca_exec_net_command,
									 (status_callback_t) mca_net_status_report);

	//init other startup variables
	app.timeout_len = 0;
	app.bool_active   = FALSE;
	app.start_time  = 0;
	app.upd_data    = NULL;
	app.login_mode  = 0;
	app.login_state = 0;
	app.mca_sec_count = 0;
	app.poll_interval = 10;  //default poll interval

	return 0;
}

gint
main (int argc, char **argv)
{
	GtkWidget  *panel;

	g_thread_init(NULL);
	gtk_init(&argc, &argv);

	mca_init(argc, argv);

	panel = mca_info_panel_init(&app.info_panel, (cmd_callback_t) mca_exec_panel_command,
															(status_callback_t) mca_panel_status_report);
	//panel can be contained/hosted inside an applet or window
	
	//now start network
	mca_network_start(&app.cnxn);

	while (!app.cnxn.connected)  //wait until connected
		g_usleep(1000000);
	DEBUG_PRINT("isConnected:= %d\n", app.cnxn.connected);

	mca_network_send_cmd(&app.cnxn, MC_GET_STATUS, NULL, 0);

	gtk_main();

	return 1;
}
