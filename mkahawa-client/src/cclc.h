#ifndef CCLC_H
#define CCLC_H


#ifdef WIN32

#ifdef BUILD_DLL
     // the dll exports
     #define EXPORT __declspec(dllexport)
#else
     // the exe imports
     #define EXPORT extern cppfudge  __declspec(dllimport)
#endif

#else 

#define EXPORT

#endif



/******************** Callback Types **********************/
/**
 * Callback used when a message from a server is received
 *
 * @param   cmd The command.
 * @param   data Data associated with the command.
 * @param   datasize The size of the data.
 * @param   userdata User supplied data.
 */
typedef void (*on_event_cb) (unsigned cmd, void * data, unsigned datasize,
			     void * userdata);

/**
 * Callback used when a client disconnects from the server.
 *
 * @param   userdata User supplied data.
 */
typedef void (*on_disconnect_cb) (void * userdata);

/******************** Error codes *************************/
#define CCLC_ERROR_NO_ERROR		0 /** No error ocurred */
#define CCLC_ERROR_CONNECT_FAIL		1 /** Error when connecting */
#define CCLC_ERROR_BAD_PASSWORD		2 /** Bad password */
#define CCLC_ERROR_COULD_NOT_LOAD_VL	3 /** Couldn't load verify locations */

/******************* MD5 Digest Length ********************/
#define CCLC_MD5_DIGEST_LENGTH		16 /** MD5 digest length (in bytes) */

/**********************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
EXPORT int	      CCLC_init(void);
EXPORT int	      CCLC_shutdown(void);
EXPORT void	      CCLC_free(void * mem);
EXPORT unsigned char *
	      CCLC_MD5(const unsigned char * d, unsigned long n,
		       unsigned char * md);
EXPORT unsigned      CCLC_htonl(unsigned val);
EXPORT unsigned short
	      CCLC_htons(unsigned short val);
EXPORT unsigned      CCLC_ntohl(unsigned val);
EXPORT unsigned short
	      CCLC_ntohs(unsigned short val);
EXPORT void	      CCLC_set_on_event_callback(on_event_cb callback,
					 void * userdata);
EXPORT void	      CCLC_set_on_disconnect_callback(on_disconnect_cb callback,
					      void * userdata);
EXPORT int	      CCLC_SSL_init(const char * cafile, const char * certfile,
			    const char * certpass, int * error);
EXPORT int	      CCLC_networking_init(const char * server, unsigned short port,
				   const char * myname, int * error);
EXPORT int	      CCLC_networking_shutdown(void);
EXPORT int	      CCLC_check_events(void);
EXPORT void	      CCLC_send_cmd(unsigned cmd, const void * data,
			    unsigned datasize);
#ifdef __cplusplus
}
#endif

#endif
