/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/*
 * mca_network.c
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
#include "mca_network.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#include <mca_network_private.h>

//#define DEBUG
#include "mca_debug.h"

MCACnxn *mca_cnxn;

#define BUFLEN   256
char cmdBuf[BUFLEN];

static gint32   mca_connect_server (MCACnxn *cnxn);
static gint32   net_read (MCACnxn *cnxn, void *data, guint32 datalen);
static gint32   net_write (MCACnxn *cnxn, void *data, guint32 datalen);
static gint32   send_signature (MCACnxn *cnxn);


/*
  mca_network_init (MCACnxn *cnxn)
  
  Completes initialization of MCACnxn 

  exec_net_cmd      --- callback used when network command is executed
  net_status_report --- callback for reporting connection status

  //ideally should also validate other cnxn members  

  returns value > 1 if successful
 */
gint32 
mca_network_init (MCACnxn *cnxn, cmd_callback_t exec_net_cmd, status_callback_t net_status_report)
{
  gint32 retval = 1; 
  
  cnxn->cmd_cb = exec_net_cmd;
  cnxn->status_cb = net_status_report;
  cnxn->retrysec = 3;  //retry connecting every 3 seconds
  if (!cnxn->port)
    cnxn->port = 2999;

	init_ssl("CA.pem", "cert.pem", cnxn->cert_passwd, cnxn->uses_ssl, &retval);

  return retval;
}

/*
  mca_network_start (MCACnxn *cnxn)
  
  Starts network connection thread
  
  This function starts the connection thread
  cnxn should already be allocated but has partial connection info

  Returns > 0 on success, 0 on failure. 
 */
gint32 
mca_network_start (MCACnxn *cnxn)
{
  GError     *error;
  gint32      retval = 1;
  
  assert(cnxn != NULL);
  cnxn->connected = FALSE;
  //start the thread that connects and listens to server
  cnxn->cnxn_thread = g_thread_create ( (GThreadFunc) mca_network_cnxn_thread, 
					(gpointer) cnxn, FALSE, &error);
  //error
  if (cnxn->cnxn_thread == NULL){
    DEBUG_PRINT("mca_network_connect(): %s\n", (char *)error);
    retval = 0;
  }
  return retval;
}

/*
  mca_network_cnxn_thread (MCACnxn *cnxn)

  Connects to server, wait for commands and executes them.
  It retrys to connect to the server in case the connection fails

  This function never returns
 */
cnxn_status_t
mca_network_cnxn_thread (MCACnxn *cnxn)
{
  gint32    retval = 0;
  gint32    cmd;

  while (1){
    DEBUG_PRINT ("mca_network_cnxn_thread(): Attempt to connect: %d\n", retval);
    retval = mca_connect_server(cnxn);
    if (!retval){
      DEBUG_PRINT("mca_network_cnxn_thread(): failed to connect to %s\n", cnxn->server_name);
      cnxn->connected = FALSE;
      count_seconds(cnxn->retrysec);
      continue; 
    }
    DEBUG_PRINT("mca_network_cnxn_thread(): connected to %s\n", cnxn->server_name);
    //identify myself to server
    send_signature(cnxn);

    //wait for commands and process them when we received 
    while ((cmd = mca_network_wait_for_cmd(cnxn))){
      retval = cnxn->cmd_cb(cmd, cnxn->cmd_data, cnxn->cmd_data_len);
      DEBUG_PRINT("mca_network_cnxn_thread(): executed [%08X], returned [%d]\n", cmd, retval);
    }
    //cmd == 0 - connection must have broken or flow was destroyed
    DEBUG_PRINT("mca_network_cnxn_thread(): connection to %s broke\n", cnxn->server_name);
    mca_network_close_connection(cnxn);
    count_seconds(cnxn->retrysec);
  }

  return 0;
}

