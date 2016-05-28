/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_update.h
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
#ifndef MCA_UPDATE_H
#define MCA_UPDATE_H

#define UPDATE_STATE_OFF  0
#define UPDATE_STATE_ON   1

typedef struct
{
  char sgnstr[5];
  char update_name[32];
  char dest_fname[24];
  char dest_path[128];
  long fsize;
  long fmode;
  long vernr;
  char verstr[16];
  long key;
  long lbuf;
  char cbuf[24];
} update_info_t;

#define MAXCHUNKSIZE 2048
typedef struct {
  guint32 pos;
  guint32 blen;
  gchar buf[MAXCHUNKSIZE];
} chunk_t;

#endif //MCA_UPDATE_H
