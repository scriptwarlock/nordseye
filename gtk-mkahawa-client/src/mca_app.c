/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_app.c
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


#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "mca_app.h"

//#define DEBUG
#include "mca_debug.h"

#ifndef _
#define _(x) gettext (x)
#endif //_


extern MCA_App *mca;

static gint set_parse_val (MCA_App *app, gchar *itemstr, gchar *valstr);

static void
show_help(const char *appname)
{
  g_print(_(
"Usage: %s -host <server> -name <myname> [options]\n"
"Usage: %s -conf <config file>\n\n"
"<server>: the server's hostname or IP\n"
"<myname>: the name this client is going to be identified with\n"
"<config file>: configuration file. default is /etc/default/mkahawa-client\n\n"
"Options:\n"
"\t-port <port>: Connect to the specified port (default: 2999).\n"
"\t-nossl: do not use SSL to connect (default: use SSL).\n"
"\t-certpass <password>: password used for the cerfificate (default: none).\n"
"\t                      Ignored if not using SSL (-nossl option enabled).\n"),
					appname, appname);
}
gint 
parse_args (MCA_App *app, gint argc, gchar **argv)
{
  gint    retval = 1;
	gchar  *confname = NULL;
  int     i = 1;

  DEBUG_PRINT ("parse_args(): %d\n", argc);


  while (i < argc) {
    if (!strcmp(argv[i],"-host") && ++i < argc)
      app->cnxn.server_name = argv[i];
    else if (!strcmp(argv[i],"-name") && ++i < argc)
      app->cnxn.client_name = argv[i];
    else if (!strcmp(argv[i],"-port") && ++i < argc)
      app->cnxn.port = atoi(argv[i]);
    else if (!strcmp(argv[i],"-nossl"))
      app->cnxn.uses_ssl = 0;
    else if (!strcmp(argv[i],"-certpass") && ++i < argc)
      app->cnxn.cert_passwd = argv[i];
    else if (!strcmp(argv[i],"-dir") && ++i < argc)
      app->home_dir = g_string_new(argv[i]);
    else if (!strcmp(argv[i],"-conf") && ++i < argc)
      confname = argv[i];
    else {
      g_print(_("[E]Invalid option %s\n"),argv[i]);
      return 0;
    }
    ++i;
  }

	if ((argc < 5) && (confname == NULL)){  // -host , -name  are a must
		confname = "/etc/default/mkahawa-client";
	}
	if (confname != NULL)
		retval = parse_conf_file(mca, confname);

	if (retval)
		show_help(argv[0]);

  return retval;
}



gint
parse_conf_file (MCA_App *app, gchar *conf_name)
{
  gint     retval = 0;
  gchar    buf[256];
  gchar   *itemstr, *valstr;
  FILE    *fp;

  DEBUG_PRINT("parse_conf_file(): confname = %s\n", conf_name);
  fp = fopen(conf_name, "r");

  if (fp == NULL){
    DEBUG_PRINT("parse_conf_file(): Unable to open file [%s]\n", conf_name);
    return -1;
  }
  //memset(buf, 0, 256);
  while (fgets(buf, 256, fp)){
    //sscanf(buf, "%s=%s", itemstr, valstr);
    itemstr = strtok(buf, "=");
    valstr = strtok(NULL, "\n");
    if (valstr != NULL && itemstr != NULL){
		
		set_parse_val(app, itemstr, valstr);
      //DEBUG_PRINT("parse_conf_file(): %s: [%s][%s]\n", confname, itemstr, valstr);
      memset(buf, 0, 256);
    }
   }
  fclose(fp);

  return retval;
}


#define USERNAMELEN    20
#define CLIENTNAMELEN  50
#define SERVERNAMELEN  100
#define HOMEDIRLEN     128


static gint 
set_parse_val (MCA_App *app, gchar *itemstr, gchar *valstr)
{
  if (!strncasecmp(itemstr, "USERNAME", 8) && !app->cnxn.user_name){
    app->cnxn.user_name = strndup(valstr, USERNAMELEN);
    DEBUG_PRINT("set_parse_val(): username: %s\n", app->cnxn.user_name);
    return 1;
  }
  else if (!strncasecmp(itemstr, "SERVER_ADDR", 11) && !app->cnxn.server_name ){
    app->cnxn.server_name= strndup(valstr, SERVERNAMELEN);
    DEBUG_PRINT("set_parse_val(): server: %s\n", app->cnxn.server_name);
    return 1;
  }
  else if (!strncasecmp(itemstr, "CLIENT_NAME", 11) && !app->cnxn.client_name){
    app->cnxn.client_name= strndup(valstr, CLIENTNAMELEN);
    DEBUG_PRINT("set_parse_val(): client name: %s\n", app->cnxn.client_name);
    return 1;
  }
  else if (!strncasecmp(itemstr, "PORT", 11) && !app->cnxn.port){
    app->cnxn.port = atoi(valstr);
    DEBUG_PRINT("set_parse_val(): port: %d\n", app->cnxn.port);
    return 1;
  }
  else if (!strncasecmp(itemstr, "SSL", 3 )){
    if (!strncasecmp(valstr, "NO", 2))
      app->cnxn.uses_ssl = 0;
    else
      app->cnxn.uses_ssl = 1;
    DEBUG_PRINT("set_parse_val(): ssl status: %d\n", app->uses_ssl);
    return 1;
  }
  else if (!strncasecmp(itemstr, "HOMEDIR", 7 )){
    app->home_dir = g_string_new(valstr);
    DEBUG_PRINT("set_parse_val(): home directory: %s\n", app->home_dir->str);
    return 1;
  }
  return 0;
}

