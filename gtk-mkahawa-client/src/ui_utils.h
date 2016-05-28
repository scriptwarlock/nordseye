/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * ui_utils.h
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
#ifndef UI_UTILS_H
#define UI_UTILS_H

#include "mca_info_panel.h"

GtkWidget*     info_panel_internal_create (MCAInfoPanel *ipnl);
GtkWidget*     info_panel_menu_create (void);

GtkWidget*     exit_dlg_create (MCAInfoPanel *ipnl);
GtkWidget*     exit_wnd_create (MCAInfoPanel *ipnl);
GtkWidget*     help_wnd_create (MCAInfoPanel *ipnl);

GtkStatusIcon*   tray_icon_create(MCAInfoPanel *ipnl);


gint           info_panel_menu_init (GtkWidget *menu);

gboolean  info_panel_on_btn_exit_press (GtkWidget *event_box, GdkEventButton *event, gpointer data);
gboolean  info_panel_on_btn_help_press (GtkWidget *event_box, GdkEventButton *event, gpointer data);

gboolean  on_btn_help_call_press (GtkWidget *event_box, GdkEventButton *event, gpointer data);
gboolean  on_btn_help_passwd_press (GtkWidget *event_box, GdkEventButton *event, gpointer data);
gboolean  on_btn_help_send_press (GtkWidget *event_box, GdkEventButton *event, gpointer data);

gboolean  on_btn_exit_yes_press (GtkWidget *event_box, GdkEventButton *event, gpointer data);
gboolean  on_btn_exit_no_press (GtkWidget *event_box, GdkEventButton *event, gpointer data);


#endif // UI_UTILS_H
