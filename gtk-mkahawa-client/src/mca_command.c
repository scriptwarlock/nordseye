/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_command.c
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

#include <assert.h>
#include "mca_command.h"
#include "mca_app.h"

#include <arpa/inet.h>

//#define DEBUG
#include "mca_debug.h"

extern MCA_App *mca;

gint32
mca_command_init(void)
{
  int retval = 0;

  //do nothing
  return retval;
}

/*
  This command interpreter  adopts and extends the binary protocol as 
  defined in CCL <ccl.sourceforge.net> and earlier versions of mkahawa 
  <mkahawa.sourceforge.net> projects. This is to achieve compatibility 
  with the 2 projects' servers. There are notable changes in 
  implementation of the commands
*/

static gint32
mca_exec_svr_command (guint32 cmd, void *data, guint32 datalen)
{
  gint     retval = 0;
  gint32   nr_data_h = 0;
  
  DEBUG_PRINT("mca_exec_command(): cmd=%08X [%d bytes]\n", cmd, datalen);

  if (datalen > 0)
    nr_data_h = ntohl(*(guint32 *)data);
  switch (cmd) {
    //server-to-client message/command
  case MKW_OWED_CASH:      //CS_SETOWED:
    DBG_PRINT("mca_exec_command(): cmd = MKW_OWED_CASH: [%d]\n", nr_data_h);
    set_owed_cash(nr_data_h);
    break;
  case MKW_PRODUCTS_CASH:   //CS_SETADDITIONAL:
    DBG_PRINT("mca_exec_command(): cmd = MKW_PRODUCTS_CASH: [%d]\n", datalen);
    set_products_cash(nr_data_h);
    break;
  case MKW_SESSION_TIME:    //CS_SETTIME:
    DBG_PRINT("mca_exec_command(): cmd = MKW_SESSION_TIME: [%d]\n", datalen);
    set_time_len (nr_data_h);
    break;
  case MKW_SERVER_MESSAGE: //CS_DISPLAYMESSAGE:
    DBG_PRINT("mca_exec_command(): cmd = MKW_SERVER_MESSAGE: [%d]\n", datalen);
    show_server_message((gchar *)data);
    break;
  case MKW_ACK_ASSIST:        //CS_CALLASSIST:
    DBG_PRINT("mca_exec_command(): cmd = MKW_ACK_ASSIST: [%d]\n", datalen);
    ack_assist_request((gchar *) data, datalen);
    break;
    //session control
  case MKW_STOP_SESSION: //CS_STOP:
    DBG_PRINT("mca_exec_command(): cmd = MKW_STOP_SESSION: [%d]\n", datalen);
    stop_session();
    break;
  case MKW_START_SESSION: //CS_START:
    DBG_PRINT("mca_exec_command(): cmd = MKW_START_SESSION: [%d]\n", datalen);
    start_session();
    break;
  case MKW_PAUSE_SESSION: //CS_PAUSE:
    DBG_PRINT("mca_exec_command(): cmd = MKW_PAUSE_SESSION: [%d]\n", datalen);
    pause_session();
    break;
  case MKW_RESUME_SESSION: //CS_RESUME:
    DBG_PRINT("mca_exec_command(): cmd = MKW_RESUME_SESSION: [%d]\n", datalen);
    resume_session();
    break;
  case MKW_TIMEOUT_SESSION:   //CS_SETTIMEOUT:
    DBG_PRINT("mca_exec_command(): cmd = MKW_TIMEOUT_SESSION: [%d]\n", datalen);
    start_session_timeout(nr_data_h);
    break;
    //system control
  case MKW_BLANK_MONITOR: //CS_MONITOROFF:
    DBG_PRINT("mca_exec_command(): cmd = MKW_BLANK_MONITOR: [%d]\n", datalen);
    blank_monitor();
    break;
  case MKW_SHUTDOWN_SYSTEM: // CS_SHUTDOWN:
    DBG_PRINT("mca_exec_command(): cmd = MKW_SHUTDOWN_SYSTEM: [%d]\n", datalen);
    shutdown_system();
    break;
  case MKW_REBOOT_SYSTEM:   //CS_REBOOT:
    DBG_PRINT("mca_exec_command(): cmd = MKW_REBOOT_SYSTEM: [%d]\n", datalen);
    reboot_system();
    break;
  case MKW_EXIT_MCA:     //CS_QUITCLIENT:
    DBG_PRINT("mca_exec_command(): cmd = MKW_EXIT_MCA: [%d]\n", datalen);
    exit_mca_program();
    break;
  case MKW_SET_ADMIN_PASSWD: //CS_SETADMINPASS:
    DBG_PRINT("mca_exec_command(): cmd = MKW_SET_ADMIN_PASSWD: [%d]\n", datalen);
    set_admin_passwd((gchar *)data, datalen);
    break;
  case MKW_SET_POLL_INTERVAL:   //  CS_SETPOLLINTERVAL:
    DBG_PRINT("mca_exec_command(): cmd = MKW_SET_POLL_INTERVAL: [%d]\n", datalen);
    set_poll_interval( nr_data_h  );
    break;
    //utils 
  case MKW_UNBLOCK_SCREEN : //CS_UNLOCKSCREEN:
    DBG_PRINT("mca_exec_command(): cmd = MKW_UNBLOCK_SCREEN: [%d]\n", datalen);
    unblock_screen();
    break;
  case MKW_BLOCK_SCREEN:    //CS_LOCKSCREEN:
    DBG_PRINT("mca_exec_command(): cmd = MKW_BLOCK_SCREEN: [%d]\n", datalen);
    block_screen();
    break;
    //UI and program state
  case MKW_ENABLE_PASSWD_BUTTON:    //CS_ENABLEPASSWORDEDIT:
    DBG_PRINT("mca_exec_command(): cmd = MKW_ENABLE_PASSWD_BUTTON: [%d]\n", datalen);
    set_passwd_edit_state(nr_data_h);
    break;
  case MKW_ALLOW_MEMBER_LOGIN:  // CS_ALLOWMEMBERLOGIN:
    DBG_PRINT("mca_exec_command(): cmd = MKW_ALLOW_MEMBER_LOGIN: [%d]\n", datalen);
    set_member_loginable(nr_data_h);
    break;
  case MKW_ALLOW_TICKET_LOGIN:  //CS_ALLOWTICKETLOGIN:
    DBG_PRINT("mca_exec_command(): cmd = MKW_ALLOW_TICKET_LOGIN: [%d]\n", datalen);
    set_ticket_loginable(nr_data_h);
    break;
  case MKW_ALLOW_USER_LOGIN: //CS_ALLOWUSERLOGIN:
    DBG_PRINT("mca_exec_command(): cmd = MKW_ALLOW_USER_LOGIN: [%d]\n", datalen);
    set_user_loginable(nr_data_h);
    break;
  case MKW_ENABLE_ASSIST_BUTTON: //CS_ENABLEASSIST:
    DBG_PRINT("mca_exec_command(): cmd = MKW_ENABLE_ASSIST_BUTTON: [%d]\n", datalen);
    set_assist_able(nr_data_h);
    break;
    //update
  case MKW_START_UPDATE:    //CS_UPDATE:
    DBG_PRINT("mca_exec_command(): cmd = MKW_START_UPDATE: [%d]\n", datalen);
    start_mkw_update ((gchar *) data, datalen);
    break;
  case MKW_UPDATE_DATA:  // CS_UPDATEDATA:
    DBG_PRINT("mca_exec_command(): cmd = MKW_UPDATE_DATA: [%d]\n", datalen);
    proc_mkw_update_data((gchar *)data, datalen);
    break;
  case MKW_END_UPDATE:  //CS_UPDATEEND:
    DBG_PRINT("mca_exec_command(): cmd = MKW_END_UPDATE: [%d]\n", datalen);
    end_mkw_update((gchar *)data, datalen);
    break;
  }

  return retval;
}


