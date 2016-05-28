/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_cmd_message.h
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
#ifndef MCA_CMD_MESSAGE_H
#define MCA_CMD_MESSAGE_H

#include "mca_app.h"


void     set_owed_cash (gint32 amt_deci);
void     set_products_cash (gint32 amt_deci);
void     set_time_len (gint32 svr_time);
void     show_server_message (gchar *msgstr);
guint32  ack_assist_request(gchar *respstr, guint32 resp_len);


#endif //MCA_CMD_MESSAGE_H
