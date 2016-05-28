/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_cmd_message.c
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


void
set_owed_cash (gint32 amt_deci)
{
  gchar     amt_str[32];

  DEBUG_PRINT("set_owed_cash(): %d\n", amt_deci);
  amt_deci = (amt_deci + 50) / 100;   //round-up
  g_snprintf(amt_str, 32, "%.2f", (double)amt_deci);
  mca_ipnl_set_owed(amt_str);
}

void
set_products_cash (gint32 amt_deci)
{
  gchar     amt_str[32];
  
  DEBUG_PRINT("set_products_cash(): %d\n", amt_deci);
  g_snprintf(amt_str, 32, "%.2f", amt_deci / 100.0);
  mca_ipnl_set_other(amt_str);
}

void
set_time_len (gint32 svr_time)
{
  DEBUG_PRINT("set_time_len(): %d\n", svr_time);
  mca->session_time_len = time(NULL) - svr_time;
  mca->start_time = time(NULL) - svr_time;
}

void
show_server_message(gchar *msgstr)
{
  mca_ipnl_display_info(msgstr);
}

guint32  
ack_assist_request(gchar *respstr, guint32 resp_len)
{
  guint32 retval = 0;
  mca->ack_assist = 1;
  mca_ipnl_ack_assist(respstr, resp_len);
  mca_ipnl_assist_able(TRUE);

  return retval;
}
  
