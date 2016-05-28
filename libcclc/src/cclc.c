#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "cclc.h"
#include "cclc_private.h"

/* Static functions */
static SSL_CTX * _initialize_ssl_ctx(const char * cafile,
				     const char * keyfile,
				     const char * keypass, int * err);

CCLC *cclc = NULL;

/* Public interface */

/**
 * Initializes CCLC.
 *
 * @return  FALSE on failure, TRUE if initialized correctly.
 *
 * You need to call this before doing anything else with CCLC.
 */
int
CCLC_init(void)
{
  if (cclc)
    return TRUE;
  if (NULL == (cclc = calloc(1, sizeof(CCLC))))
    return FALSE;
  cclc->sockfd = INVALID_SOCKET;

  SSL_library_init();
  SSL_load_error_strings();
 
  return TRUE;
}

/**
 * Shutdowns CCLC.
 *
 * This functions frees all memory allocated by CCLC.
 *
 * @return FALSE
 *
 * Call this after you are no longer going to use CCLC.
 */
int
CCLC_shutdown(void)
{
  if (cclc)
    {
      CCLC_networking_shutdown();
      if (cclc->ssl_ctx) SSL_CTX_free(cclc->ssl_ctx);
      if (cclc->certpass) free(cclc->certpass);
      free(cclc);
      cclc = NULL;
    }

  return FALSE;
}

/**
 * Sets the callback to be used when a message from the server is received.
 *
 * @param   callback The function to be called when an event occurs.
 * @param   userdata The data to be passed as an argument to the callback.
 */
void
CCLC_set_on_event_callback(on_event_cb callback, void * userdata)
{
  cclc->on_event = callback;
  cclc->on_event_data = userdata;
}

/**
 * Sets the callback to be used when the client disconnects from the server.
 *
 * @param   callback The function to be called when a client disconnects.
 * @param   userdata The data to be passed as an argument to the callback.
 */
void
CCLC_set_on_disconnect_callback(on_disconnect_cb callback, void * userdata)
{
  cclc->on_disconnect = callback;
  cclc->on_disconnect_data = userdata;
}

/**
 * Initialize SSL support.
 *
 * @param   cafile CA cert filename.
 * @param   certfile Certificate filename.
 * @param   certpass Certificate password.
 * @param[out] error The error code will be stored here.
 * @return  FALSE on failure, TRUE otherwise.
 *
 * error can be NULL if you don't care about it.
 * Call this before you initialize CCLC networking if you want SSL support.
 */
int
CCLC_SSL_init(const char * cafile, const char * certfile,
	      const char * certpass, int * error)
{
  if (!cclc->ssl_ctx)
    if (!(cclc->ssl_ctx = _initialize_ssl_ctx(cafile, certfile, certpass,
					      error)))
      return FALSE;

  return TRUE;
}

/**
 * Initialize networking.
 *
 * @param   sname The server's hostname or ip.
 * @param   sport The port where the server is listening for new connections.
 * @param   myname The name to indentify this client.
 * @param[out] error The error code will be stored here.
 * @return  FALSE on failure, TRUE otherwise.
 *
 * error can be NULL if you don't care about it.
 * Call this to establish a connection with the server.
 */
int
CCLC_networking_init(const char * sname, unsigned short sport,
		     const char * myname, int * error)
{
  char hoststr[1024];
  int slen;

  if (cclc->bio) /* Already initialized */
    return TRUE;

  snprintf(hoststr, sizeof(hoststr)/sizeof(char), "%s:%u", sname, sport);
  slen = strlen(hoststr);
 
  if (cclc->ssl_ctx)
    {
      cclc->bio = BIO_new_ssl_connect(cclc->ssl_ctx);
      BIO_set_conn_hostname(cclc->bio, hoststr);
    }
  else
    cclc->bio = BIO_new_connect(hoststr);

  if (1 == BIO_do_handshake(cclc->bio))
    {
      cclc->sockfd = BIO_get_fd(cclc->bio, NULL);
      BIO_write(cclc->bio, &slen, sizeof(slen));
      BIO_write(cclc->bio, myname, slen);
      return TRUE;
    }
  else
    {
      if (error) *error = CCLC_ERROR_CONNECT_FAIL;
      CCLC_networking_shutdown();
      return FALSE;
    }
}

/**
 * Shutdowns networking.
 *
 * @return TRUE
 *
 * Call this when you no longer need networking support.
 */
int
CCLC_networking_shutdown(void)
{
  if (NULL != cclc->bio)
    {
      BIO_free(cclc->bio);
      cclc->bio = NULL;
    }

  cclc->sockfd = INVALID_SOCKET;

  return TRUE;
}

