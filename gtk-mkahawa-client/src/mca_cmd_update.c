/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_cmd_update.c
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

#ifndef _
#define _(x) gettext (x)
#endif //_

extern MCA_App  *mca;
chunk_t  *chunk;

#define MAX_UPD_FSIZE  4000000

static gboolean
auth_update (gchar *hdr, guint32 hdr_len)
{
  update_info_t   *upd_info;

  if (hdr_len < sizeof(update_info_t))
    return 0; //size checking
  memset((char *)&mca->upd_info, 0, sizeof(update_info_t));
  upd_info  = (update_info_t *)hdr;
  mca->upd_info = *upd_info; 
  if (mca->upd_info.fsize > MAX_UPD_FSIZE) {
    DEBUG_PRINT("auth_update(): file is too large: [%ld bytes]\n", mca->upd_info.fsize);
    return FALSE;
  }
  if (mca->upd_data) {
    g_free(mca->upd_data);
    mca->upd_data = NULL;
  }
  mca->upd_data = (char *)g_malloc(mca->upd_info.fsize);
  if (!mca->upd_data){
    DEBUG_PRINT("auth_update(): Unable to allocate memory: %d\n", (gint32)retval);
    return FALSE;
  }

  return TRUE;
}


static gint32
process_update_data(void)
{
  /**** FIX ME ****/
  return 1;

}


static gint32
do_update(void)
{
  GFile               *upd_file;
  GFileOutputStream   *fos;
  GError              *error = NULL;
  gboolean             retval = FALSE;
  gchar               *fname;


  fname = g_build_filename(mca->upd_info.dest_path, mca->upd_info.dest_fname, NULL);
  upd_file = g_file_new_for_path(fname);
  fos = g_file_replace(upd_file, NULL, TRUE, G_FILE_CREATE_PRIVATE, NULL, &error);
  if (!fos){
    //retry without backup
    g_error_free(error);
    fos = g_file_replace(upd_file, NULL, FALSE , G_FILE_CREATE_PRIVATE, NULL, &error);
  }

  if (fos){
    //authenticate & decrypt
    if (!process_update_data())
      return 0;
    retval = g_output_stream_write( (GOutputStream *)fos, 
				    mca->upd_data, mca->upd_info.fsize, NULL, &error);
    if (!retval){
      DEBUG_PRINT("do_update(): Error: %s\n", error->message);
      g_error_free(error);
      error = NULL;
    }
    g_output_stream_close((GOutputStream *)fos, NULL, &error);
  }
  if (error){
    g_error_free(error);
    retval = 0;
  }

  return retval;
}



gint32
store_update_chunk(void *data, guint32 datalen)
{
  gint32 retval=1;

  memcpy((mca->upd_data + chunk->pos), chunk->buf, chunk->blen);

  return retval;
}

gint32
start_mkw_update (gchar *hdr, guint32 hdr_len)
{
  gint32 ack, retval;

  ack = htonl(0);
  if (auth_update(hdr, hdr_len))
    ack = htonl(1);

  retval = mca_app_send_cmd(mca, MC_UPD_STATUS, (void *)&ack, sizeof(ack));
  
  return retval;
}

gint32
proc_mkw_update_data (gchar *data, guint32 datalen)
{
  gint32 resp, chunk_size, n_resp, retval = 0;

  mca->mkw_update_state = UPDATE_STATE_ON;
  chunk = (chunk_t *)data;
  resp = chunk->pos;
  chunk_size = chunk->blen;
  if (store_update_chunk(data, datalen))
    resp += chunk_size;
  n_resp = htonl(resp);
 
  return retval;
}

gint32
end_mkw_update (gchar *data, guint32 datalen)
{
  gint32 retval = 0;

  if (do_update()){
    guint32 resp;

    resp = htonl(1);
    retval = mca_app_send_cmd(mca, MC_END_UPDATE, &resp, sizeof(resp));
  }
  else {
    gchar      msg[256], *cp;
    guint32    msglen, *resp;
    
    resp = (guint32 *)&msg;
    *resp = htonl(0);
    cp = (gchar *)msg;
    cp += sizeof (resp);
    msglen = g_strlcpy(cp, (const gchar *)_("Unable to write update file"), 240);
    msglen += sizeof (resp);
    retval = mca_app_send_cmd(mca, MC_END_UPDATE, (void *)msg, msglen);
  }

  return retval;
}
