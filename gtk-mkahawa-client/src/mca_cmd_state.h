/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_cmd_state.h
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

#ifndef MCA_CMD_STATE_H
#define MCA_CMD_STATE_H

#include <gtk/gtk.h>

gint32    set_passwd_edit_state (gint32 bool);
gint32    set_member_loginable (gint32 bool);
gint32    set_ticket_loginable (gint32 bool);
gint32    set_user_loginable (gint32 bool);
gint32    set_assist_able (gint32 bool);

#endif //MCA_CMD_STATE_H