/**
 * Check for incoming messages from the server, or disconnections.
 *
 * @return FALSE if no events where procesed, TRUE otherwise.
 *
 * When an event occurs, the respective callback is going to be called.
 * This should be called repeatedly in your program if it supports networking.
 */
int
CCLC_check_events(void)
{
  struct timeval delta;
  int nfds;
  int fd = cclc->sockfd;
  fd_set readfds;

  if (INVALID_SOCKET == fd)
    return FALSE;

  FD_SET(fd, &readfds);
  
  delta.tv_usec = 0;
  delta.tv_sec = 0;

  nfds = select(fd + 1, &readfds, NULL, NULL, &delta);

  if (nfds <= 0)
    return FALSE;
  
  if (FD_ISSET(fd, &readfds))
    {
      int connection_closed = FALSE;
      unsigned cmd = 0;
      unsigned size = 0;
      void *data = NULL;
      int bytes = 0;
      
      bytes = _recvall(cclc->bio, &cmd, sizeof(cmd));
      if (0 == bytes || bytes != sizeof(cmd))
	connection_closed = TRUE;
     
      if (!connection_closed)
	{
	  bytes = _recvall(cclc->bio, &size, sizeof(size));
	  if (0 >= bytes || bytes != sizeof(size))
	    connection_closed = TRUE;
	}
     
      cmd = CCLC_ntohl(cmd);
      size = CCLC_ntohl(size);

      if (!connection_closed && 0 < cmd && 0 < size)
	{
	  data = calloc(1, size);
	  bytes = _recvall(cclc->bio, data, size);
	  if (0 == bytes || bytes != size)
	    connection_closed = TRUE;
	}
      if (cclc->on_event)
	cclc->on_event(cmd, data, size, cclc->on_event_data);
      if (data)
	free(data);
      /* If the connection broke, or was closed */
      if (connection_closed)
	{
	  CCLC_networking_shutdown();
	  if (cclc->on_disconnect)
	    cclc->on_disconnect(cclc->on_disconnect_data);
	}
    }

  return TRUE;
}

/**
 * Send a command to the server (only if networking is enabled).
 *
 * @param   cmd The command.
 * @param   data A pointer to the data sent with command.
 * @param   datasize The size of the data.
 */
void
CCLC_send_cmd(unsigned cmd, const void *data, unsigned datasize)
{
  if (INVALID_SOCKET != cclc->sockfd)
    {
      unsigned ncmd = CCLC_htonl(cmd);
      unsigned ndatasize = CCLC_htonl(datasize);
      
      _sendall(cclc->bio, &ncmd, sizeof(ncmd));
      _sendall(cclc->bio, &ndatasize, sizeof(ndatasize));
      if (data && datasize)
	_sendall(cclc->bio, data, datasize);
    }
}

/* Static functions */

static int
_cert_verify(int ok, X509_STORE_CTX * x509_ctx)
{
  return 1;
}

static int
_passwd_cb(char * buf, int num, int rw, void * password)
{
  if (!password) password = "_nopassword";

  strncpy(buf, (char *) password, num);
  buf[num - 1] = '\0';

  return strlen(buf);
}

static SSL_CTX *
_initialize_ssl_ctx(const char * cafile, const char * keyfile,
		    const char * certpass, int * err)
{
  SSL_CTX *ctx;

  if (err) *err = CCLC_ERROR_NO_ERROR;

  if (cclc->certpass) free(cclc->certpass);
  cclc->certpass = (certpass ? strdup(certpass) : NULL);

  /* Create our context*/
  ctx = SSL_CTX_new(SSLv23_client_method());
  SSL_CTX_set_default_passwd_cb(ctx, _passwd_cb);
  SSL_CTX_set_default_passwd_cb_userdata(ctx, cclc->certpass);

  /* Load our keys and certificates*/
  SSL_CTX_use_certificate_file(ctx, keyfile, SSL_FILETYPE_PEM);
  
  if (1 != SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM))
    {
      if (err) *err = CCLC_ERROR_BAD_PASSWORD;
      goto error;
    }
 
  if (1 != SSL_CTX_check_private_key(ctx))
    { 
      if (err) *err = CCLC_ERROR_BAD_PASSWORD;
      goto error;
    }
  
  if (1 !=SSL_CTX_load_verify_locations(ctx, cafile, NULL))
    {
      if (err) *err = CCLC_ERROR_COULD_NOT_LOAD_VL;
      goto error;
    }
  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, _cert_verify);
  
  return ctx;

error:
  SSL_CTX_free(ctx);

  return NULL;
}
