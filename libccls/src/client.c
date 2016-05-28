#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ccls.h"
#include "ccl_private.h"

enum ROUNDOFF { RND_00=1, RND_05=2, RND_50=3, RND_01=4, RND_10=5, RND_25=6 };

/*
#define DEBUG_LIBCCL 1
#define DEBUG 1
*/
extern CCL *ccl;

/* Static functions */
static gint _FindProductByIdFunc(gconstpointer a, gconstpointer b);
static void _FindGreatestFdFunc(GQuark key_id, gpointer data,
				gpointer user_data);
static gboolean _CCL_client_store(gint client);

/* Public interface */

/**
 * Add a new client to the list.
 *
 * @param   name The name of the new client.
 * @return The id of the new client.
 *
 * If the client already exists, this will return the id of the existing
 * client.
 */
gint
CCL_client_new(const gchar * name)
{
  gchar *cmd = NULL;
  gint id;

  id = CCL_client_find(name);

  if (-1 == id)
    {
      CCL_client *client = g_new0(CCL_client, 1);

      _CCL_client_init(client, name);
      cmd = sqlite3_mprintf("insert into clients\n"
			    "(name, status, intervals, products,\n"
			    "timeout, member, flags)\n"
			    "values(%Q, 0, null, null, 0, 0, 0);", name);
      sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
      sqlite3_free(cmd);
      id = sqlite3_last_insert_rowid(ccl->db);
      g_datalist_id_set_data_full(&(ccl->clients), id, client, _destroy_client);
    }

  return id;
}

/**
 * Checks if a client already exists.
 *
 * @param   client The client's id.
 * @return TRUE if the client exists, FALSE if not.
 */
gboolean
CCL_client_exists(gint client)
{
  if (g_datalist_id_get_data(&(ccl->clients), client))
    return TRUE;
  else
    return FALSE;
}

/**
 * Find a client by name.
 *
 * @param   name The name of the client we are looking for.
 * @return The client's id if found, -1 if not found.
 */
gint
CCL_client_find(const gchar * name)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint id = -1;
 
  cmd = sqlite3_mprintf("select id from clients\n"
			"where lower(name) = lower(%Q);", name);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    {
      id = sqlite3_column_int(stmt, 0);
      
      if (!g_datalist_id_get_data(&(ccl->clients), id)) /* If not in memory */
	{
	  CCL_client *client = g_new0(CCL_client, 1);

	  _CCL_client_init(client, name);
	  g_datalist_id_set_data_full(&(ccl->clients), id, client,
				      _destroy_client);
	  _CCL_client_restore(id);
	}
    }

  sqlite3_finalize(stmt);

  return id;
}

/**
 * Gets the nth client on the list.
 *
 * @param   nth The index of the client to get (starting at 0).
 * @return The client's id, or -1 if there aren't more clients.
 *
 * Use this to iterate on the clients, for example when building a list of
 * all the clients.
 */
gint
CCL_client_get_nth(guint nth)
{
  gint id = 1;
  gint num = -1;

  while (id < 256)
    {
      if (g_datalist_id_get_data(&(ccl->clients), id))
	num++;

      if (num == nth)
	return id;

      id++;
    }

  return -1;
}

/**
 * Resets the state of a client.
 *
 * @param   client The client's id.
 *
 * Use this if you need to clear the status of a client.
 * You are probably not going to use this.
 */
void
CCL_client_reset(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  GSList *link = NULL;

  g_return_if_fail(c);

  while (c->intervals->len)
    g_free(g_ptr_array_remove_index_fast(c->intervals, 0));
  c->timeout = 0;
  c->status = CCL_INACTIVE;
  while ((link = g_slist_last(c->products)) &&
	 (c->products = g_slist_remove_link(c->products, link)))
    {
      g_free(link->data);
      g_slist_free_1(link);
    }
  c->products = NULL;
  c->member = 0;
  _CCL_client_store(client);
}

/**
 * Gets the name of a client.
 *
 * @param   client The client's id.
 * @return The name of the client, or NULL if that client doesn't exist.
 */
const gchar *
CCL_client_name_get(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);

  g_return_val_if_fail(c, NULL);
    
  return (const gchar *)c->name;
}

/**
 * Gets the status of a client.
 *
 * @param   client The client's id.
 * @return The status of the client (one of CCL_ACTIVE, CCL_PAUSED, etc).
 */
