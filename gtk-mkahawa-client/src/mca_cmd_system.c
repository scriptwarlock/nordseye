/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_cmd_system.c
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
#include "stdlib.h"


//#define DEBUG
#include "mca_debug.h"


extern MCA_App *mca;

void
blank_monitor(void)
{
  gint32 retval;

  retval = system("xset dpms force off");
}

void 
reboot_system ()
{
  gint32 retval;

  retval = system("/sbin/reboot");
}

void     
shutdown_system (void)
{
  gint32 retval;

  retval = system("/sbin/halt");
}

void
exit_mca_program ()
{
  exit(0);
}

#define DIGEST_LEN 160/8

gint32
set_admin_passwd (gchar *pwdstr, guint32 pwd_len)
{
  GFile               *prop_file;
  GFileOutputStream   *fos;
  GError              *error = NULL;
  gchar                md5_passwd[DIGEST_LEN];
  gint32               retval = 0;

  prop_file = g_file_new_for_path(mca->prop_fname->str);
  fos = g_file_replace(prop_file, NULL, TRUE, G_FILE_CREATE_PRIVATE, NULL, &error);
  if (!fos){
    //retry without backup
    g_error_free(error);
    fos = g_file_replace(prop_file, NULL, FALSE , G_FILE_CREATE_PRIVATE, NULL, &error);
  }
  /***  FIX ME FIX ME  FIX ME   - no encryption because of no SSL support ***/
  if (fos){
    retval = g_output_stream_write( (GOutputStream *)fos, 
				    md5_passwd, DIGEST_LEN, NULL, &error);
  }
  if (error){
    g_error_free(error);
    retval = 0;
  }

  return retval;
}

gint32
set_poll_interval(gint32 poll_interval)
{
  if (poll_interval >= 0)
    mca->poll_interval = poll_interval;

  return mca->poll_interval;
}
