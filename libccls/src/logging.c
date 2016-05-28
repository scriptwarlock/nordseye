#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ccls.h"
#include "ccl_private.h"
/*
#define DEBUG 1
*/
extern CCL *ccl;

/* Static functions */
static gint _highest_logged_session_id();
static gint _construct_query_part(const CCL_log_search_rules * sr, guint parts,
				  gchar buf[], gint buf_size);

/* Public interface */

/**
 * Log an expense.
 *
 * @param   description The description text.
 * @param   cash How much was paid.
 */
void
CCL_log_expense(const gchar description[128], guint cash, gint flags)
{
  gchar *cmd = NULL;

  cmd = sqlite3_mprintf("insert into expenseslog (description, time, cash,\n"
			"flags)\n"
			"values(%Q, %ld, %u, %d);",
			description, time(NULL), cash, flags);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Get a list of expenses that match with the search rules.
 *
 * @param   sr The search rules.
 * @param[out] ee This pointer will be set to point to an array of results.
 * @return The amount of results found.
 *
 * After you are done with the values, you must free ee with CCL_free.
 */
gint
CCL_log_expenses_get(CCL_log_search_rules * sr, CCL_log_expense_entry ** ee)
{
  CCL_log_expense_entry *entryarray = NULL;
  gint nrow = 0;
  gint ncol = 0;
  gchar *cmd = NULL;
  gchar **argv = NULL;
  gchar querypart[8096] = { '\0' };

  if (sr)
    {
      guint rules = CCL_SR_ID|CCL_SR_TIMEMIN|CCL_SR_TIMEMAX|CCL_SR_PRICEMIN
		    |CCL_SR_PRICEMAX|CCL_SR_FLAGS|CCL_SR_FLAGSNOT
		    |CCL_SR_DAYTIME_RANGE|CCL_SR_DAYS|CCL_SR_EMPLOYEE;
      
      _construct_query_part(sr, rules, querypart,
			    sizeof(querypart) / sizeof(gchar));
    }

  cmd = sqlite3_mprintf("select description,time,cash,empid,flags\n"
			"from expenseslog where cash != -1\n"
			"%s order by time asc;", querypart);

  if (sqlite3_get_table(ccl->db, cmd, &argv, &nrow, &ncol, NULL) == SQLITE_OK
      && argv && nrow > 0 && ncol)
    {
      gint i;
      
      if (!ee)	      /* If !ee, return the number of found entries */
	{
	  sqlite3_free(cmd);
	  sqlite3_free_table(argv);
	  return nrow;
	}
		      /* Otherwise, lets fill the array with found entries */
      entryarray = g_new0(CCL_log_expense_entry, nrow);
      
      for (i = 0; i < nrow; i++)
	{
	  gint os = (i+1) * ncol; /* offset */
	 
	  g_snprintf((gchar *) entryarray[i].description, 128, "%s", argv[os]);
	  sscanf(argv[os+1], "%ld", &(entryarray[i].time));
	  sscanf(argv[os+2], "%u", &(entryarray[i].cash));
	  sscanf(argv[os+3], "%d", &(entryarray[i].flags));
	}
    }

  sqlite3_free(cmd);
  sqlite3_free_table(argv);
  *ee = entryarray;
  
  return nrow;	/* nrow is the number of found entries */
}

/**
 * Log a session.
 *
 * @param   client The client's id.
 * @param   price The amount paid.
 * @param   paid The flags.
 * @return The logged session id.
 */
gint
CCL_log_session(gint client, guint price, gint flags, gint emp)
{
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint i;
  gint id;
  guint amount;
  guint nintervals;
  gint ibytes;
  gint sessionid;
  time_t stime;
  time_t etime;
  CCL_interval *interval = NULL;
  CCL_interval *intervals = NULL;

  g_return_val_if_fail((c && c->status == CCL_INACTIVE), 0);
  
  nintervals = c->intervals->len;
  ibytes = nintervals * sizeof(CCL_interval);
  sessionid = 1 + _highest_logged_session_id();
  stime = CCL_client_stime_get(client);
  etime = CCL_client_etime_get(client);
  interval = NULL;
  intervals = g_new0(CCL_interval, nintervals);

  for (i = 0; i < nintervals; i++)
    {
      interval = (CCL_interval *) g_ptr_array_index(c->intervals, i);
      intervals[i].stime = interval->stime;
      intervals[i].etime = interval->etime;
    }

  cmd = sqlite3_mprintf("insert into sessionslog (id, client, empid, member,"
			"stime, etime, time, price, flags, intervals)\n"
			"values(%d, %d, %d, %d, %d, %d, %d, %u, %d, ?1);",
			sessionid, client, emp, c->member, stime, etime,
			CCL_client_time_used(client), price, flags);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
#ifdef DEBUG
  printf("CCL_log_sessions(): cmd = %s\n", cmd);
#endif
  sqlite3_free(cmd);
  sqlite3_bind_blob(stmt, 1, intervals, ibytes, g_free);
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  for (i = 0; CCL_client_product_get_nth(client, i, &id, &amount); i++)
    {
      guint price = 0;
      
      CCL_product_info_get(id, NULL, NULL, &price);
      cmd = sqlite3_mprintf("insert into productslog (session, client, empid,"
			    "member, product, amount, price, time, flags)\n"
			    "values(%d, %d, %d, %d, %d, %u, %u, %ld, %d);",
			    sessionid, client, emp, c->member, id, amount,
			    price * amount, etime, flags);
      sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
      sqlite3_free(cmd);
    }
#ifdef DEBUG
  printf("CCL_log_session(): Number of Products = %d\n", i);
#endif

  return sessionid;
}

/**
 * Find a logged session id.
 *
 * @param   client The client's id.
 * @param   stime The session start time.
 * @param   etime The session end time.
 * @return The session id if found, -1 otherwise.
 */
gint
CCL_log_session_find(gint client, time_t stime, time_t etime)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint session = -1;

  g_return_val_if_fail(stime <= etime, -1);

  cmd = sqlite3_mprintf("select id from sessionslog\n"
			"where client = %d and stime = %ld and etime = %ld;",
			client, stime, etime);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  if (sqlite3_step(stmt) == SQLITE_ROW)
    session = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);

  return session;
}