/*
  mca_network_close_connection(MCACnxn *cnxn)

  Reset the connection data 
*/
void
mca_network_close_connection(MCACnxn *cnxn)
{
  DEBUG_PRINT("mca_network_close_connection(): closing connection: %d\n", 1);
  cnxn->connected = FALSE;
	close_connection(cnxn->conn_priv);

	cnxn->connected = FALSE;
      
  return;
}

/*
  mca_network_cmd_wait_for_cmd(MCACnxn *cnxn)

  Wait for and read command sent from the server 

  Returns the received command, but also updates cmd field in cnxn
 */
gint32
mca_network_wait_for_cmd (MCACnxn *cnxn)
{
  //  GInputStream *server = NULL;
  gint32     cmd = 0, cmd_data_size = 0;
  gint32     cmd_n = 0, cmd_data_size_n = 0;
  gssize     retsize = 0;

  assert(cnxn->connected == TRUE);
  DEBUG_PRINT("mca_network_wait_for_cmd(): **** start **** %d\n", retsize);

  cnxn->cmd = 0; cnxn->cmd_data_len = 0;
  retsize = net_read(cnxn, (void *)&cmd_n, sizeof(cmd_n));
  DEBUG_PRINT("mca_network_wait_for_cmd(): server command: %08X\n", ntohl(cmd_n));
  if (retsize <= 0){ 
    //Unable to read the command
    DEBUG_PRINT("mca_network_wait_for_cmd(): Unable to read command: [%d]\n", retsize);
    return 0;
  }
  //the command was read
  cmd = ntohl(cmd_n);
  cnxn->cmd = cmd;

  //now read the size of command payload ( data );
  retsize = net_read(cnxn, (void *)&cmd_data_size_n, sizeof(cmd_data_size_n));
  if (retsize <= 0){
    DEBUG_PRINT("mca_network_wait_for_cmd(): command: %08X, BUT error reading data size\n", cmd);
    return 0;
  }
  cmd_data_size = ntohl(cmd_data_size_n);
  cnxn->cmd_data_len = cmd_data_size;
  DEBUG_PRINT("mca_network_wait_for_cmd(): server command [%08X] data size [%d bytes]\n", cmd, cmd_data_size);

  if (!cmd_data_size){
    DEBUG_PRINT("mca_network_wait_for_cmd(): command: %08X BUT size is [%d bytes]\n", cmd, cmd_data_size);
    return cmd;
  }
  //now read the command's payload ( data )
  if (cnxn->cmd_data_len > MAX_RCV_BUF_SIZE)
    cnxn->cmd_data_len = MAX_RCV_BUF_SIZE;
  retsize = net_read(cnxn, cnxn->cmd_data, cnxn->cmd_data_len);
  if (retsize>=0)
    DEBUG_PRINT("mca_network_wait_cmd(): command data [%d bytes]\n", retsize);
  else {
    DEBUG_PRINT("mca_network_wait_cmd(): reading command data failed: %d\n", retsize);
  }

  return cmd;
}  

/*
  mca_network_send_cmd(MCACnxn *cnxn)

  send command and its data to mkahawa server
  
  Return > 0 if success. Else return 0
 */
gint32
mca_network_send_cmd (MCACnxn *cnxn, guint32 cmd, void *data, gint32 datalen)
{
  gint32       retsize;
  guint32      cmd_n, datalen_n;
 
  DEBUG_PRINT("mca_network_send_cmd(): sending  = %08X\n", cmd);
  assert(cnxn->connected == TRUE);

  cmd_n = htonl(cmd);
  retsize = net_write(cnxn, (void *)&cmd_n, (guint32) sizeof(cmd_n));
  datalen_n = htonl(datalen);
  retsize = net_write(cnxn, (void *)&datalen_n, sizeof(datalen_n));
  if (retsize<=0){
    DEBUG_PRINT("mca_network_send_cmd(): send error: retsize = %d\n", retsize);
    cnxn->connected = FALSE;
  }
  if (datalen > 0){
    retsize = net_write(cnxn, (void *)&data, datalen);
    DEBUG_PRINT("mca_network_send_cmd(): command data size  [%d]\n", datalen);
    if (retsize <= 0){
      cnxn->connected = TRUE;
      DEBUG_PRINT("mca_network_send_cmd(): unable to send data: retsize = %d\n", retsize);
    }
  }
  
  return 0;
}

