/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_cmd_utils.c
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

extern MCA_App *mca;

gint32
block_screen (void)
{
  gint32 retval=0;

  mca_app_set_login_state(0);
  mca_app_set_login_mode(0);

  retval = mca_blocker_block();
  if (retval)
    mca_ipnl_hide_panel();
  
  return retval;
}

gint32
unblock_screen()
{
  gint32 retval = 0;

  retval = mca_blocker_unblock();
  if (retval)
    mca_ipnl_show_panel();

  return retval;
}

