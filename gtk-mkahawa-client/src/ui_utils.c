/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * ui_utils.c
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


#include <gtk/gtk.h>
#include "ui_utils.h"

#define UI_FILE             PACKAGE_UI_DIR"/info-panel.ui"
#define UI_EXIT_DLG_FILE    PACKAGE_UI_DIR"/info-panel-exit-dlg.ui"
#define UI_EXIT_WND_FILE    PACKAGE_UI_DIR"/info-panel-exit-wnd.ui"
#define UI_HELP_WND_FILE    PACKAGE_UI_DIR"/info-panel-help.ui"
#define UI_MENU_FILE        PACKAGE_UI_DIR"/info-panel-menu.ui"
#define UI_TRAY_ICON_FILE   PACKAGE_DATA_DIR"/pixmaps/mkahawa-icon.png"


static void  tray_icon_activated (GObject *tray_icon, gpointer *window);


GtkWidget*
info_panel_internal_create (MCAInfoPanel *ipnl)
{
  GtkWidget   *window;
  GtkBuilder  *builder;
  GError      *error = NULL;
	//	GdkColor     bg_col = {10, 255, 255, 0};

  
  builder = gtk_builder_new ();
  if (!gtk_builder_add_from_file (builder, UI_FILE, &error)) {
    g_warning ("Couldn't load builder file: %s", error->message);
    g_error_free (error);
  }
  
  gtk_builder_connect_signals (builder, NULL);

  //window = GTK_WIDGET (gtk_builder_get_object (builder, "winMain"));
  window = GTK_WIDGET (gtk_builder_get_object (builder, "hbox1"));
  
  ipnl->lbl_owed = GTK_WIDGET (gtk_builder_get_object (builder, "lblPrice"));
  ipnl->lbl_other = GTK_WIDGET (gtk_builder_get_object (builder, "lblOther"));
  ipnl->lbl_time = GTK_WIDGET (gtk_builder_get_object (builder, "lblTime"));
	//get and color their event boxes
	/*GtkWidget *ebx;
	ebx = GTK_WIDGET (gtk_builder_get_object (builder, "ebxPrice"));
	gtk_widget_modify_bg(ebx, GTK_STATE_NORMAL, &bg_col);
	gtk_object_unref(GTK_OBJECT (ebx));
	ebx = GTK_WIDGET (gtk_builder_get_object (builder, "ebxOther"));
	gtk_widget_modify_bg(ebx, GTK_STATE_NORMAL, &bg_col);
	gtk_object_unref(GTK_OBJECT (ebx));
	ebx = GTK_WIDGET (gtk_builder_get_object (builder, "ebxTime"));
	gtk_widget_modify_bg(ebx, GTK_STATE_NORMAL, &bg_col);
	gtk_object_unref(GTK_OBJECT (ebx));
	*/

	//these labels are simply for names
  ipnl->lbl_name_owed = GTK_WIDGET (gtk_builder_get_object (builder, "lblPriceName"));
  ipnl->lbl_name_other = GTK_WIDGET (gtk_builder_get_object (builder, "lblOtherName"));
  ipnl->lbl_name_time = GTK_WIDGET (gtk_builder_get_object (builder, "lblTimeName"));

  ipnl->btn_help = GTK_WIDGET (gtk_builder_get_object (builder, "btnHelp"));
  ipnl->btn_exit = GTK_WIDGET (gtk_builder_get_object (builder, "btnExit"));

  g_object_unref (builder);

  g_signal_connect (G_OBJECT (ipnl->btn_help), "button-press-event",
										G_CALLBACK (info_panel_on_btn_help_press),
										ipnl);

  g_signal_connect (G_OBJECT (ipnl->btn_exit), "button-press-event",
										G_CALLBACK (info_panel_on_btn_exit_press),
										ipnl);

	return window;
}