/*
  Information panel command interpreter. 
*/

static gint32 
mca_exec_ipnl_command (guint32 cmd, void *data, guint32 cmdsize)
{
  gint32 retval = 0;

  DBG_PRINT("mca_exec_ipnl_command(): cmd=%08X\n", cmd);
  switch (cmd){
  case IPNL_HELP_CALL:
    retval = user_call_assist();
    break;
  case IPNL_HELP_SEND:   
    retval = user_send_message();
    break;
  case IPNL_HELP_PASSWD:
    retval = user_change_passwd();
    break;
  case IPNL_EXIT_YES:
    retval = user_end_session();
    break;
  case IPNL_EXIT_NO:
    retval = user_cancel_exit();
    break;
  }
  return retval;
}


/*
  Blocker command interpreter. 
*/

static gint32 
mca_exec_blkr_command (guint32 cmd, void *data, guint32 cmdsize)
{
  gint32 retval = 0;

  DBG_PRINT("mca_exec_blkr_command(): cmd=%08X\n", cmd);
  switch (cmd){
  case IPNL_HELP_CALL:
    retval = user_call_assist();
    break;
  case IPNL_HELP_SEND:   
    retval = user_send_message();
    break;
  case IPNL_HELP_PASSWD:
    retval = user_change_passwd();
    break;
  case IPNL_EXIT_YES:
    retval = user_end_session();
    break;
  case IPNL_EXIT_NO:
    retval = user_cancel_exit();
    break;
  }
  return retval;
}



gint32 
mca_exec_net_command (guint32 cmd, void *data, guint32 cmdsize)
{
  gint32 retval = 0;

  DBG_PRINT("mca_exec_net_command(): cmd=%08X\n", cmd);
  retval = mca_exec_svr_command(cmd, data, cmdsize);

  return retval;
}


gint32 
mca_exec_panel_command (guint32 cmd, void *data, guint32 cmdsize)
{
  gint32 retval = 0;

  DBG_PRINT("mca_exec_panel_command(): cmd=%08X\n", cmd);

  mca_exec_ipnl_command(cmd, data, cmdsize);

  return retval;
}

gint32 
mca_exec_blocker_command (guint32 cmd, void *data, guint32 cmdsize)
{
  gint32 retval = 0;

  DBG_PRINT("mca_exec_panel_command(): cmd=%08X\n", cmd);

  mca_exec_blkr_command(cmd, data, cmdsize);

  return retval;
}
