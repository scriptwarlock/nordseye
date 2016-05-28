#include <stdlib.h>
#include <stdio.h>
#include "ccls.h"
#include "ccl_private.h"
/*
#define DEBUG 1
*/
/* Static functions */
static void _tarif_rebuild(gint tarif_id, GSList ** tarif);
static void _tarif_clear(gint tarif_id);
static time_t _weekstart(time_t t);
static guint _CCL_tarif_realcalc(GSList * slist, time_t s, time_t e,
				 gboolean permin);

extern CCL *ccl;

/* Public interface */

/**
 * Create a new tarif.
 *
 * @param   hr The starting hour.
 * @param   min The starting minute.
 * @param   days The days it covers.
 * @param   hourprice The price per hour.
 * @return The new tarif's id, or -1 if failed.
 */
gint
CCL_tarif_new(guint hr, guint min, guint days, guint hourprice, 
	      guint incprice, guint fafter, char *name)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint tarif_id;
  gint i;

  g_return_val_if_fail(days <= 127 && days, -1);
  g_return_val_if_fail(hr <= 23 && min <= 59, -1);

  for (i = 0; -1 != CCL_tarif_get_nth(i); i++)
    ;
  
  tarif_id = i ? 1 + CCL_tarif_get_nth(i - 1) : 1;
  cmd = sqlite3_mprintf("insert into tarifs\n"
			"(tarif, days, stime, hourprice, incprice, fafter, tname)\n"
			"values(%d, %u, '%.2u:%.2u', %u, %u, %d, %Q);",
			tarif_id, days, hr, min, hourprice, incprice, fafter, name);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  return tarif_id;
}

/**
 * Deletes a tarif.
 *
 * @param   tarif The tarif's id.
 */
void
CCL_tarif_delete(gint tarif)
{
  gint i;
  gint part;
  gint current_tarif = ccl->tarif_id;
  gchar *cmd = NULL;

  g_return_if_fail(tarif != current_tarif && tarif > 0);
 
  CCL_tarif_set(tarif);
 
  for (i = 0; -1 != (part = CCL_tarifpart_get_nth(i)); i++)
    CCL_tarifpart_price_clear(part);

  CCL_tarif_set(current_tarif);
  
  cmd = sqlite3_mprintf("delete from tarifs where tarif = %d;", tarif);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);

}

/**
 * Gets the nth tarif on the list.
 *
 * @param   nth The index of the tarif to get (starting at 0).
 * @return The tarif's id, or -1 if there aren't more tarifs.
 *
 * Use this to iterate on the tarifs, for example when building a list of
 * all the tarifs.
 */
gint
CCL_tarif_get_nth(guint nth)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint i;
  gint tarif = -1;

  cmd = sqlite3_mprintf("select distinct tarif from tarifs\n"
			"order by tarif asc;");
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  for (i = nth; i > 0; i--)
    sqlite3_step(stmt);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    tarif = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);

  return tarif;
}

/**
 * Gets the tariff name
 *
 * @param   tarif id
 * @return  pointer to tarif name - remember to free
 *
 */
gchar *
CCL_tarif_name_get(gint tarif)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  char *cp = NULL;

  cmd = sqlite3_mprintf("select tname from tarifs\n"
			"where tarif = %d;", tarif);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    cp = g_strdup((gchar *) sqlite3_column_text(stmt, 0));
  sqlite3_finalize(stmt);

  return cp;
}

/**
 * Rebuild the current tarif from the data on the db.
 *
 * This should be used after you are done modifying the tarif.
 */
void
CCL_tarif_rebuild(void)
{
  g_return_if_fail(ccl->tarif_id >= 1);
  
  _tarif_clear(ccl->tarif_id);
  ccl->tarif = g_datalist_id_get_data(&(ccl->tarifs), ccl->tarif_id);
  _tarif_rebuild(ccl->tarif_id, &(ccl->tarif));
}

/**
 * Rebuild all the tarifs from the data on the db.
 *
 * This should be used after you have initialized CCL.
 */
void
CCL_tarif_rebuild_all(void)
{
  GSList ** tarif = NULL;
  gint tarif_id;
  gint i;

  for (i = 0; -1 != (tarif_id = CCL_tarif_get_nth(i)); i++)
    _tarif_clear(tarif_id);

  g_datalist_clear(&ccl->tarifs);
  g_datalist_init(&ccl->tarifs);
  
  for (i = 0; -1 != (tarif_id = CCL_tarif_get_nth(i)); i++)
    {
      tarif = g_new0(GSList *, 1);
      *tarif = NULL;
      _tarif_rebuild(tarif_id, tarif);
      g_datalist_id_set_data(&ccl->tarifs, tarif_id, *tarif);
      g_free(tarif);
    }
#ifdef DEBUG
  printf("CCL_tarif_rebuild_all(): [ %d ] Tariffs found\n", i);
#endif
}

