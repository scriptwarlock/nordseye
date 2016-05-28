#ifndef CCLC_PRIVATE_H
#define CCLC_PRIVATE_H

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

#include <openssl/ssl.h>
#include <openssl/err.h>

#ifndef WIN32
#define ERRNO		errno
#define H_ERRNO		h_errno
#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1
#define DATA(x)		(char*)(x)
#define TRYAGAIN	EAGAIN
#else
#define ERRNO		WSAGetLastError()
#define H_ERRNO		WSAGetLastError()
#define DATA(x)		(char*)(x)
#define TRYAGAIN	WSAEWOULDBLOCK
typedef int socklen_t;
#endif

#ifndef FALSE
#define FALSE		(0)
#endif

#ifndef TRUE
#define TRUE		(!FALSE)
#endif

struct _CCLC
{
  int		     sockfd;
  on_event_cb	     on_event;
  void		    *on_event_data;
  on_disconnect_cb   on_disconnect;
  void		    *on_disconnect_data;
  SSL_CTX	    *ssl_ctx;
  BIO		    *bio;
  char		    *certpass;
};
typedef struct _CCLC CCLC;

int _recvall(BIO * bio, void * buf, int len);
int _sendall(BIO * bio, const void * buf, int len);

#endif
