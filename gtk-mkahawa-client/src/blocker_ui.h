/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * blocker_ui.h
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

#ifndef BLOCKER_UI_H
#define BLOCKER_UI_H

#include "mca_blocker.h"

GtkWidget*     blocker_ui_create (MCABlocker *blkr);

GtkWidget*     tkt_login_create (MCABlocker *blkr);
GtkWidget*     mbr_login_create (MCABlocker *blkr);
GtkWidget*     usr_login_create (MCABlocker *blkr);

gboolean  on_btn_mbr_login_press (GtkWidget *event_box, GdkEventButton *event, gpointer data);
gboolean  on_btn_tkt_login_press (GtkWidget *event_box, GdkEventButton *event, gpointer data);
gboolean  on_btn_usr_login_press (GtkWidget *event_box, GdkEventButton *event, gpointer data);


void      clear_mbr_name (MCABlocker *blkr);
void      clear_mbr_passwd (MCABlocker *blkr);
void      clear_tkt_code (MCABlocker *blkr);

#endif // UI_UTILS_H