/**
 * Sets the current tarif.
 *
 * @param   tarif The tarif's id.
 * @return FALSE if something failed, TRUE otherwise.
 */
gboolean
CCL_tarif_set(gint tarif)
{
  int id;
 
  g_return_val_if_fail(CCL_tarif_exists(tarif), FALSE);
  g_assert(g_datalist_id_get_data(&ccl->tarifs, tarif));

  ccl->tarif_id = tarif;
  ccl->tarif = g_datalist_id_get_data(&ccl->tarifs, tarif);
  /*added by bernard: sets also perminafter */
  id = CCL_tarifpart_get_nth(0);
  ccl->perminafter = CCL_tarifpart_fafter_get(id);
#ifdef DEBUG
  printf("Tarif: ");
  for (int i=0; (id=CCL_tarifpart_get_nth(i))>0; i++)
    printf("[%d] ", CCL_tarifpart_fafter_get(id));
  printf("\n");
#endif

  return TRUE;
}

/**
 * Gets the current tarif.
 *
 * @return The current tarif's id.
 */
gint
CCL_tarif_get(void)
{
  return ccl->tarif_id;
}

/**
 * Checks if a tarif exists.
 *
 * @param   tarif The tarif's id.
 * @return TRUE if exists, FALSE if not.
 */
gboolean
CCL_tarif_exists(gint tarif)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gboolean found = FALSE;

  cmd = sqlite3_mprintf("select tarif from tarifs where tarif = %d;", tarif);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    found = TRUE;

  sqlite3_finalize(stmt);

  return found;
}

/**
 * Checks if a tarif's name exists.
 *
 * @param   tarif The tarif's name.
 * @return  tarif id if found, -1 if not found
 */
gint
CCL_tarif_name_exists(char *tarif)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  int tid;

  tid = -1;
  cmd = sqlite3_mprintf("select tarif from tarifs where \n"
			"lower(tname) = lower(%Q);", tarif);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    tid = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);

  return tid;
}

/**
 * Gets the tarif part that would apply at hour:min of day.
 *
 * @param   hour The hour.
 * @param   min The minute.
 * @param   day The day.
 * @return The part's id.
 */
gint
CCL_tarif_get_part_at(guint hour, guint min, guint day)
{
  GSList *link = ccl->tarif;
  guint stime = hour * 60 + min;
  gint i;

  g_return_val_if_fail((hour <= 23 && min <= 59 &&
			(day == CCL_SUN || day == CCL_MON || day == CCL_TUE ||
			 day == CCL_WED || day == CCL_THU || day == CCL_FRI ||
			 day == CCL_SAT)), -1);

  for (i = 0; (1 << i) != day; i++)
    ;

  stime += i * 24 * 60;

  while (link)
    {
      CCL_tarifpart *curr = (CCL_tarifpart *) link->data;
      CCL_tarifpart *next = NULL;
      if (g_slist_next(link))
	next = (CCL_tarifpart *) g_slist_next(link)->data;
      else
	next = (CCL_tarifpart *) ccl->tarif->data;
	  
      if ((curr->stime <= stime && next->stime > stime)
	  || (curr->stime > next->stime))
	return curr->id;
      
      link = g_slist_next(link);
    }

  g_assert_not_reached();
  
  return -1;
}

/**
 * Calculate the price of a given time.
 *
 * @param   start The start time.
 * @param   end The end time.
 * @param   permin Calculate per minute if TRUE.
 * @return The calculated price.
 */
guint
CCL_tarif_calc(time_t start, time_t end, gboolean permin)
{
  return _CCL_tarif_realcalc(ccl->tarif, start, end, permin);
}

/**
 * Calculate the price of mins minutes using the given tarif part.
 *
 * @param   id The tarifpart's id.
 * @param   mins How many minutes.
 * @param   permin Calculate per minute if TRUE.
 * @return The calculated price.
 */
guint
CCL_tarif_calc_with_tarifpart(gint id, guint mins, gboolean permin)
{
  guint hprice = CCL_tarifpart_hourprice_get(id);

  g_return_val_if_fail(CCL_tarifpart_exists(id), 0);

  if (permin)
    return (hprice * mins) / 60;
  else
    return CCL_tarifpart_price_get(id, mins, FALSE);
}

/**********************************************************/

