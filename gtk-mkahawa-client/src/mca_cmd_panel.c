/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_cmd_panel.c
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

//#define DEBUG
#include "mca_debug.h"


gint32
user_call_assist (void)
{
  gint32 retval = 0;
  DBG_PRINT("user_call_assist(): %d\n", retval);
  
  return retval;
}

gint32
user_send_message (void)
{
  gint32 retval = 0;
  DBG_PRINT("user_send_message (): %d\n", retval);
  
  return retval;
}

gint32
user_change_passwd (void)
{
  gint32 retval = 0;
  DBG_PRINT("user_change_passwd (): %d\n", retval);
  
  return retval;
}


gint32
user_end_session (void)
{
  gint32 retval = 0;
  DBG_PRINT("user_end_session (): %d\n", retval);
  
  mca_exit_session();


  return retval;
}

gint32
user_cancel_exit (void)
{
  gint32 retval = 0;
  DBG_PRINT("user_cancel_exit (): %d\n", retval);

  return retval;
}
