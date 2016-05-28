#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ccls.h"
#include "ccl_private.h"

extern CCL *ccl;
/*#define DEBUG 1*/
/* Public interface */

/**
 * Sets key entry to a string value.
 *
 * @param   cid What the id represents (CCL_DATA_NONE, CCL_DATA_CLIENT, ...)
 * @param   id The id for the value.
 * @param   key The key that will contain the value.
 * @param   value The value.
 */
void
CCL_data_set_string(gint cid, gint id, const gchar * key, const gchar * value)
{
  gchar *cmd = NULL;
  int retval;
  char *errmsg;

#ifdef DEBUG
  printf("CCL_data_set_string(): key (%s) <=> value (%s)\n", key, value);
#endif
  g_return_if_fail(key);
  
  cmd = sqlite3_mprintf("insert or replace into tbldata \n"
			"(cid, id, key, value)\n"
			"values(%d, %d, %Q, %Q);", cid, id, key, value);

  /*retval = sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);*/
  retval = sqlite3_exec(ccl->db, cmd, NULL, NULL, &errmsg);
#ifdef DEBUG
  printf("CCL_data_set_string(): retval[%d] cmd = %s \n", retval, cmd);
  if (retval>0){
    printf("CCL_data_set_string(): ** Error [%s]\n", errmsg);
    sqlite3_free(errmsg);
  }
#endif
  sqlite3_free(cmd);
}

/**
 * Gets the value of a key as a string.
 *
 * @param   cid What the id represents (CCL_DATA_NONE, CCL_DATA_CLIENT, ...)
 * @param   id The id for the value.
 * @param   key The key to get the value of.
 * @param   defvalue The value returned if the key is not found.
 * @return The value of the key if it exists, defvalue otherwise.
 *
 * The returned value must be freed with CCL_free to avoid memory leaks.
 */
gchar *
CCL_data_get_string(gint cid, gint id, const char * key, const char * defval)
{
  gchar *cmd = NULL;
  gchar *ret = NULL;
  sqlite3_stmt *stmt = NULL;

  g_return_val_if_fail(key, NULL);

  cmd = sqlite3_mprintf("select value from tbldata\n"
			"where cid = %d and id = %d and key = %Q;",
			cid, id, key);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    ret = g_strdup((gchar *) sqlite3_column_text(stmt, 0));

  sqlite3_finalize(stmt);

  if (!ret && defval)
    return g_strdup(defval);
  else
    return ret;
}

/**
 * Sets key entry to an int value.
 *
 * @param   cid What the id represents (CCL_DATA_NONE, CCL_DATA_CLIENT, ...)
 * @param   id The id for the value.
 * @param   key The key that will contain the value.
 * @param   value The value.
 */
void
CCL_data_set_int(gint cid, gint id, const gchar * key, gint value)
{
  gchar *cmd = NULL;
  
  g_return_if_fail(key);

  cmd = sqlite3_mprintf("insert or replace into tbldata\n"
			"(cid, id, key, value)\n"
			"values(%d, %d, %Q, %d);", cid, id, key, value);

  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Gets the value of a key as an int.
 *
 * @param   cid What the id represents (CCL_DATA_NONE, CCL_DATA_CLIENT, ...)
 * @param   id The id for the value.
 * @param   key The key to get the value of.
 * @param   defvalue The value returned if the key is not found.
 * @return The value of the key if it exists, defval otherwise.
 */
gint
CCL_data_get_int(gint cid, gint id, const gchar * key, gint defval)
{
  gchar *cmd = NULL;
  gint ret = defval;
  sqlite3_stmt *stmt = NULL;

  g_return_val_if_fail(key, -1);

  cmd = sqlite3_mprintf("select value from tbldata\n"
			"where cid = %d and id = %d and key = %Q;",
			cid, id, key);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    ret = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);

  return ret;
}

/**
 * Sets key entry to a blob value.
 * 
 * @param   cid What the id represents (CCL_DATA_NONE, CCL_DATA_CLIENT, ...)
 * @param   id The id for the value.
 * @param   key The key that will contain the value.
 * @param   value The value.
 * @param   size The size of the blob data.
 */
void
CCL_data_set_blob(gint cid, gint id, const gchar * key, void * value,
		  gint size)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  
  g_return_if_fail(key);

  cmd = sqlite3_mprintf("insert or replace into tbldata\n"
			"(cid, id, key, value)\n"
			"values(%d, %d, %Q, ?1);", cid, id, key);

  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  sqlite3_bind_blob(stmt, 1, value, size, SQLITE_TRANSIENT);
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);
}

/**
 * Gets the value of a key as a blob.
 *
 * @param   cid What the id represents (CCL_DATA_NONE, CCL_DATA_CLIENT, ...)
 * @param   id The id for the value.
 * @param   key The key to get the value of.
 * @param[out] size The size of the returned data.
 * @return The value of the key if it exists, NULL otherwise.
 *
 * The returned value must be freed with CCL_free to avoid memory leaks.
 */