gboolean
info_panel_on_btn_exit_press (GtkWidget *event_box, 
			      GdkEventButton *event,
			      gpointer        data)
{
  //static int         window_shown;
  MCAInfoPanel      *ipnl = (MCAInfoPanel *)data;

  if (event->button != 1)
    return FALSE;
  
  if (!ipnl->is_wnd_exit_up){
    gtk_widget_show_all(GTK_WIDGET (ipnl->wnd_exit));
		ipnl->is_wnd_exit_up = TRUE;
	}
  else{
    gtk_widget_hide (GTK_WIDGET (ipnl->wnd_exit));
		ipnl->is_wnd_exit_up = FALSE;
  }
  //window_shown = !window_shown;
  return TRUE;
}

gboolean
info_panel_on_btn_help_press (GtkWidget      *event_box, 
                         GdkEventButton *event,
                         gpointer        data)
{
  //static int         window_shown;
  MCAInfoPanel      *ipnl = (MCAInfoPanel *)data;

  if (event->button != 1)
    return FALSE;

  if (!ipnl->is_wnd_help_up){
    gtk_widget_show_all( ipnl->wnd_help);
		ipnl->is_wnd_help_up = TRUE;
	}
  else{
    gtk_widget_hide (GTK_WIDGET (ipnl->wnd_help));
		ipnl->is_wnd_help_up = FALSE;
  }
  //window_shown = !window_shown;
  return TRUE;
}


GtkWidget*
exit_dlg_create (MCAInfoPanel *ipnl)
{
  GtkWidget   *window;
  GtkBuilder  *builder;
  GError      *error = NULL;
  
  builder = gtk_builder_new ();
  if (!gtk_builder_add_from_file (builder, UI_EXIT_DLG_FILE, &error)) {
    g_warning ("Couldn't load builder file: %s", error->message);
    g_error_free (error);
  }
  
  /* This is important */
  gtk_builder_connect_signals (builder, NULL);

  window = GTK_WIDGET (gtk_builder_get_object (builder, "dlgExit"));
  
  ipnl->lbl_exit_cost = GTK_WIDGET (gtk_builder_get_object (builder, "lblExitCost"));
  ipnl->lbl_exit_time = GTK_WIDGET (gtk_builder_get_object (builder, "lblExitTime"));
  ipnl->btn_exit_yes = GTK_WIDGET (gtk_builder_get_object (builder, "btnExitYes"));
  ipnl->btn_exit_no = GTK_WIDGET (gtk_builder_get_object (builder, "btnExitNo"));

  g_object_unref (builder);
  
  g_signal_connect (G_OBJECT (ipnl->btn_exit_yes), "button-press-event",
		    G_CALLBACK (on_btn_exit_yes_press),
		    ipnl);

  g_signal_connect (G_OBJECT (ipnl->btn_exit_no), "button-press-event",
		    G_CALLBACK (on_btn_exit_no_press),
		    ipnl);

  //gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
  gtk_window_set_deletable(GTK_WINDOW (window), FALSE);
  gtk_window_stick(GTK_WINDOW (window));
  gtk_window_set_resizable(GTK_WINDOW (window), FALSE);
  
  return window;
}