gint
CCL_client_status_get(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);

  g_return_val_if_fail(c, -1);

  return c->status;
}

/**
 * Starts a new session, or continues a paused one, on a client.
 *
 * @param   client The client's id.
 *
 * This only works if the client state is CCL_INACTIVE or CCL_PAUSED.
 * If it is CCL_INACTIVE, this starts a new session, reseting the client
 * (clear products lists, intervals, etc), and counting from 0.
 * If it is CCL_PAUSED, this continues the current session, adding a new
 * interval (that means that the time the client was CCL_PAUSED is not
 * added to the total). This sets the status to CCL_ACTIVE.
 */
void
CCL_client_start(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  CCL_interval *interval = NULL;

  g_return_if_fail(c);
  if(!(c->status == CCL_INACTIVE || c->status == CCL_PAUSED)) return;

#ifdef DEBUG_LIBCCL
  (c->status == CCL_INACTIVE)? printf("was INACTIVE\n"): printf("was PAUSED\n");
#endif
  interval = g_new0(CCL_interval, 1);
  if (c->status == CCL_INACTIVE)
    CCL_client_reset(client);

  c->status = CCL_ACTIVE;
  interval->stime = time(NULL);
  g_ptr_array_add(c->intervals, interval);
  _CCL_client_store(client);
}

/**
 * Ends the current session of a client.
 *
 * @param   client The client id.
 *
 * When this is called, CCL stops counting the time for this session, and
 * the status is set to CCL_INACTIVE.
 */
void
CCL_client_stop(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);

  g_return_if_fail(c);
  
  if (c->status == CCL_ACTIVE)
    {
      gint i = c->intervals->len - 1;
      
      g_return_if_fail(0 <= i);
      
      c->status = CCL_INACTIVE;

      ((CCL_interval *)
       g_ptr_array_index(c->intervals, i))->etime = time(NULL);
      _CCL_client_store(client);
    }
}

/**
 * Continues a session on an CCL_INACTIVE client, as if you never stopped it.
 *
 * @param   client The client's id.
 *
 * Think of this like a "undo stop", it restores the state, and removes
 * the entry from the log, so everything is left as if you never stopped it.
 * Only works if state is CCL_INACTIVE. State is set to CCL_ACTIVE.
 */
void
CCL_client_unstop(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  gint i;
  time_t stime;
  time_t etime;
  gint session;

  g_return_if_fail(c);
  if (!(c->status == CCL_INACTIVE)) return;
  
  i = c->intervals->len - 1;
  
  g_return_if_fail(0 <= i);

  CCL_client_interval_get_nth(client, 0, &stime, NULL);
  CCL_client_interval_get_nth(client, i, NULL, &etime);
  session = CCL_log_session_find(client, stime, etime);

  if (-1 != session)
    CCL_log_session_clear(session);
  
  if (CCL_client_time_used(client) > c->timeout)
    CCL_client_timeout_set(client, 0);

  c->status = CCL_ACTIVE;
  ((CCL_interval *) g_ptr_array_index(c->intervals, i))->etime = 0;
  _CCL_client_store(client);
}

/**
 * Pauses the session on a client.
 *
 * @param   client The client's id.
 *
 * When you pause a session, and then continue it, all the time it was paused
 * is not added to the total.
 * Only works if the state is CCL_ACTIVE. State is set to CCL_PAUSED.
 */
void
CCL_client_pause(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  gint i;
  CCL_interval *interval = NULL;

  g_return_if_fail(c);
  if (!(c->status == CCL_ACTIVE)) return;
  
  i = c->intervals->len - 1;

  g_return_if_fail(0 <= i);

  interval = (CCL_interval *) g_ptr_array_index(c->intervals, i);
  interval->etime = time(NULL);
  c->status = CCL_PAUSED;
  _CCL_client_store(client);
}

/**
 * Continues a session on an CCL_PAUSED client, as if you never paused it.
 *
 * @param   client The client's id.
 *
 * Think of this like a "undo pause", it restores the state, and the time
 * the client was paused is added to the total, so everything is left as if
 * you never paused it.
 * Only works if state is CCL_PAUSED. State is set to CCL_ACTIVE.
 */