/**
 * Add a new part to the current tarif.
 *
 * @param   hour The starting hour.
 * @param   min The starting minute.
 * @param   days The days it applies.
 * @param   hourprice The price per hour.
 * @return The tarifpart's id.
 */
gint
CCL_tarifpart_new(guint hour, guint min, guint days, guint hourprice, guint incprice)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;

  g_return_val_if_fail(-1 == CCL_tarifpart_id_get(hour, min, days), -1);
  g_return_val_if_fail(!CCL_tarifpart_conflicts(hour, min, days, NULL), -1);
  g_return_val_if_fail(days <= 127 && days, -1);
  g_return_val_if_fail(1 <= ccl->tarif_id, -1);

  cmd = sqlite3_mprintf("insert into tarifs\n"
			"(tarif, days, stime, hourprice, incprice)\n"
			"values(%d, %u, '%.2u:%.2u', %u, %u);",
			ccl->tarif_id, days, hour, min, hourprice, incprice);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  return sqlite3_last_insert_rowid(ccl->db);
}

/**
 * Deletes a part from the current tarif.
 *
 * @param   id The part's id.
 */
void
CCL_tarifpart_delete(gint id)
{
  gchar *cmd = NULL;

  cmd = sqlite3_mprintf("delete from tarifs where id = %d;\n"
			"delete from prices where id = %d;",
			id, id);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Gets the nth tarifpart of the current tarif.
 *
 * @param   nth The index of the tarifpart to get (starting at 0).
 * @return The tarifpart's id, or -1 if there aren't more tarifparts.
 *
 * Use this to iterate on the tarifparts, for example when building a list of
 * all the tarifparts of the current tarif.
 */
gint
CCL_tarifpart_get_nth(guint nth)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint i;
  gint id = -1;

  cmd = sqlite3_mprintf("select id from tarifs where tarif = %d\n"
			"order by id asc;", ccl->tarif_id);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  for (i = nth; i > 0; i--)
    sqlite3_step(stmt);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    id = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);

  return id;
}

/**
 * Gets the id of a tarifpart at hour:min, that applies at days.
 *
 * @param   hour The hour.
 * @param   min The minute.
 * @param   days The days it covers.
 * @return The found tarifpart's id, or -1 if none found.
 */
gint
CCL_tarifpart_id_get(guint hour, guint min, guint days)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint id = -1;

  g_return_val_if_fail(hour <= 23 && min <= 59, -1);

  cmd = sqlite3_mprintf("select id from tarifs where\n"
			"tarif = %d and days = %u and stime = '%.2u:%.2u';",
			ccl->tarif_id, days, hour, min);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  if (sqlite3_step(stmt) == SQLITE_ROW)
    id = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);

  return id;
}

/**
 * Checks for starting time / days conflicts.
 *
 * @param   hour The starting hour.
 * @param   min The starting minute.
 * @param   days The days it covers.
 * @param[out] conflicts The conflicting days will be put here.
 * @return The conflicting part's id, or 0 if none.
 *
 * Use this before adding a new part to check if some other part has the
 * same start time, on one of the days that apply.
 * conflicts can be NULL if you don't care about the days where the conflict
 * happens.
 */
gint
CCL_tarifpart_conflicts(guint hour, guint min, guint days, guint * conflicts)
{
  guint nth = 0;
  gint id;
  
  while (-1 != (id = CCL_tarifpart_get_nth(nth)))
    {
      guint h;
      guint m;
      guint d;

      CCL_tarifpart_info_get(id, &h, &m, &d, NULL,NULL);
      if (h == hour && m == min && days & d)
	{
	  if (conflicts)
	    *conflicts = days & d;
	      
	  return id;
	}

      nth++;
    }
  
  return 0;
}

/**
 * Gets information about a tarifpart.
 *
 * @param   The tarif's id.
 * @param[out] The starting hour.
 * @param[out] The starting minute.
 * @param[out] The days it covers.
 * @param[out] The price per hour.
 * @return TRUE on success, FALSE on failure.
 *
 * Any of the [out] parameters can be NULL if you don't care about them.
 */
gboolean
CCL_tarifpart_info_get(gint id, guint *hour, guint *min, guint *days, 
		       guint *hourprice, guint *incprice)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gboolean found = FALSE;

  cmd = sqlite3_mprintf("select stime, days, hourprice, incprice from tarifs\n"
			"where id = %d;", id);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  if (sqlite3_step(stmt) == SQLITE_ROW)
    {
      guint _hour = 0;
      guint _min = 0;

      sscanf((gchar *) sqlite3_column_text(stmt, 0), "%u:%u", &_hour, &_min);
      if (hour)
	*hour = _hour;
      if (min)
	*min = _min;
      if (days)
	*days = sqlite3_column_int(stmt, 1);
      if (hourprice)
	*hourprice = sqlite3_column_double(stmt, 2);
      if (incprice)
	*incprice = sqlite3_column_double(stmt, 3);
      found = TRUE;
    }
  sqlite3_finalize(stmt);

  return found;
}

