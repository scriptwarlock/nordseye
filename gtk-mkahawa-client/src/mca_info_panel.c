/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_info_panel.c
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

#include "mca_info_panel.h"
#include "ui_utils.h"

#include <libnotify/notify.h>

//#define DEBUG
#include "mca_debug.h"

static GtkWidget *mca_info_panel_host (GtkWidget *panel);

MCAInfoPanel *ipnl;




GtkWidget *
mca_info_panel_init (MCAInfoPanel *g_ipnl, cmd_callback_t exec_ipnl_event,
		     status_callback_t proc_status)
{
  GtkWidget *panel, *event_box;
  
  ipnl = g_ipnl;
  ipnl->notify_timeout = 15;
  ipnl->wnd_exit = exit_wnd_create(ipnl);
  ipnl->wnd_help = help_wnd_create(ipnl);
  ipnl->event_cb = exec_ipnl_event;
  ipnl->status_cb = proc_status;
	//ipnl->tray_icon = init_status_icon(ipnl);
	ipnl->tray_icon = tray_icon_create(ipnl);
	ipnl->is_wnd_help_up = FALSE;
	ipnl->is_wnd_exit_up = FALSE;
	ipnl->is_wnd_panel_up = FALSE;

  panel = info_panel_internal_create(ipnl);
  /*if (!info_panel_menu_init(panel)){
    g_print("Unable to init menu\n");
		}*/
  event_box = gtk_event_box_new ();
  //gtk_container_add (GTK_CONTAINER (event_box), image);
  /*  g_signal_connect (G_OBJECT (event_box), 
		    "button_press_event",
		    G_CALLBACK (info_panel_on_button_press),
		    panel);
  */
  gtk_widget_reparent(GTK_WIDGET(panel), GTK_WIDGET(event_box));

	ipnl->wnd_ipnl = mca_info_panel_host(event_box);

  return event_box;
}

/* functions for the Window that hosts the panel */

static gboolean
ipnl_destroy (GtkWidget *window, gpointer data)
{
  //MCAInfoPanel *ipnl = (MCAInfoPanel *)data;

	DEBUG_PRINT("Info panel destroyed: %s\n", "");


	//ipnl->is_wnd_panel_up = FALSE;

	return FALSE;
}

static gboolean 
ipnl_delete_event (GtkWidget *window, GdkEvent *event, gpointer data)
{
  g_print("Info panel deleted\n");

	if (ipnl->is_wnd_exit_up){
    gtk_widget_hide (GTK_WIDGET (ipnl->wnd_exit));
		ipnl->is_wnd_exit_up = FALSE;
	}
	else{
    gtk_widget_show_all(GTK_WIDGET (ipnl->wnd_exit));
		ipnl->is_wnd_exit_up = TRUE;
	}

	return TRUE;
}

static gboolean
ipnl_window_state_event (GtkWidget *widget, GdkEventWindowState *event, gpointer tray_icon)
{

  DBG_PRINT("Info panel window-state-event: [%08X] -> %08X\n", event->changed_mask, 
						event->new_window_state);

	if(event->changed_mask & GDK_WINDOW_STATE_ICONIFIED && 
		 (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED || 
			event->new_window_state & (GDK_WINDOW_STATE_ICONIFIED | 
																	GDK_WINDOW_STATE_MAXIMIZED) )){
		
		gtk_widget_hide (GTK_WIDGET(widget));
		gtk_status_icon_set_visible(GTK_STATUS_ICON(tray_icon), TRUE);
		DBG_PRINT(">>Info panel window-state-event: Iconified: %08X\n", event->new_window_state);
	}
	else if(event->changed_mask & GDK_WINDOW_STATE_WITHDRAWN && 
					(event->new_window_state & GDK_WINDOW_STATE_ICONIFIED || 
					 event->new_window_state & (GDK_WINDOW_STATE_ICONIFIED | 
																			 GDK_WINDOW_STATE_MAXIMIZED))) {
		
		//gtk_status_icon_set_visible(GTK_STATUS_ICON(tray_icon), FALSE);
		//gtk_widget_show_all (GTK_WIDGET(widget));
		DBG_PRINT(">>Info panel window-state-event: Withdrawn: %08X\n", event->new_window_state);
	}
	
	return FALSE;
}

static GtkWidget *
mca_info_panel_host (GtkWidget *panel)
{
	GtkWidget   *window;

	//host panel inside a normal window
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_add (GTK_CONTAINER(window), GTK_WIDGET(panel));

	gtk_window_set_title(GTK_WINDOW (window), "Mkahawa Cyber Client");
	gtk_widget_hide_on_delete(window);
	//gtk_window_set_position(GTK_WINDOW (window), GTK_WIN_POS_NONE);
	gtk_window_stick(GTK_WINDOW (window));
	gtk_window_set_keep_above(GTK_WINDOW (window), TRUE);
	//gtk_window_set_decorated(GTK_WINDOW (window), FALSE);
	gtk_window_set_deletable(GTK_WINDOW (window), FALSE);
	gtk_window_set_resizable(GTK_WINDOW (window), FALSE);

	g_signal_connect (G_OBJECT (window), "destroy", 
										G_CALLBACK (ipnl_destroy), 
										G_OBJECT (window));

	g_signal_connect (G_OBJECT (window), "delete-event", 
										G_CALLBACK (ipnl_delete_event), 
										NULL);
	
	g_signal_connect (G_OBJECT (window), "window-state-event", 
										G_CALLBACK (ipnl_window_state_event), 
										ipnl->tray_icon);
 
	ipnl->wnd_ipnl = window;

	gtk_widget_show_all(window);

	return window;
}