void
CCL_client_unpause(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  gint i;
  CCL_interval *interval = NULL;

  g_return_if_fail(c);
  if (!(c->status == CCL_PAUSED)) return;
  
  i = c->intervals->len - 1;

  g_return_if_fail(0 <= i);

  interval = (CCL_interval *) g_ptr_array_index(c->intervals, i);
  interval->etime = 0;
  c->status = CCL_ACTIVE;
  _CCL_client_store(client);
}

/**
 * Swaps the status from one client to another.
 *
 * @param   lclient The id of one of the clients.
 * @param   rclient The id of the other client.
 *
 * This swaps member, satus, products, intervals, and timeout between
 * lclient and rclient, this is useful when a user moves from a PC to
 * another PC.
 */
void
CCL_client_swap(gint lclient, gint rclient)
{
  CCL_client *lc = g_datalist_id_get_data(&(ccl->clients), lclient);
  CCL_client *rc = g_datalist_id_get_data(&(ccl->clients), rclient);
  CCL_client *tc = NULL;
    
  g_return_if_fail(lc && rc);
  
  tc = g_new0(CCL_client, 1);

  tc->status = lc->status;
  tc->products = lc->products;
  tc->intervals = lc->intervals;
  tc->timeout = lc->timeout;
  tc->member = lc->member;

  lc->status = rc->status;
  lc->products = rc->products;
  lc->intervals = rc->intervals;
  lc->timeout = rc->timeout;
  lc->member = rc->member;

  rc->status = tc->status;
  rc->products = tc->products;
  rc->intervals = tc->intervals;
  rc->timeout = tc->timeout;
  rc->member = tc->member;

  _CCL_client_store(lclient);
  _CCL_client_store(rclient);

  g_free(tc);
}

/**
 * Adds a product to the lists of products of a client.
 *
 * @param   client The client's id.
 * @param   id The id of the product.
 * @param   amount How much of this product to add.
 */
void
CCL_client_product_add(gint client, gint id, guint amount)
{
  guint camount = CCL_client_product_amount_get(client, id);

  CCL_client_product_amount_set(client, id, camount + amount);
  if (CCL_product_stock_get(id) != CCL_DELETEDPRODUCT)
    CCL_product_stock_set(id, CCL_product_stock_get(id) - amount);
}

/**
 * Subtracts from the amount of a product of a client's products.
 *
 * @param   client The client's id.
 * @param   id The id of the product.
 * @param   amount How much of this product to subtract.
 */
void
CCL_client_product_sub(gint client, gint id, guint amount)
{
  guint camount = CCL_client_product_amount_get(client, id);

  if (amount >= camount)
    {
      CCL_client_product_amount_set(client, id, 0);
      if (CCL_product_stock_get(id) != CCL_DELETEDPRODUCT)
	CCL_product_stock_set(id, CCL_product_stock_get(id) + camount);
    }
  else
    {
      CCL_client_product_amount_set(client, id, camount - amount);
      if (CCL_product_stock_get(id) != CCL_DELETEDPRODUCT)
	CCL_product_stock_set(id, CCL_product_stock_get(id) + amount);
    }
}

/**
 * Gets how much of the specified product a client has.
 *
 * @param   client The client's id.
 * @param   id The id of the product.
 * @return How much of that product the client has.
 */
guint
CCL_client_product_amount_get(gint client, gint id)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  GSList *link = NULL;

  g_return_val_if_fail(c, 0);

  link = g_slist_find_custom(c->products, GINT_TO_POINTER(id),
			     _FindProductByIdFunc);
  if (link)
    return (((guint *) link->data)[1]);
  else
    return 0;
}

/**
 * Sets the amount of a product the client has.
 *
 * @param   client The client's id.
 * @param   id The id of the product.
 * @param   amount The amount to set.
 */
void
CCL_client_product_amount_set(gint client, gint id, guint amount)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  GSList *link = NULL;

  g_return_if_fail(c);
  
  link = g_slist_find_custom(c->products, GINT_TO_POINTER(id),
			     _FindProductByIdFunc);
  if (link)
    {
      if (amount > 0)
	((guint *) link->data)[1] = amount;
      else
	{
	  g_free(link->data);
	  c->products = g_slist_remove_link(c->products, link);
	  g_slist_free_1(link);
	}
    }
  else if (amount > 0)
    {
      guint *pair = g_malloc(sizeof(gint) + sizeof(guint));

      ((gint *) pair)[0] = id;
      ((gint *) pair)[1] = amount;
      c->products = g_slist_append(c->products, pair);
    }

  _CCL_client_store(client);
}