gint32
init_dir_files (MCA_App *app)
{
  GFile    *prop_file = NULL;
  gchar    *fname = NULL;
  GError   *error = NULL;

  if (app->home_dir == NULL)
    app->home_dir = g_string_new(g_get_home_dir());
  if (app->prop_fname == NULL){
    fname = g_build_filename(app->home_dir->str, ".mkahawa", "mkahawa.inf", NULL);
    app->prop_fname = g_string_new(fname);
  }
  //check existence or create
  prop_file = g_file_new_for_path(fname);
  if (!g_file_query_exists(prop_file, NULL)){
    GFileOutputStream *gfos;
    
    gfos = g_file_create(prop_file, G_FILE_CREATE_PRIVATE, NULL, &error);
    if (gfos == NULL){
      DEBUG_PRINT("init_dir_files(): Error: %s\n" ,error->message);
      g_error_free(error);
      return 0;
    }
    else{
      g_object_unref(gfos);
    }
  }
  return 1;
}

gint32
mca_poll_server (void)
{
  gint32 retval = 0;

  retval = mca_network_send_cmd (&mca->cnxn, MC_USAGE_AMOUNT, NULL, 0);
  
  return retval;
}

void
mca_exit_session (void)
{
  //stop the timer
  g_source_remove(mca->sec_timer_id);
  if (mca->bool_active){
    mca_network_send_cmd(&mca->cnxn, MC_END_SESSION, NULL, 0);
    DBG_PRINT("mca_exit_session(): %s\n", "Exiting session");
  }
  else{
    DBG_PRINT("mca_exit_session(): %s\n", "Only blocking the screen");
  }
  mca_blocker_block();
}

#define WARNING_TIME     120    // 2 minutes of warning time <-- FIX ME
gboolean 
mca_sec_timer (gpointer data)
{
  gboolean retval = TRUE;
  gchar    buf[8];
  gint     used_time, hours,  mins;

  static gint32 last_mins = 0; 

	used_time = time(NULL) - mca->start_time;
	hours = used_time / 3600;
	mins = (used_time % 3600) / 60;

  DEBUG_PRINT("mca_sec_timer(): %d\n", mca->mca_sec_count);
  //show time
  if ( hours > 0 && last_mins != mins) {// update display every min
    //    snprintf(buf,8,"%.2dh%.2dm",hours,mins);
    snprintf(buf, 8, "%.2d:%.2d", hours, mins);
		mca_ipnl_set_time(buf);
	}
  else if (hours == 0) {                                // update display every minute
    //    snprintf(buf, 8, "%.2dm%.2ds", mins, used_time % 60); 
    snprintf(buf, 8, "%.2d:%.2d", mins, used_time % 60); 
		mca_ipnl_set_time(buf);
  }
	else{
		// don't change anything
	}


  //poll server at poll intervals
  if (!(mca->mca_sec_count % mca->poll_interval)  && mca->cnxn.connected)
    mca_poll_server();
	else if (mca->info_panel.is_wnd_exit_up){ 
		//we need to update the exit window every second
		//this is inspite of the "poll interval"
    mca_poll_server();
		if ( hours > 0 ){
			snprintf(buf, 8, "%.2d:%.2d", hours, mins);
			mca_ipnl_set_time(buf);
		}
	}

  last_mins = mins;

  //in case this is a timeout session
  if (mca->timeout_len > 0){  
    if (used_time > mca->timeout_len)
      mca_exit_session();
    else if ( (mca->timeout_len - used_time) < WARNING_TIME &&
	      !mca->time_warned ){
      int    wtime  = WARNING_TIME;
      char   wstr[64];
      
      sprintf(wstr, _("You have less than %d minutes to browse."), wtime/60);
      mca->time_warned = 1;
      mca_ipnl_display_info(wstr);
      mca->poll_interval = 1000;
    }
  }
  //inc counter
  mca->mca_sec_count++;
  retval = mca->bool_active;

  return retval;
}



void
mca_app_set_login_mode(guint32 login_mode)
{
  mca->login_mode = login_mode;
  if (login_mode == OPMODE_TICKET  || login_mode == OPMODE_MEMBER)
    mca_ipnl_set_owed_lbl(_("Balance:"));
  else 
    mca_ipnl_set_owed_lbl(_("Owed:"));
}


gint32
mca_app_set_login_state(guint32 login_state)
{
  gint32 retval = 0;
  mca->login_state = login_state;
  //doing nothing
  return retval;
}



gint32 
mca_app_send_cmd (MCA_App *app, guint32 cmd, void *data, guint32 datalen)
{
  return mca_network_send_cmd(&app->cnxn, cmd, data, datalen);
}
