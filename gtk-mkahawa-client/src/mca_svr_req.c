/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_svr_req.c
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

#include <string.h>
#include <arpa/inet.h>

//#define DEBUG
#include "mca_debug.h"

extern MCA_App *mca;


gint32
svr_req_start_by_user (guint32 profile)
{
  gint32      retval = 0;
  guint32     n_profile = htonl(profile);

  DEBUG_PRINT("svr_req_start_by_user(): %s\n", "entry");
  retval =  mca_app_send_cmd(mca, MC_USER_START, &n_profile, sizeof(n_profile));

  return retval;
}

gint32
svr_req_start_by_member (const gchar *mbr_name, 
			 const gchar *passwd, guint32 profile)
{
  gint32      retval = 0;
  guint32     n_profile = htonl(profile);

  DEBUG_PRINT("svr_req_start_by_member(): [%s]\n", "entry");


  /***  FIX ME FIX ME  FIX ME   - no encryption yet ***/

  retval =  mca_app_send_cmd(mca, MC_MEMBER_START, &n_profile, sizeof(n_profile));

  return retval;
}

gint32
svr_req_start_by_ticket (const gchar *tkt_code, guint32 profile)
{
  gint32     retval = 0;
  gchar     *n_profile;
  gchar     *send_buf = NULL;
  guint32    code_len = strlen(tkt_code) + 1; // the "1" is for /0
  
  DEBUG_PRINT("svr_req_start_by_ticket(): [%s]\n", tkt_code);

  send_buf = g_malloc(code_len + sizeof(profile));  

  strncpy(send_buf, tkt_code, code_len);
  
  n_profile = send_buf + code_len;
  *n_profile = (guint32) htonl(profile);
	  
  retval =  mca_app_send_cmd(mca, MC_TICKET_START, send_buf, code_len + sizeof(profile));

  return retval;
}