/**
 * Gets the nth product on the list of products on a client.
 *
 * @param   client The client's id.
 * @param   nth The index of the product to get (starting at 0).
 * @param[out] product The product id will be put here.
 * @param[out] amount The amount of that product will be put here.
 * @return FALSE if no more products, TRUE otherwise.
 *
 * Use this to iterate on the products, for example when building a list of
 * all the products.
 */
gboolean
CCL_client_product_get_nth(gint client, guint nth,
			   gint * product, guint * amount)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  gboolean found = FALSE;
  GSList *link = NULL;
  gint i;

  g_return_val_if_fail(c, FALSE);
      
  link = c->products;
  for (i = 0; i < nth; i++)
    link = g_slist_next(link);

  if (link)
    {
      found = TRUE;
      if (product)
	*product = ((gint *) link->data)[0];
      if (amount)
	*amount = ((guint *) link->data)[1];
    }

  return found;
}

/**
 * Sets the session timeout for a client.
 *
 * @param   client The client's id.
 * @param   timeout The timeout (in seconds).
 */
void
CCL_client_timeout_set(gint client, time_t timeout)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);

  g_return_if_fail(c);
  
  c->timeout = 0;
  if (timeout > 0)
    c->timeout = timeout;
  _CCL_client_store(client);
}

/**
 * Gets the timeout for a client.
 *
 * @param   client The client's id.
 * @return The timeout (in seconds).
 */
time_t
CCL_client_timeout_get(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);

  g_return_val_if_fail(c, -1);

  return c->timeout;
}

/**
 * Calculates the time the current session on a client has lasted.
 *
 * @param   client The client's id.
 * @return The total used time (in seconds).
 */
time_t
CCL_client_time_used(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  gint i;
  gint stime = 0;
  gint etime = 0;
  time_t ttime = 0;
  CCL_interval *interval = NULL;

  g_return_val_if_fail(c, -1);

  for (i = 0; i < c->intervals->len; i++)
    {
      interval = g_ptr_array_index(c->intervals, i);
      stime = interval->stime;
      etime = ((interval->etime > 0) ? interval->etime : time(NULL));
      ttime += etime - stime;
    }

  return ttime;
}

/**
 * Calculates the time the client has left to meet the timeout.
 *
 * @param   client The client's id.
 * @return The time left, 0 if no time left, or -2 if no timeout was set.
 *
 * The time is in seconds.
 */
time_t
CCL_client_time_left(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);

  g_return_val_if_fail(c, -1);
  
  if (!c->timeout)
    return -2;
  else
    return MAX((c->timeout - CCL_client_time_used(client)), 0);
}

/**
 * Gets the session start time for client.
 *
 * @param   client The client's id.
 * @return The start time.
 */
time_t
CCL_client_stime_get(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  time_t stime = 0;

  g_return_val_if_fail(c, -1);
      
  CCL_client_interval_get_nth(client, 0, &stime, NULL);
      
  return stime;
}

/**
 * Gets the session end time for client.
 *
 * @param   client The client's id.
 * @return The end time.
 */
time_t
CCL_client_etime_get(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  time_t etime = 0;
  gint nintervals;

  g_return_val_if_fail(c, -1);
  
  nintervals = c->intervals->len;
  CCL_client_interval_get_nth(client, nintervals - 1, NULL, &etime);
      
  return etime;
}

/**
 * Gets the number of intervals for the current session of client.
 *
 * @param   client The client's id.
 * @return The number of intervals.
 */
gint
CCL_client_intervals_get_num(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);

  g_return_val_if_fail(c, -1);
    
  return c->intervals->len;
}

/**
 * Gets the nth interval for the current session of client.
 *
 * @param   client The client's id.
 * @param   nth The index (starting from 0).
 * @param[out] stime The start time of the interval will be put here.
 * @param[out] etime The end time of the interval will be put here.
 * @return TRUE if the interval exists, FALSE otherwise.
 */
