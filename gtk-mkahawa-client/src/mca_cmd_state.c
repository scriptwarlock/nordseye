/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_cmd_state.c
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

#include "mca_cmd_state.h"
#include "mca_app.h"


//#define DEBUG
#include "mca_debug.h"

gint32
set_passwd_edit_state(gint32 bool)
{
  mca_ipnl_passwd_editable((bool)? TRUE: FALSE);

  return bool; 
}

gint32
set_member_loginable(gint32 bool)
{
  mca_app_set_login_state(0);
  mca_app_set_login_mode(0);

  mca_ipnl_member_loginable((bool)? TRUE: FALSE);
  return bool;
}
  
gint32
set_ticket_loginable(gint32 bool)
{
  mca_app_set_login_state(0);
  mca_app_set_login_mode(0);

  mca_ipnl_ticket_loginable((bool)? TRUE: FALSE);
  return bool;
}


gint32
set_user_loginable(gint32 bool)
{
  mca_app_set_login_mode(0);

  mca_ipnl_user_loginable((bool)? TRUE: FALSE);
  return bool;
}

gint32
set_assist_able(gint32 bool)
{
  mca_ipnl_assist_able((bool)? TRUE: FALSE);

  return bool;
}
