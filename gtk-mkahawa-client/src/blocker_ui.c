/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * blocker_ui.c
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
#include "blocker_ui.h"

//#define DEBUG

#define BLKR_UI_FILE  PACKAGE_UI_DIR"/blocker.ui"
#define BLKR_IMAGE    PACKAGE_UI_DIR"/lockpix.png"

gboolean         trap_ctrl_alt_key_state (GtkWidget *window, GdkEventKey *event, GtkWidget *display);
static guint32   get_login_profile (MCABlocker *blkr);
static void      set_blocker_bgs (MCABlocker *blkr, GtkWidget *win, gboolean Transient);

GtkWidget*
blocker_ui_create (MCABlocker *blkr)
{
  GtkWidget     *window;
  GtkBuilder    *builder;
  GdkScreen     *screen;
  GError        *error = NULL;
  
  builder = gtk_builder_new();
  if (!gtk_builder_add_from_file (builder, BLKR_UI_FILE, &error)) {
    g_warning ("Couldn't load builder file: %s", error->message);
    g_error_free (error);
  }
  
  gtk_builder_connect_signals (builder, NULL);
  window = GTK_WIDGET (gtk_builder_get_object (builder, "wndBlocker"));
  
  blkr->btn_usr_login = GTK_WIDGET (gtk_builder_get_object (builder, "btnUserLogin"));
  blkr->btn_mbr_login = GTK_WIDGET (gtk_builder_get_object (builder, "btnMemberLogin"));
  blkr->btn_tkt_login = GTK_WIDGET (gtk_builder_get_object (builder, "btnTicketLogin"));
  blkr->txt_mbr_name = GTK_ENTRY (gtk_builder_get_object (builder, "entryMemberName"));
  blkr->txt_mbr_passwd = GTK_ENTRY (gtk_builder_get_object (builder, "entryMemberPasswd"));
  blkr->txt_tkt_code = GTK_ENTRY (gtk_builder_get_object (builder, "entryTicketCode"));
  blkr->lbl_blkr_msg = GTK_LABEL (gtk_builder_get_object (builder, "lblBlockerMessage"));
  //blkr->cmb_profile = GTK_COMBO_BOX (gtk_builder_get_object (builder, "cmbProfile"));
  //gtk_widget_set_sensitive(GTK_WIDGET (blkr->cmb_profile), FALSE);

  blkr->frm_usr_login = GTK_WIDGET (gtk_builder_get_object (builder, "frmUserLogin"));
  blkr->frm_mbr_login = GTK_WIDGET (gtk_builder_get_object (builder, "frmMemberLogin"));
  blkr->frm_tkt_login = GTK_WIDGET (gtk_builder_get_object (builder, "frmTicketLogin"));

  g_object_unref (builder);

  g_signal_connect(G_OBJECT (blkr->btn_usr_login), "button_press_event",
		   G_CALLBACK (on_btn_usr_login_press),
		   blkr);
  g_signal_connect(G_OBJECT (blkr->btn_mbr_login), "button_press_event",
		   G_CALLBACK (on_btn_mbr_login_press),
		   blkr);
  g_signal_connect(G_OBJECT (blkr->btn_tkt_login), "button_press_event",
		   G_CALLBACK (on_btn_tkt_login_press),
		   blkr);

  g_signal_connect(G_OBJECT (window), "key-press-event", 
		   G_CALLBACK(trap_ctrl_alt_key_state),
		   blkr);

  gtk_window_stick(GTK_WINDOW (window));

  //  gtk_widget_hide(GTK_WIDGET(window));

  screen = gdk_screen_get_default();
  blkr->width = gdk_screen_get_width(screen);
  blkr->height = gdk_screen_get_height(screen);
  //create the image
  set_blocker_bgs(blkr, window, TRUE);

  return window;
}

#define TKT_PANEL_BGCOLOR   {0xFFFFFF, 0xFFFF, 0, 0}
#define MBR_PANEL_BGCOLOR   {0xAAAAAAAA, 0xFFFF, 0, 0}
#define USR_PANEL_BGCOLOR   {0xFFFFFF, 0xFFFF, 0, 0}


static void
set_blocker_color_bg (GtkWidget *win, const GdkColor *bg_col)
{

  gtk_widget_modify_bg(win, GTK_STATE_NORMAL, bg_col);
  
  //g_print ("set_blocker_color_bg(): changed color\n");
}


static void
set_blocker_image_bg (GtkWidget *win, const gchar *img_fname)
{
  GdkPixmap     *bg_pixmap;
  GdkPixbuf     *pixbuf;
  GError        *error = NULL;
  GtkStyle      *style;

  pixbuf = gdk_pixbuf_new_from_file (img_fname, &error);

	if (error != NULL){ //Try other default locations
		g_error_free(error);
		error = NULL;
		pixbuf = gdk_pixbuf_new_from_file ("/lockpix.png", &error);
	}

	if (error != NULL){ //Try another default location
		g_error_free(error);
		error = NULL;
		pixbuf = gdk_pixbuf_new_from_file ("lockpix.png", &error);
	}

  if (error != NULL) {  // just paint some color
    GdkColor  bg_col = {0x00FF00, 0x80, 0x80, 0};
    if (error->domain == GDK_PIXBUF_ERROR) {
      g_print ("Pixbuf Related Error:\n");
    }
    if (error->domain == G_FILE_ERROR) {
      g_print ("File Error: Check file permissions and state:\n");
    }
    g_printerr ("%s\n", error[0].message);

    gtk_widget_modify_bg(win, GTK_STATE_NORMAL, &bg_col);
		return;
  }
  //else { 
	gdk_pixbuf_render_pixmap_and_mask (pixbuf, &bg_pixmap, NULL, 0);
	
	style = gtk_style_copy( win->style );
	if (style->bg_pixmap[GTK_STATE_NORMAL])
		g_object_unref(style->bg_pixmap[GTK_STATE_NORMAL]);
	
	style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref(bg_pixmap);
	gtk_widget_set_style(win, style);
	g_object_unref(style);
  
	gdk_window_set_back_pixmap(win->window, bg_pixmap, FALSE);
	//}
}

