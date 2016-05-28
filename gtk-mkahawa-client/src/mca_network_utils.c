#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "mca_network.h"
#include "mca_network_private.h"

#define MCA_ERROR_SSL_HANDSHAKE  17
#define MCA_ERROR_NO_ERROR       0
#define MCA_ERROR_BAD_PASSWORD   18
#define MCA_ERROR_COULD_NOT_LOAD_VALUE   19


/* Static functions */
static SSL_CTX * _initialize_ssl_ctx(const char * cafile,
				     const char * keyfile,
				     const char * keypass, int * err);
SSL_CTX  *ssl_ctx = NULL;
char     *cert_passwd = NULL;

/**
 * Initialize SSL support.
 * error can be NULL if you don't care about it.
 */
int
init_ssl (const char *cafile, const char *certfile,
	  const char *certpass, int with_ssl, int *error)
{
  SSL_library_init();
  SSL_load_error_strings();

  if (!with_ssl)
    return 1;
  if (ssl_ctx)
    return 1;
  if (!(ssl_ctx = _initialize_ssl_ctx(cafile, certfile, certpass, error)))
    return 0;

  return 1;
}

void
close_ssl (void)
{
  
  if (ssl_ctx) 
    SSL_CTX_free(ssl_ctx);
  if (cert_passwd) 
    free(cert_passwd);

  ssl_ctx = NULL;
  cert_passwd = NULL;
}


/**
 * Connects to server
 *
 * Returns BIO (or data structur
 */
BIO*
connect_to_server (const char *svr_name, unsigned int svr_port,
		  const char *client_name, int *error)
{
  char  hostport[512];
  int   slen;
  BIO  *bio = NULL; 

  snprintf(hostport, sizeof(hostport)/sizeof(char), "%s:%u", 
	   svr_name, svr_port);
  slen = strlen(hostport);
 
  if (ssl_ctx){
    bio = BIO_new_ssl_connect(ssl_ctx);
    BIO_set_conn_hostname(bio, hostport);
  }
  else
    bio = BIO_new_connect(hostport);

  if (BIO_do_handshake(bio) != 1) {
    if (error) 
      *error = MCA_ERROR_SSL_HANDSHAKE;
    close_ssl();

    return NULL;
  }

  return bio;
}

/**
 * Closes the connection - stopping networking.
 *
 * @return INVALID_SOCKET (which can be set to sock_fd
 */
int
close_connection (void *conn_data)
{
  BIO *bio = (BIO *)conn_data;

  if (bio != NULL){
    BIO_free(bio);
  }

  return INVALID_SOCKET;
}

gint32
write_data (void *conn_data, void *data, guint32 datalen)
{
  BIO *bio = (BIO *)conn_data;

  return BIO_write(bio, data, datalen);
}


gint32
read_data (void *conn_data, void *data, guint32 datalen)
{
  BIO *bio = (BIO *)conn_data;

  return BIO_read(bio, data, datalen);
}


/* Static functions */

static int
_cert_verify(int ok, X509_STORE_CTX *x509_ctx)
{
  return 1;
}

static int
_pem_passwd_cb(char *buf, int passwd_len, int rw_flag, void *passwd)
{
  if (!passwd) 
    passwd = "_nopassword";

  strncpy(buf, (char *) passwd, passwd_len);
  buf[passwd_len - 1] = '\0';

  return strlen(buf);
}

static SSL_CTX *
_initialize_ssl_ctx(const char *ca_file, const char *key_file,
		    const char *new_cert_passwd, int *err)
{
  SSL_CTX *ctx;

  if (err) *err = MCA_ERROR_NO_ERROR;

  if (cert_passwd) 
    free(cert_passwd);
  cert_passwd = (new_cert_passwd ? strdup(new_cert_passwd) : NULL);

  /* Create our context*/
  ctx = SSL_CTX_new(SSLv23_client_method());
  SSL_CTX_set_default_passwd_cb(ctx, _pem_passwd_cb);
  SSL_CTX_set_default_passwd_cb_userdata(ctx, cert_passwd);

  /* Load our keys and certificates*/
  SSL_CTX_use_certificate_file(ctx, key_file, SSL_FILETYPE_PEM);
  
  if (1 != SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM))
    {
      if (err) *err = MCA_ERROR_BAD_PASSWORD;
      goto error;
    }
 
  if (1 != SSL_CTX_check_private_key(ctx))
    { 
      if (err) *err = MCA_ERROR_BAD_PASSWORD;
      goto error;
    }
  
  if (1 !=SSL_CTX_load_verify_locations(ctx, ca_file, NULL))
    {
      if (err) *err = MCA_ERROR_COULD_NOT_LOAD_VALUE;
      goto error;
    }
  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, _cert_verify);
  
  return ctx;

error:
  SSL_CTX_free(ctx);

  return NULL;
}
