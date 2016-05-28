#include "cclc.h"
#include "cclc_private.h"

/* Private interface */

int
_sendall(BIO * bio, const void * buf, int len)
{
  return BIO_write(bio, buf, len);
}

int
_recvall(BIO * bio, void * buf, int len)
{
  return BIO_read(bio, buf, len);
}

/* Public interface */

/**
 * Calculates the MD5 digest of the n bytes contained at d.
 *
 * @param   d The bytes.
 * @param   n The number of bytes.
 * @param[out] md The place to store the digest.
 * @return A pointer to the digest (md, unless md was NULL).
 *
 * md must be a pointer to a memory block of size CCL_MD5_DIGEST_LENGTH
 */
unsigned char *
CCLC_MD5(const unsigned char * d, unsigned long n, unsigned char * md)
{
  return (unsigned char *) MD5(d, n, md);
}

/**
 * Frees data allocated by CCL.
 *
 * @param   mem A pointer the data allocated by CCL that must be freed.
 */
void
CCLC_free(void * mem)
{
  free(mem);
}

/**
 * Converts an integer from host to network byte order.
 *
 * @param   val The integer.
 * @return The integer, in network byte order.
 */
unsigned int
CCLC_htonl(unsigned int val)
{
  return htonl(val);
}

/**
 * Converts a short integer from host to network byte order.
 *
 * @param   val The integer.
 * @return The integer, in network byte order.
 */
unsigned short
CCLC_htons(unsigned short val)
{
  return htons(val);
}

/**
 * Converts an integer from network to host byte order.
 *
 * @param   val The integer.
 * @return The integer, in network byte order.
 */
unsigned
CCLC_ntohl(unsigned val)
{
  return ntohl(val);
}

/**
 * Converts a short integer from network to host byte order.
 *
 * @param   val The integer.
 * @return The integer, in network byte order.
 */
unsigned short
CCLC_ntohs(unsigned short val)
{
  return ntohs(val);
}
