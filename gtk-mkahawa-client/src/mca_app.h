/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_app.h
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
#ifndef MCA_APP_H
#define MCA_APP_H


#include "mca_network.h"
#include "mca_info_panel.h"
#include "mca_blocker.h"
#include "mca_command.h"
#include "mca_svr_req.h"
#include "mca_update.h"

#include <gtk/gtk.h>
#include <libintl.h>


#define OPMODE_POSTPAY   1
#define OPMODE_TICKET    2
#define OPMODE_MEMBER    4


#define MEMBER_LOGIN_NAME 1
#define MEMBER_LOGIN_PWD  2


typedef struct _MCA_App
{
  gint               port;
  char             *server_name;
  char             *client_name;
  char             *username;
  int               uses_ssl;
  MCACnxn           cnxn;
  MCABlocker        blocker;
  MCAInfoPanel      info_panel;

  update_info_t     upd_info;
  gchar            *upd_data;

  gboolean          bool_active;
  guint32           start_time;
  guint32           timeout_len;
  GString          *home_dir;
  GString          *prop_fname;
  guint32           login_mode;
  guint32           login_state;
  gint32            poll_interval;
  gboolean          bool_passwd_edit;
  guint32           mkw_update_state;
  guint32           time_warned;
  guint32           session_time_len;
  guint32           ack_assist;

  guint             sec_timer_id;
  guint32           mca_sec_count;
} MCA_App;

gint32 parse_args (MCA_App *app, gint argc, gchar **argv);
gint32 parse_conf_file (MCA_App *app, gchar *conf_name);
gint32 init_dir_files (MCA_App *app);

void     mca_app_set_login_mode (guint32 login_mode);
gint32   mca_app_set_login_state (guint32 login_state);
gint32   mca_app_send_cmd (MCA_App *app, guint32 cmd, void *data, guint32 datalen);
gboolean mca_sec_timer (gpointer data);

void     mca_exit_session (void);

//include command implementations
#include "mca_cmd_session.h"
#include "mca_cmd_system.h"
#include "mca_cmd_state.h"
#include "mca_cmd_message.h"
#include "mca_cmd_utils.h"
#include "mca_cmd_update.h"
#include "mca_cmd_panel.h"

#endif
