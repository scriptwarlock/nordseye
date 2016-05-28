#include "ccls.h"
#include "ccl_private.h"

/* Private functions */

gint
_sendall(BIO * bio, const void * buf, gint len)
{
  return BIO_write(bio, buf, len);
}

gint
_recvall(BIO * bio, void * buf, gint len)
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
guchar *
CCL_MD5(const guchar * d, gulong n, guchar * md)
{
  return (guchar *)MD5(d, n, md);
}

/**
 * Frees data allocated by CCL.
 *
 * @param   mem A pointer the data allocated by CCL that must be freed.
 */
void
CCL_free(const void * mem)
{
  g_free(mem);
}

/**
 * Converts an integer from host to network byte order.
 *
 * @param   val The integer.
 * @return The integer, in network byte order.
 */
guint
CCL_htonl(guint val)
{
  return htonl(val);
}

/**
 * Converts a short integer from host to network byte order.
 *
 * @param   val The integer.
 * @return The integer, in network byte order.
 */
gushort
CCL_htons(gushort val)
{
  return htons(val);
}

/**
 * Converts an integer from network to host byte order.
 *
 * @param   val The integer.
 * @return The integer, in network byte order.
 */
guint
CCL_ntohl(guint val)
{
  return ntohl(val);
}

/**
 * Converts a short integer from network to host byte order.
 *
 * @param   val The integer.
 * @return The integer, in network byte order.
 */
gushort
CCL_ntohs(gushort val)
{
  return ntohs(val);
}
