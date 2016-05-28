#ifndef MCA_NETWORK_PRIVATE_H
#define MCA_NETWORK_PRIVATE_H

#include <openssl/ssl.h>
#include <openssl/err.h>


#ifndef WIN32
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#else
#include <windows.h>
#include <winuser.h>
#include <winsock2.h>
#endif

#ifndef WIN32
#define ERRNO		        errno
#define H_ERRNO		      h_errno
#define INVALID_SOCKET	-1
#define SOCKET_ERROR	  -1
#define DATA(x)		      (char*)(x)
#define TRYAGAIN	      EAGAIN
#else
#define ERRNO		        WSAGetLastError()
#define H_ERRNO		      WSAGetLastError()
#define DATA(x)		      (char*)(x)
#define TRYAGAIN	      WSAEWOULDBLOCK
typedef int socklen_t;

#endif


int    init_ssl (const char *cafile, const char *certfile,
		 const char *certpass, int with_ssl, int *error);
void   close_ssl (void);
BIO*   connect_to_server (const char *svr_name, unsigned int svr_port,
			  const char *client_name, int *error);
int    close_connection (void *conn_priv);
int    write_data (void *conn_priv, void *data, guint32 datalen);
int    read_data  (void *conn_data, void *data, guint32 datalen);




#endif //MCA_NETWORK_PRIVATE_H