/**
 * Checks if a tarifpart exists.
 *
 * @param   id The tarifpart's id.
 * @return TRUE if exists, FALSE if not.
 */
gboolean
CCL_tarifpart_exists(gint id)
{
  return CCL_tarifpart_info_get(id, NULL, NULL, NULL, NULL, NULL);
}

/**
 * Sets a tarifpart's start sime.
 *
 * @param   id The tarifpart's id.
 * @param   hour The starting hour.
 * @param   min The starting minute.
 * @return TRUE on success, FALSE if bad hour/min value, or a conflict exists.
 */
gboolean
CCL_tarifpart_stime_set(gint id, guint hour, guint min)
{
  gchar *cmd = NULL;
  gint conflicting_part = CCL_tarifpart_conflicts(hour, min,
						  CCL_tarifpart_days_get(id),
						  NULL);

  g_return_val_if_fail(hour <= 23 && min <= 59, FALSE);
  g_return_val_if_fail(!conflicting_part || conflicting_part == id, FALSE);

  cmd = sqlite3_mprintf("update tarifs\n"
			"set stime = '%.2u:%.2u'\n"
			"where id = %d;",
			hour, min, id, ccl->tarif_id);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);

  return TRUE;
}

/**
 * Gets a tarifpart's starting time.
 *
 * @param   id The tarifpart's id.
 * @param[out] hour The starting hour will be put here.
 * @param[out] min The starting minute will be put here.
 */
void
CCL_tarifpart_stime_get(gint id, guint * hour, guint * min)
{
  CCL_tarifpart_info_get(id, hour, min, NULL, NULL, NULL);
}

/**
 * Sets the days a tarifpart covers.
 *
 * @param   id The tarifpart's id.
 * @param   days The days to cover.
 * @return TRUE on success, FALSE if bad days value, or a conflict exists.
 */
gboolean
CCL_tarifpart_days_set(gint id, guint days)
{
  gchar *cmd = NULL;
  guint hour, min;

  g_return_val_if_fail(days <= 127, FALSE);
  CCL_tarifpart_stime_get(id, &hour, &min);
  g_return_val_if_fail(CCL_tarifpart_conflicts(hour, min, days, NULL), FALSE);

  cmd = sqlite3_mprintf("update tarifs\n"
			"set days = %u where id = %d;",
			days, id);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);

  return TRUE;
}

/**
 * Gets the days a tarifpart covers.
 *
 * @param   id The tarifpart's id.
 * @return The days it covers.
 */
guint
CCL_tarifpart_days_get(gint id)
{
  guint days = 0;

  CCL_tarifpart_info_get(id, NULL, NULL, &days, NULL, NULL);

  return days;
}

/**
 * Gets a tarifpart's price per hour.
 *
 * @param   id The tarifpart's id.
 * @return The price.
 */
guint
CCL_tarifpart_hourprice_get(gint id)
{
  guint hprice = 0;

  CCL_tarifpart_info_get(id, NULL, NULL, NULL, &hprice, NULL);

  return hprice;
}

/**
 * Sets a tarifpart's price per hour.
 *
 * @param   id The tarifpart's id.
 * @param   price The price.
 */
void
CCL_tarifpart_hourprice_set(gint id, guint price)
{
  gchar *cmd = NULL;

  cmd = sqlite3_mprintf("update tarifs\n"
			"set hourprice = %u where id = %d;",
			price, id);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Gets a tarifpart's fractioned after
 *
 * @param   id The tarifpart's id.
 * @return number of minutes fractioned after
 */
guint
CCL_tarifpart_fafter_get(gint id)
{
  guint fafter = 0;
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;


  cmd = sqlite3_mprintf("select fafter from tarifs\n"
			"where id = %d;", id);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    fafter = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);

  return fafter;
}

/**
 * Sets a tarifpart's fractioned after
 *
 * @param   id The tarifpart's id.
 * @param   fractioned after minutes
 */
