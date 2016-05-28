/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_blocker.c
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


#include "mca_blocker.h"
#include "blocker_ui.h"
#include "mca_svr_req.h"
#include "utils.h"

#define DEBUG
#include "mca_debug.h"

MCABlocker *blkr;

gint32
mca_blocker_init (MCABlocker *blocker)
{
  gint retval = 0;
  
  blkr = blocker;
 
  blkr->wnd_blkr = blocker_ui_create(blkr);

  DBG_PRINT("mca_blocker_init(): blocker window [w=%d, h=%d]\n", blkr->width,
	    blkr->height);

  return retval;
}

gint32
mca_blocker_unblock (void)
{
  gint32 retval = 0;

  DEBUG_PRINT("mca_blocker_unblock(): %d\n", retval);

  gdk_keyboard_ungrab (GDK_CURRENT_TIME);

  gtk_widget_hide (GTK_WIDGET (blkr->wnd_blkr));

  return retval;
}

gint32
mca_blocker_block (void)
{
  gint32        retval = 0;
  
  DEBUG_PRINT("mca_blocker_block(): %d\n", retval);

  gtk_widget_show_all(GTK_WIDGET (blkr->wnd_blkr));
  DEBUG_PRINT("mca_blocker_block(): %s\n", "shown");

  DEBUG_PRINT("mca_blocker_block(): resized: [%dx%d]\n", blkr->width, 
	      blkr->height);

  if (! GTK_WIDGET_REALIZED(blkr->wnd_blkr) )
      gtk_widget_realize(blkr->wnd_blkr);
  DEBUG_PRINT("mca_blocker_block(): %s\n", "window realized");

  gdk_window_move_resize(blkr->wnd_blkr->window, 0,0, blkr->width, blkr->height);
  DEBUG_PRINT("mca_blocker_block(): %s\n", "moved and resized");

  while (gdk_keyboard_grab(blkr->wnd_blkr->window, FALSE, GDK_CURRENT_TIME) != GDK_GRAB_SUCCESS){
		DEBUG_PRINT("mca_blocker_block(): %s\n", "failed grabbing keys");
		count_seconds(1);
	}
  DEBUG_PRINT("mca_blocker_block(): %s\n", "grabbed keys");

  gdk_window_fullscreen(blkr->wnd_blkr->window);
  DEBUG_PRINT("mca_blocker_block(): %s\n", "shown & out");

  return retval;
}

gint32
mca_blocker_user_login (guint32 profile)
{
  gint32      retval = 0;
  
  DEBUG_PRINT("mca_blocker_user_login(): %d\n", retval);
  
  svr_req_start_by_user(profile);

  return retval;
}

gint32
mca_blocker_member_login (const gchar *mbr_name, const gchar *passwd, guint32 profile)
{
  gint32    retval = 0;


  DBG_PRINT("mca_blocker_member_login(): member name: [%s] password: [%s]\n", mbr_name, passwd);
  
  retval = svr_req_start_by_member(mbr_name, passwd, profile);

  //exit (1);

  return retval;
}

gint32  
mca_blocker_ticket_login (const gchar *tkt_code, guint32 profile)
{
  gint32    retval = 0;

  DBG_PRINT("mca_blocker_ticket_login(): ticket-code: [%s]\n", tkt_code);
  
  retval = svr_req_start_by_ticket(tkt_code, profile);

  return retval;
}

void
mca_blocker_ticket_clear (void)
{
  DEBUG_PRINT("mca_blocker_ticket_clear (): %s\n", "entry");
  clear_tkt_code(blkr);
}

void
mca_blocker_member_clear (void)
{
  DEBUG_PRINT("mca_blocker_member_clear (): %s\n", "entry");
  clear_mbr_name(blkr);
  clear_mbr_passwd(blkr);
}

void
mca_blocker_display_message (gchar *msg, guint32 display_time)
{
  DEBUG_PRINT("mca_blocker_member_clear (): %s\n", "entry");
  

}
