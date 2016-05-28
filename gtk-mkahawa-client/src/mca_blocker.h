/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_blocker.h
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
#ifndef MCA_BLOCKER_H
#define MCA_BLOCKER_H

#include <gtk/gtk.h>

typedef struct _MCABlocker
{
  gint32  blocker;

  gboolean     allow_mbr_login;
  gboolean     allow_tkt_login;
  gboolean     allow_usr_login;

  GtkWidget   *wnd_blkr;

  GtkWidget   *btn_usr_login;
  GtkWidget   *btn_mbr_login;

  GtkWidget   *frm_usr_login;
  GtkWidget   *frm_mbr_login;
  GtkWidget   *frm_tkt_login;

  GtkEntry    *txt_mbr_name;
  GtkEntry    *txt_mbr_passwd;

  GtkWidget   *btn_tkt_login;
  GtkEntry    *txt_tkt_code;

  GtkLabel    *lbl_blkr_msg;
  
  GtkComboBox *cmb_profile;

  GtkImage    *blkr_img;
  GdkPixmap   *bg_pixmap;

  guint32      width, height;


} MCABlocker;


gint32  mca_blocker_init (MCABlocker *blocker);
gint32  mca_blocker_block (void);
gint32  mca_blocker_unblock (void);
gint32  mca_blocker_user_login (guint32 profile);

gint32  mca_blocker_member_login (const gchar *mbr_name, const gchar *passwd, guint32 profile);
gint32  mca_blocker_ticket_login (const gchar *ticket_code, guint32 profile);

void    mca_blocker_ticket_clear (void);
void    mca_blocker_member_clear (void);

void    mca_blocker_display_message (gchar *msgstr, guint32 display_time);

#endif //MCA_BLOCKER_H
