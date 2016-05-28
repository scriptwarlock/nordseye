/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_command.h
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

#ifndef MCA_COMMAND_H
#define MCA_COMMAND_H

#include <gtk/gtk.h>

#include "mca_app.h"

//server-to-client message
#define  MKW_OWED_CASH          0x000A  //    CS_SETOWED
#define  MKW_PRODUCTS_CASH      0x000B  //CS_SETADDITIONAL
#define  MKW_SESSION_TIME       0x0009  //CS_SETTIME
#define  MKW_SERVER_MESSAGE     0x000D  //CS_DISPLAYMESSAGE
#define  MKW_ACK_ASSIST         0x0014  //CS_CALLASSIST
//session control
#define  MKW_STOP_SESSION       0x0001  //CS_STOP
#define  MKW_START_SESSION      0x0002  //CS_START
#define  MKW_PAUSE_SESSION      0x0003  //CS_PAUSE
#define  MKW_RESUME_SESSION     0x0004  //CS_RESUME
#define  MKW_TIMEOUT_SESSION    0x0006  //CS_SETTIMEOUT
//system control
#define  MKW_BLANK_MONITOR       0x0005   //CS_MONITOROFF
#define  MKW_SHUTDOWN_SYSTEM     0x0007   //CS_SHUTDOWN
#define  MKW_REBOOT_SYSTEM       0x0008  //CS_REBOOT
#define  MKW_EXIT_MCA            0x000C  //CS_QUITCLIENT
#define  MKW_SET_ADMIN_PASSWD    0x0019   //CS_SETADMINPASS
#define  MKW_SET_POLL_INTERVAL   0x001D  //CS_SETPOLLINTERVAL
//utils 
#define  MKW_UNBLOCK_SCREEN      0x000E  //CS_UNLOCKSCREEN
#define  MKW_BLOCK_SCREEN        0x000F  //CS_LOCKSCREEN

//UI and program state
#define  MKW_ENABLE_PASSWD_BUTTON    0x0010 //CS_ENABLEPASSWORDEDIT
#define  MKW_ALLOW_MEMBER_LOGIN      0x0011 //CS_ALLOWMEMBERLOGIN
#define  MKW_ALLOW_TICKET_LOGIN      0x001B //CS_ALLOWTICKETLOGIN
#define  MKW_ALLOW_USER_LOGIN        0x0012 //CS_ALLOWUSERLOGIN
#define  MKW_ENABLE_ASSIST_BUTTON    0x0015 //CS_ENABLEASSIST
//update
#define  MKW_START_UPDATE    0x0016  //CS_UPDATE
#define  MKW_UPDATE_DATA     0x0017  //CS_UPDATEDATA
#define  MKW_END_UPDATE      0x0018  //CS_UPDATEEND
//client info
#define  MKW_CLIENT_VERSION  MKW_END_UPDATE + 1 // CS_UPDATEEND + 1




/* Commands sent by the client */
#define MC_END_SESSION      0x0001  //CC_USEREXIT	//  1UL	/* The user ended the session */
#define MC_USER_START       0x0002  //CC_USERLOGIN	//  2UL	/* The user wants to start a new session */
#define MC_GET_STATUS       0x0004  //CC_GETSTATUS	//  4UL	/* Request the status (time, owed, etc) */
#define MC_USAGE_TIME       0x0005  //CC_GETTIME	//  5UL	/* Request the used time */
#define MC_USAGE_AMOUNT     0x0006  //CC_GETOWED	//  6UL	/* Request the amount owed by the user */
#define MC_REQ_TIMEOUT      0x0007  //CC_GETTIMEOUT	 //  7UL	/* Request the timeout */
#define MC_MEMBER_START     0x0008  //CC_MEMBERLOGIN	  // 8UL	/* Login with member id */
#define MC_SET_PASSWD       0x0009  //CC_SETMEMBERPASSWORD  //9UL/* Change the password for this member */
#define MC_NAME_START       0x000A  //CC_MEMBERLOGINWITHNAME  //10UL /* Login with member name */
#define MC_PRINT_OUT        0x000B  //CC_USERPRINTED    // 11UL  /* Report user printing */
#define MC_CLIENT_CHAT      0x000C  //CC_CHATCLIENT     // 12UL  /* Chat from client */
#define MC_CALL_ASSIST      0x000D  //CC_CALLASSIST     // 13UL  /* Call assistant */
#define MC_UPD_STATUS       0x000E  //CC_UPDATE         // 14UL  /* Respond to CS_UPDATE */
#define MC_UPD_DATA_STATUS  0x000F  //CC_UPDATEDATA     // 15UL  /* Respond to CS_UPDATEDATA */
#define MC_END_UPDATE       0x0010  //CC_UPDATEEND      // 16UL  /* Respond to CS_UPDATEEND */
#define MC_SEND_VERSION     0x0011  //CC_VERSION        // 17UL  /* Respond to CS_REQVERSION */
#define MC_ALERT_SERVER     0x0012  //CC_ALERTSERVER    // 18UL  /* Send alert to Server */
#define MC_TICKET_START     0x0013  //CC_TICKETLOGIN    // 19UL  /* Start logging in by ticket */
#define MC_MAX_CMD_NUM      0x0013  //CC_MAXCMDNR       // 19UL  /* Maximum Command ID*/




gint32 mca_command_init (void);
gint32 mca_exec_command (guint32 cmd, void *data, guint32 cmdsize);
gint32 mca_exec_net_command (guint32 cmd, void *data, guint32 cmdsize);
gint32 mca_exec_panel_command (guint32 cmd, void *data, guint32 cmdsize);

#endif // CCA_COMMAND_H