static void
set_blocker_bgs (MCABlocker *blkr, GtkWidget *win, gboolean Transient)
{
  GdkColor       bg_color =  {0xAAAAAA00, 0xFFFF, 0, 0};

  set_blocker_image_bg (win, BLKR_IMAGE);
  set_blocker_color_bg (blkr->frm_tkt_login, &bg_color);
  set_blocker_color_bg (blkr->frm_mbr_login, &bg_color);
  set_blocker_color_bg (blkr->frm_usr_login, &bg_color);
  
  return;
}

gboolean 
trap_ctrl_alt_key_state (GtkWidget *window, GdkEventKey *event, GtkWidget *display) 
{
  if (event->state & GDK_MOD1_MASK){
    g_print("Alt key pressed\n");
  }
  if (event->state & GDK_CONTROL_MASK) {
    g_print("control key pressed\n");
  }

  g_print ("Key pressed\n");

  return FALSE;
}

static gboolean
func_delete_msg (gpointer data)
{
  GtkLabel *lbl = (GtkLabel *)data;

  gtk_label_set_text(lbl, "");
  
  return FALSE;
}

static void
blocker_show_timed_message (GtkLabel *lbl, const gchar *msg_str, guint32 duration)
{
  gtk_label_set_text(lbl, msg_str);
  g_timeout_add_seconds(duration, func_delete_msg, (gpointer) lbl);
}


static guint32
get_login_profile (MCABlocker *blkr)
{
  gint retval = 0;//gtk_combo_box_get_active(blkr->cmb_profile);
  
  if (retval < 0) 
    retval = 0;

  return retval;
}

gboolean
on_btn_usr_login_press (GtkWidget      *event_box, 
			GdkEventButton *event,
			gpointer        data)
{
  MCABlocker   *blkr = (MCABlocker *)data;
  guint32       profile;
  
  if (event->button != 1)
    return FALSE;
  
  profile = get_login_profile(blkr);

  mca_blocker_user_login(profile);

  return TRUE;
}

gboolean
on_btn_mbr_login_press (GtkWidget      *event_box, 
			GdkEventButton *event,
			gpointer        data)
{
  MCABlocker      *blkr = (MCABlocker *)data;
  GtkEntryBuffer  *name_buf, *passwd_buf;
  const gchar     *mbr_name, *passwd;
  guint32          profile;

  if (event->button != 1)
    return FALSE;
  
  profile = get_login_profile(blkr);

  name_buf = gtk_entry_get_buffer(blkr->txt_mbr_name);
  mbr_name = gtk_entry_buffer_get_text(name_buf);
 
  passwd_buf = gtk_entry_get_buffer(blkr->txt_mbr_passwd);
  passwd = gtk_entry_buffer_get_text(passwd_buf);

#ifdef DEBUG
  mca_blocker_member_login(mbr_name, passwd, profile);
#else
  blocker_show_timed_message (blkr->lbl_blkr_msg, "member login still disabled", 5);
#endif
  
  clear_mbr_name(blkr);
  clear_mbr_passwd(blkr);

  return TRUE;
}

gboolean
on_btn_tkt_login_press (GtkWidget      *event_box, 
			GdkEventButton *event,
			gpointer        data)
{
  MCABlocker      *blkr = (MCABlocker *)data;
  GtkEntryBuffer  *tkt_code_buf;
  const gchar     *tkt_code;
  guint32          profile;

  if (event->button != 1)
    return FALSE;
  
  profile = get_login_profile(blkr);

  tkt_code_buf = gtk_entry_get_buffer (blkr->txt_tkt_code);
  tkt_code = gtk_entry_buffer_get_text (tkt_code_buf);

  mca_blocker_ticket_login(tkt_code, profile);

  //blocker_show_timed_message(blkr->lbl_blkr_msg, "ticket login tried", 5);
  
  clear_tkt_code(blkr);

  return TRUE;
}

void
clear_mbr_name (MCABlocker *blkr)
{
  GtkEntryBuffer    *name_buf;

  name_buf = gtk_entry_get_buffer(blkr->txt_mbr_name);

  gtk_entry_buffer_delete_text(name_buf, 0, -1);
}

void
clear_mbr_passwd (MCABlocker *blkr)
{
  GtkEntryBuffer    *passwd_buf;

  passwd_buf = gtk_entry_get_buffer(blkr->txt_mbr_passwd);

  gtk_entry_buffer_delete_text(passwd_buf, 0, -1);
}

void
clear_tkt_code (MCABlocker *blkr)
{
  GtkEntryBuffer    *code_buf;

  code_buf = gtk_entry_get_buffer(blkr->txt_tkt_code);

  gtk_entry_buffer_delete_text(code_buf, 0, -1);
}