gboolean
CCL_client_interval_get_nth(gint client, guint nth,
			    time_t * stime, time_t * etime)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  CCL_interval *interval = NULL;

  g_return_val_if_fail(c, FALSE);
  if (!(c->intervals->len > nth)) return FALSE;

  interval = (CCL_interval *) g_ptr_array_index(c->intervals, nth);

  if (interval)
    {
      if (stime)
	*stime = interval->stime;
      if (etime)
	*etime = interval->etime;
      
      return TRUE;
    }
  else
    return FALSE;
}

/**
 * Sets client flags to the value of flags.
 *
 * @param   client The client's id.
 * @param   flags The new flags value.
 */
void
CCL_client_flags_set(gint client, gint flags)
{

  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);

  g_return_if_fail(c);
      
  c->flags = flags;
  _CCL_client_store(client);
}

/**
 * Gets the flags value of client.
 *
 * @param   client The client's id.
 * @return The flags value.
 */
gint
CCL_client_flags_get(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);

  g_return_val_if_fail(c, 0);

  return c->flags;
}

/**
 * Toggle flags on client on or off.
 *
 * @param   client The client's id.
 * @param   flags The flags to toggle.
 * @param   on If TRUE toggle the flags on, else toggle them off.
 */
void
CCL_client_flags_toggle(gint client, gint flags, gboolean on)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);

  g_return_if_fail(c);
  
  if (on)
    c->flags |= flags;
  else
    c->flags &= ~flags;

  _CCL_client_store(client);
}

/**
 * Sets client's associated data pointer.
 *
 * @param   client The client's id.
 * @param   data The pointer to the data.
 * @return The old data pointer.
 */
gpointer
CCL_client_data_set(gint client, gpointer data)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  gpointer olddata = NULL;
  
  g_return_val_if_fail(c, NULL);
      
  olddata = c->data;
  c->data = data;

  return olddata;
}

/**
 * Gets a pointer to the client's associated data.
 *
 * @param   client The client's id.
 * @return The pointer to the data.
 */
gpointer
CCL_client_data_get(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);

  g_return_val_if_fail(c, NULL);
  
  return c->data;
}

guint
CCL_round_cash(int cash)
{
  switch((enum ROUNDOFF)ccl->rndoff){
  case RND_00:
    if (cash % 100)
      cash += 100 - cash % 100;
    break;
  case RND_05:
    if (cash % 5)
      cash += 5 - cash % 5;
    break;
  case RND_10:
    if (cash % 10)
      cash += 10 - cash % 10;
    break;
  case RND_25:
    if (cash % 25)
      cash += 25 - cash % 25;
    break;
  case RND_50:
    if (cash % 50)
      cash += 50 - cash % 50;
    break;
  default:
    cash += 1;
    break;
  }

  return cash;
}

/**
 * Calculates how much is owed on the current session of client.
 *
 * @param   client The client's id.
 * @return How much is owed of workstation use.
 *
 * This calculates how much is owed by workstation use time.
 * If this client has a member set for this session, and with a tarif
 * different from the default, then that tarif will be used.
 */
guint
CCL_client_owed_terminal(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  gint stime = 0;
  gint etime = 0;
  gint i;
  gdouble frac = 0.0;
  guint cash = 0;
  time_t ttime;
  gboolean permin;
  gint oldtarif = -1;
  CCL_interval *interval = NULL;

  g_return_val_if_fail(c, 0);

  ttime = CCL_client_time_used(client);
  if (!ttime)  return 0;

  /* Set the member tarif */
  if (CCL_member_exists(c->member) && 0 != CCL_member_tarif_get(c->member))
    {
      oldtarif = CCL_tarif_get();
      CCL_tarif_set(CCL_member_tarif_get(c->member));
#ifdef DEBUG
  printf("CCL_client_owed_terminal: Member: %d, Tariff: %d\n",
	 c->member, CCL_member_tarif_get(c->member));
#endif
    }
  permin = (CCL_client_time_used(client) / 60 >= ccl->perminafter
	    && ccl->perminafter != -1);
#ifdef DEBUG
  printf("CCL_client_owed_terminal: perminafter: %d, permin: %d\n",
	 ccl->perminafter, permin);
#endif
  for (i = 0; i < c->intervals->len; i++)
    {
      interval = g_ptr_array_index(c->intervals, i);
      stime = interval->stime;
      etime = ((interval->etime > 0) ? interval->etime : time(NULL));
      if (!permin)
	frac = (etime - stime) / (gdouble) ttime;
      else
	frac = 1.0;
      cash += (gint) (CCL_tarif_calc(stime, etime, permin) * frac);
    }

  /* Restore previous tarif */ 
  if (1 <= oldtarif)
    CCL_tarif_set(oldtarif);

  return CCL_round_cash(cash);
}