void *
CCL_data_get_blob(gint cid, gint id, const gchar * key, int * size)
{
  gchar *cmd = NULL;
  void *ret = NULL;
  sqlite3_stmt *stmt = NULL;

  g_return_val_if_fail(key, NULL);
  g_return_val_if_fail(size, NULL);

  cmd = sqlite3_mprintf("select value from tbldata\n"
			"where cid = %d and id = %d and key = %Q;",
			cid, id, key);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    {
      *size = sqlite3_column_bytes(stmt, 0);
      ret = g_malloc0(*size);
      memcpy(ret, sqlite3_column_blob(stmt, 0), *size);
    }

  sqlite3_finalize(stmt);

  return ret;
}

/**
 * Checks if a given key exists.
 *
 * @param   cid What the id represents (CCL_DATA_NONE, CCL_DATA_CLIENT, ...)
 * @param   id The id for the value.
 * @param   key The key to check.
 * @return TRUE if it exists, FALSE if not.
 */
gboolean
CCL_data_key_exists(gint cid, gint id, const gchar * key)
{
  gchar *cmd = NULL;
  gboolean ret = FALSE;
  sqlite3_stmt *stmt = NULL;

  g_return_val_if_fail(key, FALSE);

  cmd = sqlite3_mprintf("select value from tbldata\n"
			"where cid = %d and id = %d and key = %Q;",
			cid, id, key);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    ret = TRUE;

  sqlite3_finalize(stmt);

  return ret;
}

/**
 * Deletes a key.
 *
 * @param   cid What the id represents (CCL_DATA_NONE, CCL_DATA_CLIENT, ...)
 * @param   id The id for the value.
 * @param   key The key to be deleted
 */
void
CCL_data_key_delete(gint cid, gint id, const gchar * key)
{
  gchar *cmd = NULL;
  gchar *errstr = NULL; 
  int retval;

  g_return_if_fail(key);

  cmd = sqlite3_mprintf("delete from tbldata\n"
			"where cid = %d and id = %d and key = %Q;",
			cid, id, key);
  retval = sqlite3_exec(ccl->db, cmd, NULL, NULL, &errstr);
#ifdef DEBUG
  printf("CCL_data_key_delete(): retval[%d] cmd: %s\n",  
	 retval, cmd);
  if (retval > 0){
    printf("CCL_data_key_delete(): retval[%d] Error: %s\n", 
	   retval, errstr);
    sqlite3_free(errstr);
  }
#endif
  sqlite3_free(cmd);
}

/**
 * Finds the data with a key value equal to value (string version).
 * 
 * @param   cid What the id represents (CCL_DATA_NONE, CCL_DATA_CLIENT, ...)
 * @param   key The key to compare with.
 * @param   value The value to match with.
 * @return The data's id, or -1 if none found.
 */
gint
CCL_data_find_by_key_sval(gint cid, const gchar * key, const gchar * value)
{
  gchar *cmd = NULL;
  gint id = -1;
  sqlite3_stmt *stmt = NULL;

  g_return_val_if_fail(key, -1);

  cmd = sqlite3_mprintf("select id from tbldata \n"
			"where cid = %d and key = %Q and value = %Q;",
			cid, key, value);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  if (sqlite3_step(stmt) == SQLITE_ROW)
    id = sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);
#ifdef DEBUG
  /*  {
    sqlite3_stmt *stmt1 = NULL;
    gchar *cmd1 = NULL;
    cid = -11;
    cmd1 = sqlite3_mprintf("select id from tbldata\n"
			   "where cid > %d;",
			   cid);
    sqlite3_prepare(ccl->db, cmd1, -1, &stmt1, NULL);
    while (sqlite3_step(stmt1) == SQLITE_ROW){
      char *txt;
      
      id = sqlite3_column_int(stmt1,0);
      if (id>=0){
	txt = sqlite3_column_text(stmt1,1);
	txt = g_strdup((gchar *) sqlite3_column_text(stmt1, 0));
      }
      printf("**** CCL_find_by_key_sval(): id[%2d]\n", id);
    }
    sqlite3_finalize(stmt1);
    sqlite3_free(cmd1);
    }*/
  printf("** CCL_find_by_key_sval(): retval[%d] cmd=%s\n", id, cmd);
#endif
  sqlite3_free(cmd);

  return id;
}

/**
 * Finds the data with a key value equal to value (int version).
 * 
 * @param   cid What the id represents (CCL_DATA_NONE, CCL_DATA_CLIENT, ...)
 * @param   key The key to compare with.
 * @param   value The value to match with.
 * @return The data's id, or -1 if none found.
 */
gint
CCL_data_find_by_key_ival(gint cid, const gchar * key, gint value)
{
  gchar *cmd = NULL;
  gint id = -1;
  sqlite3_stmt *stmt = NULL;

  g_return_val_if_fail(key, -1);

  cmd = sqlite3_mprintf("select id from tbldata\n"
			"where cid = %d and key = %Q and value = %d;",
			cid, key, value);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    id = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);

  return id;
}