/**
 * Remove a session from the log.
 *
 * @param   session The session's id.
 */
void
CCL_log_session_clear(gint session)
{
  gchar *cmd = NULL;

  if (session <= 0)
    return;

  cmd = sqlite3_mprintf("delete from sessionslog where id = %d", session);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
  cmd = sqlite3_mprintf("delete from productslog where session = %d", session);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Set a session flags.
 *
 * @param   session The session's id.
 * @param   flags The flags.
 */
void
CCL_log_session_set_flags(gint session, gint flags)
{
  gchar *cmd = NULL;

  cmd = sqlite3_mprintf("update sessionslog set flags = %d\n"
			"where id = %d;", flags, session);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Set a session's price.
 *
 * @param   session The session's id.
 * @param   price The price.
 */
void
CCL_log_session_set_price(gint session, guint price)
{
  gchar *cmd = NULL;

  cmd = sqlite3_mprintf("update sessionslog set price = %u\n"
			"where id = %d;", price, session);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Set a member for a logged session.
 *
 * @param   session The session's id.
 * @param   member The member's id.
 */
void
CCL_log_session_set_member(gint session, gint member)
{
  gchar *cmd = NULL;
  
  g_return_if_fail(member == 0 || CCL_member_exists(member));

  cmd = sqlite3_mprintf("update sessionslog set member = %d\n"
			"where id = %d;", member, session);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
  cmd = sqlite3_mprintf("update productslog set member = %d\n"
			"where session = %d;", member, session);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Get a list of sessions that match with the search rules.
 *
 * @param   sr The search rules.
 * @param[out] ee This pointer will be set to point to an array of results.
 * @return The amount of results found.
 *
 * After you are done with the values, you must free se with CCL_free.
 */
gint
CCL_log_sessions_get(CCL_log_search_rules * sr, CCL_log_session_entry ** se)
{
  CCL_log_session_entry *entryarray = NULL;
  gint nrow = 0;
  gint ncol = 0;
  gchar *cmd = NULL;
  gchar **argv = NULL;
  gchar querypart[8096] = { '\0' };

  if (sr)
    {
      guint rules = CCL_SR_ID|CCL_SR_CLIENT|CCL_SR_MEMBER|CCL_SR_STIMEMIN
		    |CCL_SR_STIMEMAX|CCL_SR_ETIMEMIN|CCL_SR_ETIMEMAX
		    |CCL_SR_TIMEMIN|CCL_SR_TIMEMAX|CCL_SR_PRICEMIN
		    |CCL_SR_PRICEMAX|CCL_SR_FLAGS|CCL_SR_FLAGSNOT
		    |CCL_SR_DAYTIME_RANGE|CCL_SR_DAYS|CCL_SR_EMPLOYEE;
      
      _construct_query_part(sr, rules, querypart,
			    sizeof(querypart) / sizeof(gchar));
    }

  cmd = sqlite3_mprintf("select id, client, member, empid,\n"
			"stime, etime, time, price, flags\n"
			"from sessionslog where id != -1\n"
			"%s order by stime asc;", querypart);

#ifdef DEBUG
  printf("CCL_log_sessions_get(): cmd = %s\n", cmd);
#endif
  if (sqlite3_get_table(ccl->db, cmd, &argv, &nrow, &ncol, NULL) == SQLITE_OK
      && argv && nrow > 0 && ncol)
    {
      gint i;
      
      if (!se)	      /* If !se, return the number of found entries */
	{
	  sqlite3_free(cmd);
	  sqlite3_free_table(argv);
	  return nrow;
	}
      /* Otherwise, lets fill the array with found entries */
      entryarray = g_new0(CCL_log_session_entry, nrow);
      
      for (i = 0; i < nrow; i++)
	{
	  gint os = (i+1) * ncol; /* offset */
	  
	  sscanf(argv[os], "%d", &(entryarray[i].id));
	  sscanf(argv[os+1], "%d", &(entryarray[i].client));
	  sscanf(argv[os+2], "%d", &(entryarray[i].member));
	  sscanf(argv[os+3], "%d", &(entryarray[i].employee));
	  sscanf(argv[os+4], "%ld", &(entryarray[i].stime));
	  sscanf(argv[os+5], "%ld", &(entryarray[i].etime));
	  sscanf(argv[os+6], "%d", &(entryarray[i].time));
	  sscanf(argv[os+7], "%u", &(entryarray[i].price));
	  sscanf(argv[os+8], "%d", &(entryarray[i].flags));
	}
    }
 
  sqlite3_free(cmd);
  sqlite3_free_table(argv);
  *se = entryarray;
  
  return nrow;	/* nrow is the number of found entries */
}

/**
 * Gets the time intervals of a session.
 *
 * @param   session The session's id.
 * @param[out] intervals The pointer to the array of results will be put here.
 * @return The number of results.
 *
 * After you are done with the results you must free the intervals with
 * CCL_free (if the return value was > 0).
 */
gint
CCL_log_session_intervals_get(gint session, time_t ** intervals)
{
  gchar *cmd;
  sqlite3_stmt *stmt = NULL;
  gint inum = 0;

  cmd = sqlite3_mprintf("select intervals from sessionslog\n"
			"where id = %d;", session);

  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  if (sqlite3_step(stmt) == SQLITE_ROW)
    {
      gint bytes = sqlite3_column_bytes(stmt, 0);
      
      inum = bytes / sizeof(CCL_interval);
      
      if (inum)
	{
	  *intervals = g_malloc0(bytes);
	  memcpy(*intervals, sqlite3_column_blob(stmt, 0), bytes);
	}
    }
  sqlite3_finalize(stmt);

  return inum;
}

/**
 * Set a productlog entry's flags.
 *
 * @param   id The entry's id.
 * @param   flags The flags.
 */
void
CCL_log_product_set_flags(gint id, gint flags)
{
  gchar *cmd = NULL;

  cmd = sqlite3_mprintf("update productslog set flags = %d\n"
			"where id = %d;", flags, id);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Set a productlog entry's price.
 *
 * @param   id The entry's id.
 * @param   price The price.
 */
void
CCL_log_product_set_price(gint id, guint price)
{
  gchar *cmd = NULL;

  cmd = sqlite3_mprintf("update productslog set price = %u\n"
			"where id = %d;", price, id);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Get a list of sold products that match with the search rules.
 *
 * @param   sr The search rules.
 * @param[out] ee This pointer will be set to point to an array of results.
 * @return The amount of results found.
 *
 * After you are done with the values, you must free pe with CCL_free.
 */
gint
CCL_log_products_get(CCL_log_search_rules * sr, CCL_log_product_entry ** pe )
{
  CCL_log_product_entry *entryarray = NULL;
  gint nrow = 0;
  gint ncol = 0;
  gchar *cmd = NULL;
  gchar **argv = NULL;
  gchar querypart[8096] = { '\0' };
  
  if (sr)
    {
      guint rules = CCL_SR_ID|CCL_SR_SESSION|CCL_SR_CLIENT|CCL_SR_MEMBER
		    |CCL_SR_TIMEMIN|CCL_SR_TIMEMAX
		    |CCL_SR_PRICEMIN|CCL_SR_PRICEMAX
		    |CCL_SR_FLAGS|CCL_SR_FLAGSNOT|CCL_SR_PRODUCT
		    |CCL_SR_DAYTIME_RANGE|CCL_SR_DAYS|CCL_SR_EMPLOYEE;
      
      _construct_query_part(sr, rules, querypart,
			    sizeof(querypart) / sizeof(gchar));
    }

  cmd = sqlite3_mprintf("select id, session, client, member,empid,\n"
			"product, amount, price, time, flags\n"
			"from productslog where session != -1\n"
			"%s order by time asc;", querypart);
#ifdef DEBUG
  printf("CCL_log_products_get(): cmd = %s\n", cmd);
#endif

  if (sqlite3_get_table(ccl->db, cmd, &argv, &nrow, &ncol, NULL) == SQLITE_OK
      && argv && nrow > 0 && ncol)
    {
      gint i;
     
      if (!pe)		/* If !pe, return the number of found entries */
	{
	  sqlite3_free(cmd);
	  sqlite3_free_table(argv);
	  return nrow;
	}

      entryarray = g_new0(CCL_log_product_entry, nrow);
      
      for (i = 0; i < nrow; i++)
	{
	  gint os = (i+1) * ncol; /* offset */
	  
	  sscanf(argv[os], "%d", &(entryarray[i].id));
	  sscanf(argv[os+1], "%d", &(entryarray[i].session));
	  sscanf(argv[os+2], "%d", &(entryarray[i].client));
	  sscanf(argv[os+3], "%d", &(entryarray[i].member));
	  sscanf(argv[os+4], "%d", &(entryarray[i].employee));
	  sscanf(argv[os+5], "%d", &(entryarray[i].product));
	  sscanf(argv[os+6], "%u", &(entryarray[i].amount));
	  sscanf(argv[os+7], "%u", &(entryarray[i].price));
	  sscanf(argv[os+8], "%ld", &(entryarray[i].time));
	  sscanf(argv[os+9], "%d", &(entryarray[i].flags));
	}
    }

  sqlite3_free(cmd);
  sqlite3_free_table(argv);
  *pe = entryarray;
  
  return nrow;
}

/* Static */
static gint
_highest_logged_session_id()
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint id = 0;

  cmd = sqlite3_mprintf("select max(id) from sessionslog;");
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  if (sqlite3_step(stmt) == SQLITE_ROW &&
      sqlite3_column_type(stmt, 0) != SQLITE_NULL)
    id = sqlite3_column_int(stmt, 0);
  
  sqlite3_finalize(stmt);

  return id;
}

static gint
_construct_query_part(const CCL_log_search_rules * sr, guint parts,
		      gchar buf[], gint buf_size)
{
  gint pos = 0;
  gint tdiff;
  struct tm t;
  
  memset(&t, 0, sizeof(struct tm));
  t.tm_year = 70;
  t.tm_mday = 1;
  tdiff = (gint)mktime(&t);

#define _ADD_TEST(rule, val, string) \
  if (parts & sr->rulemask & rule) \
    pos += g_snprintf(buf + pos, buf_size - pos, string " ", val)

  _ADD_TEST(CCL_SR_ID, sr->id, "and id = %d");
  _ADD_TEST(CCL_SR_SESSION, sr->session, "and session = %d");
  _ADD_TEST(CCL_SR_CLIENT, sr->client, "and client = %d");
  _ADD_TEST(CCL_SR_MEMBER, sr->member, "and member = %d");
  _ADD_TEST(CCL_SR_EMPLOYEE, sr->employee, "and empid = %d");
  _ADD_TEST(CCL_SR_STIMEMIN, sr->stime_min, "and stime >= %ld");
  _ADD_TEST(CCL_SR_STIMEMAX, sr->stime_max, "and stime <= %ld");
  _ADD_TEST(CCL_SR_ETIMEMIN, sr->etime_min, "and etime >= %ld");
  _ADD_TEST(CCL_SR_ETIMEMAX, sr->etime_max, "and etime <= %ld");
  _ADD_TEST(CCL_SR_TIMEMIN, sr->time_min, "and time >= %ld");
  _ADD_TEST(CCL_SR_TIMEMAX, sr->time_max, "and time <= %ld");
  _ADD_TEST(CCL_SR_PRICEMIN, sr->price_min, "and price >= %u");
  _ADD_TEST(CCL_SR_PRICEMAX, sr->price_max, "and price <= %u");
  _ADD_TEST(CCL_SR_PRODUCT, sr->product, "and product = %d");

  if (parts & sr->rulemask & CCL_SR_FLAGS)
    {
      if (sr->rulemask & CCL_SR_FLAGS_ANY)
	pos += g_snprintf(buf + pos, buf_size - pos,
			  "and flags = (%d | flags) ", sr->flags);
      else
	pos += g_snprintf(buf + pos, buf_size - pos,
			  "and %d = (flags & %d) ", sr->flags, sr->flags);
    }

  if (parts & sr->rulemask & CCL_SR_FLAGSNOT)
    {
      if (sr->rulemask & CCL_SR_FLAGSNOT_ANY)
	pos += g_snprintf(buf + pos, buf_size - pos,
			  "and flags != (%d | flags) ", sr->flags_not);
      else
	pos += g_snprintf(buf + pos, buf_size - pos,
			  "and 0 = (flags & %d) ", sr->flags_not);
    }


  if (parts & sr->rulemask & CCL_SR_DAYTIME_RANGE)
    {
      if (sr->daytime_min == 0 && sr->daytime_max >= 86399)
	; /* Ignore this rule, because all the day is covered */
      else if (sr->daytime_min <= sr->daytime_max)
	pos += g_snprintf(buf + pos, buf_size - pos,
			  "and (stime - %d) %% 86400 >= %d "
			  "and (stime - %d) %% 86400 <= %d ",
			  tdiff, sr->daytime_min,
			  tdiff, sr->daytime_max);
      else
	pos += g_snprintf(buf + pos, buf_size - pos,
			  "and ((stime - %d) %% 86400 >= %d "
			  "or (stime - %d) %% 86400 <= %d) ",
			  tdiff, sr->daytime_min,
			  tdiff, sr->daytime_max);
    }
  if (parts & sr->rulemask & CCL_SR_DAYS && sr->days < 127)/*127 is all days*/
      pos += g_snprintf(buf + pos, buf_size - pos,
			"and (((stime - %d) / 86400) %% 7 = %u "
			"or ((stime - %d) / 86400) %% 7 = %u "
			"or ((stime - %d) / 86400) %% 7 = %u "
			"or ((stime - %d) / 86400) %% 7 = %u "
			"or ((stime - %d) / 86400) %% 7 = %u "
			"or ((stime - %d) / 86400) %% 7 = %u "
			"or ((stime - %d) / 86400) %% 7 = %u) ",
			tdiff, (sr->days & CCL_SUN ? 3 : 7 ),
			tdiff, (sr->days & CCL_MON ? 4 : 7 ),
			tdiff, (sr->days & CCL_TUE ? 5 : 7 ),
			tdiff, (sr->days & CCL_WED ? 6 : 7 ),
			tdiff, (sr->days & CCL_THU ? 0 : 7 ),
			tdiff, (sr->days & CCL_FRI ? 1 : 7 ),
			tdiff, (sr->days & CCL_SAT ? 2 : 7 ));
      /* it has an offset of - 5 days because 01/01/1970 is thursday
       * (day 5 of the week) */

  return pos;
}