/**
 * Calculates how much is owed on the current session of client.
 *
 * @param   client The client's id.
 * @return How much is owed of products.
 */
guint
CCL_client_owed_products(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  guint cash = 0;
  GSList *link = NULL;

  g_return_val_if_fail(c, 0);
  
  link = c->products;

  while (link)
    {
      guint price = 0;
      gint id = ((gint *) link->data)[0];

      if (CCL_product_info_get(id, NULL, NULL, &price))
	cash += price * CCL_client_product_amount_get(client, id);

      link = g_slist_next(link);
    }

  return cash;
}

/**
 * Sets the member for the current session of client.
 *
 * @param   client The client's id.
 * @param   member The member's id.
 */
void
CCL_client_member_set(gint client, gint member)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);

  g_return_if_fail(c);
      
  c->member = member;
  _CCL_client_store(client);
}

/**
 * Gets the member used by the current session of client.
 *
 * @param   client The client's id.
 * @return The member's id.
 */
gint
CCL_client_member_get(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);

  g_return_val_if_fail(c, -1);
      
  if (c->member)
    return c->member;
  else
    return 0;
}

/**
 * Retrieve client's ip address
 *
 * @param   ipstr The client connection's IP address.
 * @return  the IP address in long format - IPV4
 */
long
CCL_client_ip_get(int client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  
  g_return_val_if_fail(c, -1);
  
  return c->ipaddr;
}

/**
 * Send a command to client (only if networking is enabled).
 *
 * @param   client The client's id.
 * @param   cmd The command.
 * @param   data A pointer to the data sent with command.
 * @param   datasize The size of the data.
 */
void
CCL_client_send_cmd(gint client, guint cmd, const void *data, guint datasize)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);

  g_return_if_fail(c);

  if (INVALID_SOCKET != c->sockfd)
    {
      guint ncmd = CCL_htonl(cmd);
      guint ndatasize = CCL_htonl(datasize);
      
      _sendall(c->bio, &ncmd, sizeof(ncmd));
      _sendall(c->bio, &ndatasize, sizeof(ndatasize));
      if (data && datasize)
	_sendall(c->bio, data, datasize);
    }
}

/**********************************************************/

gboolean
_CCL_client_restore(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  CCL_interval *intervals = NULL;
  guint *products = NULL;

  g_return_val_if_fail(c, FALSE);

  cmd = sqlite3_mprintf("select status,intervals,products,timeout,\n"
			"member,flags from clients where id = %d;",
			client);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    {
      gint inum = sqlite3_column_bytes(stmt, 1) / sizeof(CCL_interval);
      gint pnum = sqlite3_column_bytes(stmt, 2) / (2 * sizeof(guint));
      gint i;
      GSList *link = NULL;

      /* free previous intervals and data */
      while (c->intervals->len)
	g_free(g_ptr_array_remove_index_fast(c->intervals, 0));
      while ((link = g_slist_last(c->products)) &&
	     (c->products = g_slist_remove_link(c->products, link)))
	{
	  g_free(link->data);
	  g_slist_free_1(link);
	}

      c->status = sqlite3_column_int(stmt, 0);
      /* intervals */
      if (inum)
	{
	  intervals = g_new0(CCL_interval, inum);
	  memcpy(intervals, sqlite3_column_blob(stmt, 1),
		 inum * sizeof(CCL_interval));
	  for (i = 0; i < inum; i++)
	    {
	      CCL_interval *interval = g_new0(CCL_interval, 1);

	      interval->stime = intervals[i].stime;
	      interval->etime = intervals[i].etime;
	      g_ptr_array_add(c->intervals, interval);
	    }
	  g_free(intervals);
	}
      /* products */
      if (pnum)
	{
	  products = g_malloc0(pnum * (sizeof(gint) + sizeof(guint)));
	  memcpy(products, sqlite3_column_blob(stmt, 2),
		 pnum * (sizeof(gint) + sizeof(guint)));
	  for (i = 0; i < pnum; i++)
	    {
	      gint id = ((gint *)products)[i * 2];
	      guint amount = ((guint *)products)[i * 2 + 1];
	      guint *pair = g_malloc0(sizeof(gint) + sizeof(guint));

	      ((gint *) pair)[0] = id;
	      ((guint *) pair)[1] = amount;
	      c->products = g_slist_append(c->products, pair);
	    }
	  g_free(products);
	}

      /************/
      c->timeout = sqlite3_column_int(stmt, 3);
      c->member = sqlite3_column_int(stmt, 4);
      c->flags = sqlite3_column_int(stmt, 5);
    }
  
  sqlite3_finalize(stmt);

  return TRUE;
}