/*
  mca_network_getstatus (*cnxn)

  Returns status of connection to mkahawa server. 
  For the moment: 1 for connected, 0 for not connected
 */
cnxn_status_t
mca_network_get_status(MCACnxn *cnxn)
{
  cnxn_status_t status;

  status = (cnxn_status_t)(cnxn->connected? 1 : 0);
  
  return status;
}



/**



Network Utils 


 **/
#include "mca_network_private.h"


static gint32
mca_connect_server (MCACnxn *cnxn)
{
  gint32           retval = 1;
  int             *cnxn_error = NULL; 

	cnxn->conn_priv = NULL;
	cnxn->conn_priv = (gpointer)connect_to_server(cnxn->server_name, cnxn->port,
																			cnxn->client_name, cnxn_error);

	if (cnxn->conn_priv == NULL)
		return 0;

	cnxn->connected = TRUE;
  return retval;
}

/*
  gint32 mca_network_send_signature (MCACnxn *cnxn)

  Send Client's name to the server
 */ 
static gint32
send_signature (MCACnxn *cnxn)
{
  gint32          retsize;
  guint32         datalen_n;
 
  assert(cnxn->connected == TRUE);
  DEBUG_PRINT("mca_network_send_signature(): %s\n", cnxn->client_name);

  datalen_n = htonl(strlen (cnxn->client_name));
  retsize = net_write(cnxn, (void *)&datalen_n, (guint32) sizeof(datalen_n)); 
  retsize = net_write(cnxn, (void *)cnxn->client_name, (guint32)strlen(cnxn->client_name));
  if (retsize < 0){
    DEBUG_PRINT("mca_send_signature(): Error sending signature: %d", retsize);
  }
  
  return retsize;
}

/*
  net_read ()
  
  Attempts to read datalen bytes from the network connection

  Returns number of bytes read or <=0 if error is encountered.
 */
static gint32
net_read (MCACnxn *cnxn, void *data, guint32 datalen)
{
  gint32     retsize;

	retsize = read_data(cnxn->conn_priv, data, datalen);

#if 0
  GError    *error;
  assert(cnxn != NULL);
#ifdef USE_IO_STREAM
  assert(cnxn->net_in_stream != NULL);

  retsize = g_input_stream_read(cnxn->net_in_stream, data, datalen, NULL, &error);
#else
  assert(cnxn->gsocket != NULL);

	retsize = g_socket_receive(cnxn->gsocket, data, datalen, NULL, &error);
#endif
  if (retsize < 0){
    DEBUG_PRINT("net_read(): Error [%s]\n", error->message);
    g_error_free(error);
  }
  else{
    DEBUG_PRINT("net_read(): read [%d bytes]\n", retsize);
  }  
#endif  
  return retsize;
}

/*
  net_write()

  Writes data (datalen bytes) to the connection
  Returns number of written bytes, or <=0 if an error is encountered
  The error is passed to DEBUG_PRINT
 */
static gint32
net_write (MCACnxn *cnxn, void *data, guint32 datalen)
{
  gint32     retsize;


	retsize = write_data (cnxn->conn_priv, data, datalen);

#if 0
  GError    *error;

  assert(cnxn != NULL);
#ifdef USE_IO_STREAM
  assert(cnxn->net_out_stream != NULL);

  retsize = g_output_stream_write(cnxn->net_out_stream, data, datalen, NULL, &error);
#else
  assert(cnxn->gsocket != NULL);

  retsize = g_socket_send(cnxn->gsocket, data, datalen, NULL, &error);
#endif
  if (retsize < 0){
    DEBUG_PRINT("net_write(): failed: %s\n", error->message);
    g_error_free(error);
  }
  else{
    DEBUG_PRINT("net_write(): wrote [%d bytes]\n", retsize);
  }  
#endif  
  return retsize;
}