gint32 
mca_ipnl_passwd_editable (gboolean editable)
{
  gint32 retval = 0;
  
  DEBUG_PRINT("mca_ipnl_passwd_editable(): %d\n", editable);
  if (editable){
    
  }
  else{

  }
  return retval;
}

gint32 
mca_ipnl_member_editable (gboolean editable)
{
  gint32 retval = 0;;

  DEBUG_PRINT("mca_ipnl_member_editable(): %d\n", editable);
  if (editable){
    
  }
  else{

  }
  return retval;
}

gint32 
mca_ipnl_user_editable (gboolean editable)
{
  gint32 retval = 0;;

  DEBUG_PRINT("mca_ipnl_user_editable(): %d\n", editable);
  if (editable){
    
  }
  else{

  }
  return retval;
}

gint32 
mca_ipnl_ticket_editable (gboolean editable)
{
  gint32 retval = 0;;

  DEBUG_PRINT("mca_ipnl_ticket_editable(): %d\n", editable);
  if (editable){
    
  }
  else{

  }
  return retval;
}

gint32
mca_ipnl_ack_assist (gchar *respstr, gint32 resplen)
{
  gint retval = 0;
  //set timeout to hide the message
  //echo the message

  return retval;
}

gint32
mca_ipnl_hide_panel (void)
{
  gint32 retval = 0;
  DEBUG_PRINT("mca_ipnl_hide_panel(): %d\n", retval);
  return retval;
}

gint32
mca_ipnl_show_panel (void)
{
  gint32 retval = 0;

  DEBUG_PRINT("mca_ipnl_show_panel(): %d\n", retval);
  //set timeout to hide the message
  //echo the message

  return retval;
}


gint32 
mca_ipnl_member_loginable (gboolean loginable)
{
  gint32 retval = 0;;

  DEBUG_PRINT("mca_ipnl_member_editable(): %d\n", loginable);
  if (loginable){
    
  }
  else{

  }
  return retval;
}


gint32 
mca_ipnl_ticket_loginable (gboolean loginable)
{
  gint32 retval = 0;;

  DEBUG_PRINT("mca_ipnl_ticket_loginable(): %d\n", loginable);
  if (loginable){
    
  }
  else{

  }
  return retval;
}


gint32 
mca_ipnl_user_loginable (gboolean loginable)
{
  gint32 retval = 0;;

  DEBUG_PRINT("mca_ipnl_user_loginable(): %d\n", loginable);
  if (loginable){
    
  }
  else{

  }
  return retval;
}


gint32 
mca_ipnl_assist_able (gboolean editable)
{
  gint32 retval = 0;;

  DEBUG_PRINT("mca_ipnl_assist_able(): %d\n", editable);
  if (editable){
    
  }
  else{

  }
  return retval;
}


void  
mca_ipnl_set_owed_lbl (gchar * lblstr)
{

  gtk_label_set_text(GTK_LABEL(ipnl->lbl_name_owed), lblstr);
  
  //do nothing yet

}

void  
mca_ipnl_set_owed (gchar * owedstr)
{

  gtk_label_set_text(GTK_LABEL(ipnl->lbl_owed), owedstr);
	if (ipnl->is_wnd_exit_up){
		gchar sess_owed_buf[128];

		g_sprintf(sess_owed_buf, "Session Cost: %s", owedstr);
		gtk_label_set_text(GTK_LABEL(ipnl->lbl_exit_cost), sess_owed_buf);
	}
}

void  
mca_ipnl_set_other (gchar * otherstr)
{

  gtk_label_set_text(GTK_LABEL(ipnl->lbl_other), otherstr);

}

void  
mca_ipnl_set_time (gchar * timestr)
{
  gtk_label_set_text(GTK_LABEL(ipnl->lbl_time), timestr);
	if (ipnl->is_wnd_exit_up){
		gchar sess_time_buf[128];

		g_sprintf(sess_time_buf, "Session Time: %s", timestr);
		gtk_label_set_text(GTK_LABEL(ipnl->lbl_exit_time), sess_time_buf);
	}

}

void  
mca_ipnl_display_info (gchar *infostr)
{
  //#ifdef HAVE_NOTIFICATION
  NotifyNotification* notification;
  gboolean            success = TRUE;
  GError*             error = NULL;

  notify_init("gtk_mkahawa_client");
  /* try the mkahawa-client notification */
  notification = notify_notification_new (
					  "Message from Cyber Admin:",
					  infostr,
					  PACKAGE_DATA_DIR"/pixmaps/mkahawa-icon.png"
#if !defined(NOTIFY_VERSION_MINOR) || (NOTIFY_VERSION_MAJOR == 0 && NOTIFY_VERSION_MINOR < 7)
                                            , NULL
#endif
					  );
  notify_notification_set_timeout(notification, ipnl->notify_timeout * 1000);
  success = notify_notification_show (notification, &error);
  notify_uninit();

  if (!success) {
    //do nothing yet
  }
  DEBUG_PRINT("mca_ipnl_display_info(): %s\n", infostr);
}