void
_CCL_client_init(CCL_client * client, const gchar * name)
{
  client->name = g_strdup(name);
  client->sockfd = INVALID_SOCKET;
  client->timeout = 0;
  client->intervals = g_ptr_array_sized_new(5);
  client->products = NULL;
  client->status = 0;
  client->ipaddr = 0L;
}

void
_shutdown_connection(CCL_client * client)
{
  g_assert(client);

  if (client->bio)
    {
      BIO_free(client->bio);
      client->bio = NULL;
    }

  if (INVALID_SOCKET != client->sockfd)
    {
      FD_CLR(client->sockfd, &(ccl->events.readfds));
      if (ccl->events.maxfd == client->sockfd)
	{
	  ccl->events.maxfd = 0;
	  g_datalist_foreach(&(ccl->clients), _FindGreatestFdFunc,
			     &(ccl->events.maxfd));
	}
      client->sockfd = INVALID_SOCKET;
      /*Clear the address */
      client->ipaddr = 0L;
    }
}

void
_destroy_client(gpointer data)
{
  CCL_client *client = (CCL_client *) data;
  GSList *link = NULL;

  _shutdown_connection(client);

  g_free(client->name);

  while (client->intervals->len)
    g_free(g_ptr_array_remove_index_fast(client->intervals, 0));
  
  g_ptr_array_free(client->intervals, TRUE);
  
  while ((link = g_slist_last(client->products)) &&
	 (client->products = g_slist_remove_link(client->products, link)))
    {
      g_free(link->data);
      g_slist_free_1(link);
    }
  
  g_free(client);
}

/* Static */
static gint
_FindProductByIdFunc(gconstpointer a, gconstpointer b)
{
  if ((((guint *) a)[0]) == GPOINTER_TO_INT(b))
    return 0;
  else
    return 1;
}

static void
_FindGreatestFdFunc(GQuark key_id, gpointer data, gpointer user_data)
{
  CCL_client *client = (CCL_client *) data;
  gint *fd = (gint *) user_data;

  if (*fd < client->sockfd)
    *fd = client->sockfd;
}

static gboolean
_CCL_client_store(gint client)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint i;
  gint id;
  guint amount;
  CCL_interval *interval = NULL;
  CCL_interval *intervals = NULL;
  guint *products = NULL;
  gint ibytes;
  gint pbytes;

  g_return_val_if_fail(c, FALSE);
      
  intervals = g_new0(CCL_interval, c->intervals->len);
  ibytes = sizeof(CCL_interval) * c->intervals->len;

  /* Intervals */
  for (i = 0; i < c->intervals->len; i++)
    {
      interval = (CCL_interval *) g_ptr_array_index(c->intervals, i);
      intervals[i].stime = interval->stime;
      intervals[i].etime = interval->etime;
    }

  /* Products */
  for (i = 0; CCL_client_product_get_nth(client, i, NULL, NULL); i++)
    ;

  pbytes = (i * (sizeof(gint) + sizeof(guint)));
  products = g_malloc0(pbytes);
  
  for (i = 0; CCL_client_product_get_nth(client, i, &id, &amount); i++)
    {
      ((gint *) products)[2 * i] = id;
      ((guint *) products)[2 * i + 1] = amount;
    }

  /********************/

  cmd = sqlite3_mprintf("update clients\n"
			"set status = %d,\n"
			"    intervals = ?1,\n"
			"    products = ?2,\n"
			"    timeout = %ld,\n"
			"    member = %d,\n"
			"    flags = %d\n"
			"where id = %d;", c->status, c->timeout,
			c->member, c->flags, client);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  sqlite3_bind_blob(stmt, 1, intervals, ibytes, g_free);
  sqlite3_bind_blob(stmt, 2, products, pbytes, g_free);
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  return TRUE;
}