GtkWidget*
exit_wnd_create (MCAInfoPanel *ipnl)
{
  GtkWidget   *window;
  GtkBuilder  *builder;
  GError      *error = NULL;
  
  builder = gtk_builder_new ();
  if (!gtk_builder_add_from_file (builder, UI_EXIT_WND_FILE, &error)) {
    g_warning ("Couldn't load builder file: %s", error->message);
    g_error_free (error);
  }
  
  /* This is important */
  gtk_builder_connect_signals (builder, NULL);

  window = GTK_WIDGET (gtk_builder_get_object (builder, "wndExit"));
  
  ipnl->lbl_exit_cost = GTK_WIDGET (gtk_builder_get_object (builder, "lblExitCost"));
  ipnl->lbl_exit_time = GTK_WIDGET (gtk_builder_get_object (builder, "lblExitTime"));
  ipnl->btn_exit_yes = GTK_WIDGET (gtk_builder_get_object (builder, "btnExitYes"));
  ipnl->btn_exit_no = GTK_WIDGET (gtk_builder_get_object (builder, "btnExitNo"));

  g_object_unref (builder);
  
  g_signal_connect (G_OBJECT (ipnl->btn_exit_yes), "button_press_event",
		    G_CALLBACK (on_btn_exit_yes_press),
		    ipnl);

  g_signal_connect (G_OBJECT (ipnl->btn_exit_no), "button_press_event",
		    G_CALLBACK (on_btn_exit_no_press),
		    ipnl);

  //gtk_window_set_decorated(GTK_WINDOW (window), FALSE);
  //gtk_window_set_deletable(GTK_WINDOW (window), FALSE);
  gtk_window_set_keep_above(GTK_WINDOW (window), TRUE);
  gtk_window_set_resizable(GTK_WINDOW (window), FALSE);
  gtk_window_stick(GTK_WINDOW (window));
  //gtk_widget_hide_on_delete(GTK_WIDGET (window) );
  
  return window;
}


GtkWidget*
help_wnd_create (MCAInfoPanel *ipnl)
{
  GtkWidget   *window;
  GtkBuilder  *builder;
  GError      *error = NULL;
  
  builder = gtk_builder_new ();
  if (!gtk_builder_add_from_file (builder, UI_HELP_WND_FILE, &error)) {
    g_warning ("Couldn't load builder file: %s", error->message);
    g_error_free (error);
  }
  
  /* This is important */
  gtk_builder_connect_signals (builder, NULL);

  window = GTK_WIDGET (gtk_builder_get_object (builder, "wndHelp"));
  
  ipnl->btn_help_call = GTK_WIDGET (gtk_builder_get_object (builder, "btnHelpCall"));
  ipnl->btn_help_send = GTK_WIDGET (gtk_builder_get_object (builder, "btnHelpSend"));
  ipnl->btn_help_passwd  = GTK_WIDGET (gtk_builder_get_object (builder, "btnHelpPasswd"));
  ipnl->tv_help_qry   = GTK_WIDGET (gtk_builder_get_object (builder, "tvHelpQry"));
  ipnl->tb_help_qry   = GTK_TEXT_BUFFER (gtk_builder_get_object (builder, "tbHelp"));

  g_object_unref (builder);

  g_signal_connect (G_OBJECT (ipnl->btn_help_passwd), "button_press_event",
		    G_CALLBACK (on_btn_help_passwd_press),
		    ipnl);
  g_signal_connect (G_OBJECT (ipnl->btn_help_send), "button_press_event",
		    G_CALLBACK (on_btn_help_send_press),
		    ipnl);
  g_signal_connect (G_OBJECT (ipnl->btn_help_call), "button_press_event",
		    G_CALLBACK (on_btn_help_call_press),
		    ipnl);
  
  //gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
  //gtk_window_set_deletable(GTK_WINDOW (window), FALSE);
  gtk_window_set_keep_above(GTK_WINDOW (window), TRUE);
  gtk_window_set_resizable(GTK_WINDOW (window), FALSE);
  gtk_window_stick(GTK_WINDOW (window));
  //gtk_widget_hide_on_delete(GTK_WIDGET (window) );
  return window;
}


gboolean  
on_btn_help_call_press (GtkWidget *event_box, 
												GdkEventButton *event, 
												gpointer data)
{
  MCAInfoPanel      *ipnl = (MCAInfoPanel *)data;

  if (event->button != 1)
    return FALSE;

 if (ipnl->event_cb)
   ipnl->event_cb(IPNL_HELP_CALL, NULL, 0);

  return TRUE;
}

