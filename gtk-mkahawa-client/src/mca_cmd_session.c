/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_cmd_session.c
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

#include "mca_app.h"

#include <arpa/inet.h>

//#define DEBUG
#include "mca_debug.h"

extern MCA_App *mca;


void
stop_session (void)
{
  DEBUG_PRINT("stop_session(): %s\n", "entry");

  mca->bool_active = FALSE;
  if (mca->sec_timer_id > 0)
    g_source_remove (mca->sec_timer_id);
  mca->sec_timer_id  = 0;
  DEBUG_PRINT("stop_session(): %s\n", "exit");
}


void
start_session (void)
{
  DEBUG_PRINT("start_session(): %s\n", "entry");
  mca->start_time = time(NULL);
  mca_ipnl_set_time("00:00");
  mca_ipnl_set_owed("0.00");
  mca_ipnl_set_other("0.00");
  mca_ipnl_assist_able(FALSE);
  
  if (!mca->bool_active){
    DEBUG_PRINT("start_session(): %s\n", "activated");
    mca->timeout_len = 0;
    mca->bool_active = TRUE;
    mca->time_warned = 0;
    mca_app_set_login_mode(0);
    
    //mca_start_session_timer();
    mca->sec_timer_id = g_timeout_add_seconds(1, 
					      (GSourceFunc) mca_sec_timer, 
					      (gpointer) mca);
  }
  DEBUG_PRINT("start_session(): %s\n", "exit");
}

void
resume_session(void)
{
  DEBUG_PRINT("resume_session(): %s\n", "entry");
  if (!mca->bool_active){
    mca->bool_active = TRUE;
    //restart the timer
    mca->sec_timer_id = g_timeout_add_seconds(1, 
					    (GSourceFunc) mca_sec_timer, 
					    (gpointer) mca);
  }
}

void
pause_session (void)
{
  DEBUG_PRINT("pause_session(): %s\n", "entry");

  mca->bool_active = FALSE;
}


void
start_session_timeout(gint32 timeout_len)
{
  DEBUG_PRINT("start_session_timeout(): %s\n", "entry");
  mca->timeout_len = (guint32)timeout_len;
}


