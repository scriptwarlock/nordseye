/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_info_panel.h
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


#ifndef MCA_INFO_PANEL_H
#define MCA_INFO_PANEL_H

#include <gtk/gtk.h>
#include "mca_types.h"

#define IPNL_HELP_CALL         0x00000001
#define IPNL_HELP_SEND         0x00000002
#define IPNL_HELP_PASSWD       0x00000003

#define IPNL_EXIT_YES          0x00000011
#define IPNL_EXIT_NO           0x00000012


typedef struct _MCAInfoPanel{

  GtkWidget    *wnd_ipnl;
	

  GtkWidget    *lbl_owed;
  GtkWidget    *lbl_other;
  GtkWidget    *lbl_time;
  GtkWidget    *lbl_name_owed;
  GtkWidget    *lbl_name_other;
  GtkWidget    *lbl_name_time;
  GtkWidget    *btn_help;
  GtkWidget    *btn_exit;

  //exit dialog
  GtkWidget    *lbl_exit_cost;
  GtkWidget    *lbl_exit_time;
  GtkWidget    *btn_exit_yes;
  GtkWidget    *btn_exit_no;

  //help window
  GtkWidget        *btn_help_call;
  GtkWidget        *btn_help_passwd;
  GtkWidget        *btn_help_send;
  GtkWidget        *tv_help_qry;
  GtkTextBuffer    *tb_help_qry;


  GtkWidget        *dlg_exit;
  GtkWidget        *wnd_exit;
  GtkWidget        *wnd_help;

	gboolean          is_wnd_help_up;
	gboolean          is_wnd_exit_up;
	gboolean          is_wnd_panel_up;
	//tray icon
	GtkStatusIcon    *tray_icon;


  gint              notify_timeout;
  cmd_callback_t    event_cb;
  status_callback_t status_cb;
} MCAInfoPanel;

GtkWidget *mca_info_panel_init();


void mca_ipnl_set_owed (gchar *owed_str);
void mca_ipnl_set_other (gchar *other_str);
void mca_ipnl_set_time (gchar *other_str);
void mca_ipnl_display_info (gchar *msgstr);
void mca_ipnl_enable_help_btn ();
gint32 mca_ipnl_passwd_editable (gboolean editable); 
gint32 mca_ipnl_member_loginable (gboolean loginable);
gint32 mca_ipnl_user_loginable (gboolean loginable);
gint32 mca_ipnl_ticket_loginable (gboolean loginable);
gint32 mca_ipnl_ack_assist (gchar *respstr, gint32 resplen);
gint32 mca_ipnl_assist_able (gboolean bool);
gint32 mca_ipnl_hide_panel (void);
gint32 mca_ipnl_show_panel (void);

void  mca_ipnl_set_owed_lbl (gchar *lblstr);
void  mca_ipnl_set_owed (gchar *owedstr);
void  mca_ipnl_set_other (gchar *other_str);

#endif // MCA_INFO_PANEL_H
