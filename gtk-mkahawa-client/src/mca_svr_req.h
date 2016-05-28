/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_svr_req.h
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

#ifndef MCA_SVR_REQ_H
#define MCA_SVR_REQ_H

gint32    svr_req_start_by_user (guint32 profile);
gint32    svr_req_start_by_member (const gchar *mbr_name, 
				   const gchar *passwd, guint32 profile);
gint32    svr_req_start_by_ticket (const gchar *tkt_code, guint32 profile);

#endif  // MCA_SVR_REQ_H