void
CCL_tarifpart_fafter_set(gint id, guint fafter)
{
  gchar *cmd = NULL;

  cmd = sqlite3_mprintf("update tarifs\n"
			"set fafter = %u where id = %d;", fafter, id);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Gets a tarifpart's increment
 *
 * @param   id The tarifpart's id.
 * @return the price increment
 */
guint
CCL_tarifpart_incprice_get(gint id)
{
  guint incprice = 0;
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;


  cmd = sqlite3_mprintf("select incprice from tarifs\n"
			"where id = %d;", id);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    incprice = (guint)sqlite3_column_double(stmt, 0);

  sqlite3_finalize(stmt);

  return incprice;
}

/**
 * Sets a tarifpart's increment
 *
 * @param   id The tarifpart's id.
 * @param   fractioned after minutes
 */
void
CCL_tarifpart_incprice_set(gint id, guint incprice)
{
  gchar *cmd = NULL;

  cmd = sqlite3_mprintf("update tarifs\n"
			"set incprice = %u where id = %d;", incprice, id);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Sets a tarifpart's flags to the value of flags.
 *
 * @param   tarifpart The tarifpart's id.
 * @param   flags The new flags value.
 */
void
CCL_tarifpart_flags_set(gint tarifpart, gint flags)
{
  gchar *cmd = NULL;

  g_return_if_fail(CCL_tarifpart_exists(tarifpart));
  
  cmd = sqlite3_mprintf("update tarifs\n"
			"set flags = %d where id = %d;",
			flags, tarifpart);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Gets the flags value of a tarifpart.
 *
 * @param   tarifpart The tarifpart's id.
 * @return The flags value.
 */
gint
CCL_tarifpart_flags_get(gint tarifpart)
{
  gint flags = 0;
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;

  g_return_val_if_fail(CCL_tarifpart_exists(tarifpart), 0);

  cmd = sqlite3_mprintf("select flags from from tarifs\n"
			"where id = %d;", tarifpart);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    flags = sqlite3_column_int(stmt, 0);
  
  sqlite3_finalize(stmt);

  return flags;
}

/**
 * Toggle a tarifpart's flags on or off.
 *
 * @param   tarifpart The tarifpart's id.
 * @param   flags The flags to toggle.
 * @param   on If TRUE toggle the flags on, else toggle them off.
 */
void
CCL_tarifpart_flags_toggle(gint tarifpart, gint flags, gboolean on)
{
  gchar *cmd = NULL;

  g_return_if_fail(CCL_tarifpart_exists(tarifpart));

  if (on)
    cmd = sqlite3_mprintf("update tarifs set flags = (flags | %d)\n"
			  "where id = %d;", flags, tarifpart);
  else
    cmd = sqlite3_mprintf("update tarifs set flags = (flags & (~%d))\n"
			  "where id = %d;", flags, tarifpart);

  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Adds a price to a tarifpart.
 *
 * @param   id The tarifpart's id.
 * @param   mins The amount of minutes.
 * @param   price The price for that amount of minutes.
 */
void
CCL_tarifpart_price_add(gint id, guint mins, guint price)
{
  gchar *cmd = NULL;

  g_return_if_fail((CCL_tarifpart_exists(id) &&
		    !CCL_tarifpart_price_exists(id, mins)));

  cmd = sqlite3_mprintf("insert into prices (tarifpart, mins, price)\n"
			"values(%d, %u, %u);",
			id, mins, price);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Deletes a price from a tarifpart.
 *
 * @param   id The tarifpart's id.
 * @param   mins The amount of minutes of the price.
 */
void
CCL_tarifpart_price_del(gint id, guint mins)
{
  gchar *cmd = NULL;
  
  g_return_if_fail(CCL_tarifpart_exists(id));

  cmd = sqlite3_mprintf("delete from prices\n"
			"where tarifpart = %d and mins = %u;",
			id, mins);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Remove all prices from a tarifpart.
 *
 * @param   id The tarifpart's id.
 */
void
CCL_tarifpart_price_clear(gint id)
{
  gchar *cmd = NULL;
  
  g_return_if_fail(CCL_tarifpart_exists(id));

  cmd = sqlite3_mprintf("delete from prices\n"
			"where tarifpart = %d;", id);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Gets the cost of mins amount of minutes for a tarifpart.
 *
 * @param   id The tarifpart's id.
 * @param   mins The amount of minutes.
 * @param   strict Only return a value > 0 if the exact price is found.
 * @return The price.
 *
 * If strict is TRUE, this is going to return 0 unless a price for mins amount
 * of minutes exists on the list of the tarifpart prices.
 *
 * If strict is FALSE, this is going to calculate the price for mins amount of
 * minutes, for example: if the tarifpart has a price for 15 minutes, and one
 * for 30 minutes, and mins is 19, this will return the price for 30 minutes.
 * If mins is 102, then the return value will be the sum of the prices of
 * 1 hour + the price of 30 minutes + the price of 15 minutes.
 * The tarifpart has at least the price for 60 minutes (the hourprice).
 *
 * This is not intended to calculate the price of a session, use any of
 * CCL_tarif_calc or CCL_tarif_calc_with_tarifpart for that.
 */
guint
CCL_tarifpart_price_get(gint id, guint mins, gboolean strict)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;

  g_return_val_if_fail(CCL_tarifpart_exists(id), 0);

  cmd = sqlite3_mprintf("select price from prices\n"
			"where tarifpart = %d and mins = %u;",
			id, mins);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  if (sqlite3_step(stmt) == SQLITE_ROW)
    {
      guint price;

      price = sqlite3_column_double(stmt, 0);
      sqlite3_finalize(stmt);

      return price;
    }
  else
    sqlite3_finalize(stmt);

  if (strict)
    return 0;
  else
    {
      guint lprice = CCL_tarifpart_price_get_last(id);
      
      if (mins < lprice)
	{
	  gint i;
	  guint _mins = 0;
	  guint _price = 0;

	  for (i = 0; CCL_tarifpart_price_get_nth(id, i, &_mins, &_price); i++)
	    {
	      if (mins < _mins)
		return _price;
	    }
	}
      else if (60 >= mins)
	return CCL_tarifpart_hourprice_get(id);
      else
	{
	  if (60 > lprice) lprice = 60;
	  return CCL_tarifpart_price_get(id, mins - lprice, FALSE) +
		 CCL_tarifpart_price_get(id, lprice, FALSE);
	}
    }

  return 0;
}

/**
 * Gets the nth price from a tarifpart.
 *
 * @param   id The tarifpart's id.
 * @param   nth The index of the price to get (starting at 0).
 * @param[out] mins The amount of minutes for the price. (can be NULL)
 * @param[out] price The price of that amount of minutes. (can be NULL)
 * @return FALSE if there aren't more prices, TRUE otherwise.
 *
 * Use this to iterate on the prices of a tarifpart, for example when building
 * a list of all the prices on a tarifpart.
 */

gboolean
CCL_tarifpart_price_get_nth(gint id, guint nth, guint * mins, guint * price)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint i;
  gboolean found = FALSE;

  g_return_val_if_fail(CCL_tarifpart_exists(id), FALSE);

  cmd = sqlite3_mprintf("select mins, price from prices\n"
			"where tarifpart = %d\n"
			"order by mins asc;", id);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  for (i = nth; i > 0; i--)
    sqlite3_step(stmt);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    {
      found = TRUE;
      if (mins)
	*mins = sqlite3_column_double(stmt, 0);
      if (price)
	*price = sqlite3_column_double(stmt, 1);
    }
  sqlite3_finalize(stmt);

  return found;
}

/**
 * Gets the price with the amount of minutes nearest to mins (greater or equal)
 *
 * @param   id The tarifpart's id.
 * @param   mins The amount of minutes.
 * @return The amount of minutes of a price one is found, or MAX(mins, perminafter)
 */
guint
CCL_tarifpart_price_get_nearest(gint id, guint mins)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  guint nextmins = 0;

  g_return_val_if_fail(CCL_tarifpart_exists(id), 0);

  if (mins >= ccl->perminafter)
    return mins;

  cmd = sqlite3_mprintf("select mins from prices where mins >= %d\n"
			"order by mins asc;", mins);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  if (sqlite3_step(stmt) == SQLITE_ROW)
    nextmins = sqlite3_column_double(stmt, 0);
  
  sqlite3_finalize(stmt);

  if (!nextmins)
    nextmins = ccl->perminafter;

  return nextmins;
}

/**
 * Checks if a price with mins amount of minutes exists.
 * 
 * @param   id The tarifpart's id.
 * @param   mins The amount of minutes.
 * @return TRUE if exists, FALSE otherwise.
 */
gboolean
CCL_tarifpart_price_exists(gint id, guint mins)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;

  g_return_val_if_fail(CCL_tarifpart_exists(id), FALSE);

  cmd = sqlite3_mprintf("select price from prices\n"
			"where tarifpart = %d and mins = %u;",
			id, mins);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  if (sqlite3_step(stmt) == SQLITE_ROW)
    {
      sqlite3_finalize(stmt);
      return TRUE;
    }
  else
    {
      sqlite3_finalize(stmt);
      return FALSE;
    }
}

/**
 * Gets the price with the maximum amount of minutes.
 *
 * @param   id The tarifpart's id.
 * @return The minutes of the price, or 0 if none exists.
 */
guint
CCL_tarifpart_price_get_last(gint id)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  guint mins = 0;

  g_return_val_if_fail(CCL_tarifpart_exists(id), 0);

  cmd = sqlite3_mprintf("select max(mins) from prices\n"
			"where tarifpart = %d;", id);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  if (sqlite3_step(stmt) == SQLITE_ROW)
    mins = sqlite3_column_double(stmt, 0);
  
  sqlite3_finalize(stmt);

  return mins;
}

/**********************************************************/

void
_tarif_rebuild(gint tarif_id, GSList ** tarif)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  GSList *link = NULL;

  g_assert(tarif_id > 0 && tarif != NULL);
  
  _tarif_clear(tarif_id);
  cmd = sqlite3_mprintf("select id, days, stime, hourprice from tarifs\n"
			"where tarif = %d;", tarif_id);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  while (sqlite3_step(stmt) == SQLITE_ROW)
    {
      CCL_tarifpart *part = NULL;
      guint days = sqlite3_column_double(stmt, 1);
      guint hour = 0;
      guint min = 0;
      guint stime = 0;
      gint i;
      gchar *cmd2 = NULL;
      sqlite3_stmt *stmt2 = NULL;
      GData ** prices = g_malloc0(sizeof(GData *));
      gint id = 0;

      id = sqlite3_column_int(stmt, 0);
      /* Load prices */
      g_datalist_init(prices);
      cmd2 = sqlite3_mprintf("select mins, price from prices\n"
			     "where tarifpart = %d;",
			     id);
      sqlite3_prepare(ccl->db, cmd2, -1, &stmt2, NULL);
      sqlite3_free(cmd2);
      while (sqlite3_step(stmt2) == SQLITE_ROW)
	{
	  gint mins;
	  guint price;

	  mins = sqlite3_column_int(stmt2, 0);
	  price = sqlite3_column_double(stmt2, 1);
	  g_datalist_id_set_data(prices, mins, GINT_TO_POINTER(price));
	}
      sqlite3_finalize(stmt2);

      /* Add tarifpart */
      sscanf((gchar *) sqlite3_column_text(stmt, 2), "%u:%u", &hour, &min);
      stime = hour * 60 + min;

      for (i = 0; i < 7; i++)
	{
	  if ((1 << i) & days)
	    {
	      part = g_new0(CCL_tarifpart, 1);
	      part->stime = stime + i * 24 * 60;
	      part->id = sqlite3_column_int(stmt, 0);
	      part->hprice = sqlite3_column_double(stmt, 3);
	      part->prices = prices;

	      *tarif = g_slist_insert_sorted(*tarif, part, _TarifCompareFunc);
	    }
	}
    }
  sqlite3_finalize(stmt);
  /* Now i am going to remove unnessesary links to clean things */
  link = *tarif;
  while (link)
    {
      GSList *nextlink = g_slist_next(link);
      CCL_tarifpart *part = (CCL_tarifpart *) link->data;

      if (!nextlink)
	link = NULL;
      else if (((CCL_tarifpart *) nextlink->data)->id == part->id)
	{
	  *tarif = g_slist_remove_link(*tarif, nextlink);
	  g_free(nextlink->data);
	  nextlink->data = NULL;
	  g_slist_free_1(nextlink);
	}
      else
	link = nextlink;
    }
  /* If the first link in the list does not start on sunday at 00:00  *
   * insert the last one there, so things don't broke on calculation  */
  if (*tarif && 0 != ((CCL_tarifpart *) (*tarif)->data)->stime)
    {
      CCL_tarifpart *newpart = g_new0(CCL_tarifpart, 1);
      CCL_tarifpart *lastpart = NULL;
      GSList *link = *tarif;
      
      while (link->next)
	link = link->next;
      
      lastpart = (CCL_tarifpart *) link->data;

      newpart->id = lastpart->id;
      newpart->hprice = lastpart->hprice;
      newpart->stime = 0;
      newpart->prices = lastpart->prices;

      *tarif = g_slist_prepend(*tarif, newpart);
    }
  /* add it to the list */
  g_datalist_id_set_data(&ccl->tarifs, tarif_id, *tarif);
}

static void
_tarif_clear(gint tarif_id)
{
  GSList **tarif = NULL;
  GSList *link = NULL;
  CCL_tarifpart *tp = NULL;
  GData *freed = NULL;

  tarif = g_new0(GSList *, 1);
  *tarif = g_datalist_id_get_data(&ccl->tarifs, tarif_id);

  g_assert(NULL != tarif);
  
  g_datalist_init(&freed);

  while ((link = g_slist_last(*tarif)) && *tarif)
    {
      *tarif = g_slist_remove_link(*tarif, link);
      tp = (CCL_tarifpart *) link->data;
      if (!g_datalist_id_get_data(&freed, GPOINTER_TO_INT(tp->prices)))
	{
	  g_datalist_clear(tp->prices);
	  g_datalist_id_set_data(&freed, GPOINTER_TO_INT(tp->prices),
				 (void *)tp->prices);
	  g_free(tp->prices);
	}
      g_slist_free_1(link);
      g_free(tp);
    }
  g_datalist_clear(&freed);
  *tarif = NULL;
  g_datalist_id_remove_data(&ccl->tarifs, tarif_id);
  g_free(tarif);
}

gint
_TarifCompareFunc(gconstpointer a, gconstpointer b)
{
  CCL_tarifpart *t1 = (CCL_tarifpart *) a;
  CCL_tarifpart *t2 = (CCL_tarifpart *) b;

  g_assert(t1 && t2);

  if (t1->stime > t2->stime) return 1;
  else if (t1->stime < t2->stime) return -1;
  else return 0;
}

/* Static */
static time_t
_weekstart(time_t t)
{
  struct tm *ws = NULL;
  time_t weekst = 0;
  time_t weekstart = 0;

  ws = localtime(&t);
  ws->tm_sec = ws->tm_min = ws->tm_hour = 0;
  weekst = mktime(ws);
  weekst -= 24 * 60 * 60 * (ws->tm_wday);
  ws = localtime(&weekst);
  weekstart = mktime(ws);

  return weekstart;
}

static guint
_CCL_tarif_realcalc(GSList * slist, time_t s, time_t e, gboolean permin)
{
  time_t now = time(NULL);	  /* current time */
  time_t et = e - _weekstart(s);  /* interval end time */
  time_t st = s - _weekstart(s);  /* interval start time */
  time_t etime;			  /* this part end time */
  time_t stime;			  /* this part start time */
  CCL_tarifpart *next = NULL;
  CCL_tarifpart *this = NULL;
  GSList *nextlink = g_slist_next(slist);
  
  if (slist)
    this = slist->data;
  else
    return 0;

  if (nextlink)
    next = (CCL_tarifpart *) nextlink->data;

  if (et == st)
    et++;			/* To make total time > 0 */

  if (s > now || e > now)
    g_warning("[!]:The given times are in the future!");

  stime = this->stime;

  if (!next)
    etime = 60 * 60 * 24 * 7;
  else
    etime = 60 * next->stime;	/* If et > etime, it belongs
				 * to the next */
  /* All inside */
  if (st >= stime && st < etime && et >= stime && et <= etime)
    {
      if (!permin)
	return CCL_tarifpart_price_get(this->id, (et - st) / 60, FALSE);
      else
	return ((this->hprice * (et - st)) / 3600);
    }
  else if (next && st >= etime)	/* All outside */
    return _CCL_tarif_realcalc(nextlink, s, e, permin);
  else if (et > etime)
    {				/* A part inside */
      if (!next)		/* On next week */
	nextlink = ccl->tarif;	/* then make next, the start of the week */
      else
	nextlink = g_slist_next(slist);

      next = (CCL_tarifpart *) nextlink->data;

      if (permin)		/* calculate it for each min? */
	{
	  time_t end = _weekstart(s) + etime; 

	  return _CCL_tarif_realcalc(slist, s, end, permin) +
		 _CCL_tarif_realcalc(nextlink, end, e, permin);
	}
      else
	{
	  gdouble frac1;
	  gdouble frac2;
	  guint minutes = CCL_tarifpart_price_get_nearest(this->id,
							  (et - st) / 60);
	  time_t et = st + minutes * 60;

	  frac1 = (etime - st) / (gdouble) (et - st);
	  frac2 = (et - etime) / (gdouble) (et - st);

	  return (guint) (CCL_tarifpart_price_get
			  (this->id, (et - st) / 60, FALSE) * frac1 +
			  CCL_tarifpart_price_get
			  (next->id, (et - st) / 60, FALSE) * frac2);
	}
    }
  else
    {				/* Something is wrong */
      g_message("[E]:There is something wrong with the tarif\n");
      g_message
	("stime:%ld,st:%ld,etime:%ld,et:%ld,s:%ld,e:%ld,now:%ld ws:%ld\n",
	 stime, st, etime, et, s, e, now, _weekstart(s));
      g_assert_not_reached();

      return 0;
    }
}