gboolean  
on_btn_help_passwd_press (GtkWidget *event_box, 
													GdkEventButton *event, 
													gpointer data)
{
  MCAInfoPanel      *ipnl = (MCAInfoPanel *)data;
 
  if (event->button != 1)
    return FALSE;
  
  if (ipnl->event_cb)
    ipnl->event_cb(IPNL_HELP_PASSWD, NULL, 0);

  return TRUE;
}

gboolean  
on_btn_help_send_press (GtkWidget *event_box, 
																	GdkEventButton *event, 
																	gpointer data)
{
  MCAInfoPanel      *ipnl = (MCAInfoPanel *)data;
	GtkTextIter        iter_start, iter_end;
	gchar             *chat_text;

  if (event->button != 1)
    return FALSE;

	//grab chat text
	gtk_text_buffer_get_start_iter(ipnl->tb_help_qry, &iter_start);
	gtk_text_buffer_get_end_iter(ipnl->tb_help_qry, &iter_end);
	chat_text = gtk_text_buffer_get_text(ipnl->tb_help_qry, &iter_start, &iter_end, FALSE);
	//process text
  if (ipnl->event_cb)
    ipnl->event_cb(IPNL_HELP_SEND, chat_text, 0);
	//g_print("%s\n", chat_text);
	//clear the text
	gtk_text_buffer_set_text(ipnl->tb_help_qry, "\0", -1);
	gtk_widget_grab_focus (ipnl->tv_help_qry);
	
	g_free(chat_text);

  return TRUE;
}

gboolean  
on_btn_exit_yes_press (GtkWidget *event_box, GdkEventButton *event, gpointer data)
{
  MCAInfoPanel      *ipnl = (MCAInfoPanel *)data;
 
  if (event->button != 1)
    return FALSE;
  
  //info_panel_on_btn_exit_press(event_box, event, data);
  if (ipnl->event_cb)
    ipnl->event_cb(IPNL_EXIT_YES, NULL, 0);

	gtk_widget_hide (GTK_WIDGET (ipnl->wnd_exit));
	ipnl->is_wnd_exit_up = FALSE;

  return TRUE;
}

gboolean  
on_btn_exit_no_press (GtkWidget *event_box, GdkEventButton *event, gpointer data)

{
  MCAInfoPanel      *ipnl = (MCAInfoPanel *)data;
 
  if (event->button != 1)
    return FALSE;
  
  //info_panel_on_btn_exit_press(event_box, event, data);

  if (ipnl->event_cb)
    ipnl->event_cb(IPNL_EXIT_NO, NULL, 0);

	gtk_widget_hide (GTK_WIDGET (ipnl->wnd_exit));
	ipnl->is_wnd_exit_up = FALSE;

  return TRUE;
}

GtkStatusIcon *
tray_icon_create(MCAInfoPanel *ipnl)
{
	GtkStatusIcon *tray_icon  = gtk_status_icon_new_from_file (UI_TRAY_ICON_FILE);
	//set tooltip
	gtk_status_icon_set_tooltip (tray_icon, "Mkahawa Client");
	//connect handlers for mouse events
	g_signal_connect(GTK_STATUS_ICON (tray_icon), "activate", 
									 GTK_SIGNAL_FUNC (tray_icon_activated), 
									 &ipnl->wnd_ipnl);
	//g_signal_connect(GTK_STATUS_ICON (tray_icon), "popup-menu", GTK_SIGNAL_FUNC (tray_icon_popup), menu);
	gtk_status_icon_set_visible(tray_icon, FALSE); //set icon initially invisible
    
	return tray_icon;
}

void 
tray_icon_activated(GObject *tray_icon, gpointer *window)
{
	static int window_shown = 0;

	if (!window_shown){
		gtk_widget_show(GTK_WIDGET(*window));
		gtk_window_deiconify(GTK_WINDOW(*window));
	}
	else{
		gtk_widget_hide(GTK_WIDGET(*window));
	}
	window_shown = !window_shown;

}

