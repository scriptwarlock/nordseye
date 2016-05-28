#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ccls.h"
#include "ccl_private.h"
/*
#define DEBUG 1
*/
extern CCL *ccl;


/* Public interface */

/**
 * Create a new member.
 *
 * @param   name The member's name.
 * @return The new member's id.
 *
 * If a member with this name already exists, his id will be returned.
 */
gint
CCL_member_new(const gchar * name, int emp)
{
  gchar *cmd = NULL;
  gint id;
  gchar *errstr = NULL;

  id = CCL_member_find(name);

  if (-1 == id)
    {
      CCL_member *member = g_new0(CCL_member, 1);

      _CCL_member_init(member, name);
      cmd = sqlite3_mprintf("insert into members\n"
			    "(name, sdate, empid, flags) values(%Q, %ld, %d, %d);",
			    name, time(NULL),  emp, 0);
      sqlite3_exec(ccl->db, cmd, NULL, NULL, &errstr);
      sqlite3_free(cmd);
      id = sqlite3_last_insert_rowid(ccl->db);
      g_datalist_id_set_data_full(&(ccl->members), id, member,
				  _destroy_member);

#ifdef DEBUG
      printf("CCL_member_new(): cmd = %s\nerrstr = %s\n", cmd, errstr);
#endif
      sqlite3_free(errstr);
    }

  return id;
}


/* Static functions */
static gboolean _CCL_member_store(gint member);

/* Public interface */

/**
 * Create a new ticket
 *
 * @param 1 id [ not used here ]
 * @param 2 sql string 
 * @return id of inserted record , or error 
 *
 */

gint
CCL_member_ticket_new(int id, char *sqlstr)
{
  gchar *cmd = NULL;
  gchar *errstr = NULL;

  /*  if (-1 == id)  */
    {
      cmd = sqlite3_mprintf("%s",sqlstr);
      sqlite3_exec(ccl->db, cmd, NULL, NULL, &errstr);
      sqlite3_free(cmd);
      id = sqlite3_last_insert_rowid(ccl->db);
#ifdef DEBUG 
      printf("CCL_member_ticket_new(): cmd = %s\nerrstr = %s\n", cmd, errstr);
#endif 
      sqlite3_free(errstr);
    }  
  return id;
}

/**
 * delete a ticket 
 *
 * @param 1 id [ not used here ]
 * @param 2 sql string 
 * @return 0 
 *
 */
gint
CCL_member_ticket_del(int id, char *sqlstr)
{
  gchar *cmd = NULL;
  gchar *errstr = NULL;

  if (-1 == id)
    {
      cmd = sqlite3_mprintf("%s",sqlstr);
      sqlite3_exec(ccl->db, cmd, NULL, NULL, &errstr);
      sqlite3_free(cmd);
#ifdef DEBUG
      printf("CCL_member_ticket_del(): cmd = %s\nerrstr = %s\n", cmd, errstr);
#endif
      sqlite3_free(errstr);
    }

  return 0;
}


/**
 * Find a ticket by name.
 *
 * @param   name The name of the member we are looking for.
 * @return The ticket's id if found, -1 if not found.
 */
gint
CCL_member_ticket_find(const gchar * name)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint id = -1;
 
  cmd = sqlite3_mprintf("select id from tickets where name = %Q;", name);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    id = sqlite3_column_int(stmt, 0);
  
  sqlite3_finalize(stmt);

  return id;
}


/**
 * query ticket info
 *
 * @param 1 sql string
 * @param container for ticket data : this is allocated INSIDE the function
 * @return Number of records if te is null, or the record in te
 *
 */

gint
CCL_member_tickets_get(char *sqlstr, CCL_ticket_entry **te)
{
  char    *cmd = NULL;
  char   **argv = NULL;
  int      nrow, ncol;
  CCL_ticket_entry *entryarray = NULL;

  cmd = sqlite3_mprintf("%s", sqlstr);
 
  if (sqlite3_get_table(ccl->db, cmd, &argv, &nrow, &ncol, NULL) == SQLITE_OK
      && argv && nrow > 0 && ncol)
    {
      int i;
      
      if (!te)	      /* If !se, return the number of found entries */
	{
	  sqlite3_free(cmd);
	  sqlite3_free_table(argv);
	  return nrow;
	}
      /* Otherwise, lets fill the array with found entries */
      entryarray = g_new0(CCL_ticket_entry, nrow);
      
      for (i = 0; i < nrow; i++){
	gint os = (i+1) * ncol; /* offset */
	
	sscanf(argv[os], "%d", &(entryarray[i].id));
	sscanf(argv[os+1], "%s", entryarray[i].name);
	sscanf(argv[os+2], "%ld", &(entryarray[i].pdate));
	sscanf(argv[os+4], "%ld", &(entryarray[i].stdate));
	sscanf(argv[os+5], "%ld", &(entryarray[i].enddate));
	sscanf(argv[os+7], "%d", &(entryarray[i].faceval));
	sscanf(argv[os+8], "%d", &(entryarray[i].credit));
	sscanf(argv[os+9], "%d", &(entryarray[i].flags));
      }
    }
  sqlite3_free(cmd);
  sqlite3_free_table(argv);
  *te = entryarray;
#ifdef DEBUG
  printf("CCL_member_tickets_get(): cmd = %s\nerrstr = %s\n", cmd, errstr);
#endif
  
  return nrow;
}


/**
 * Gets the nth member on the list.
 *
 * @param   nth The index of the member to get (starting at 0).
 * @return The member's id, or -1 if there aren't more members.
 *
 * Use this to iterate on the members, for example when building a list of
 * all the members.
 */
gint
CCL_member_get_nth(guint nth)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint i;
  gint member = -1;

  cmd = sqlite3_mprintf("select id from members order by id asc;");
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  for (i = nth; i > 0; i--)
    sqlite3_step(stmt);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    member = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);
  
  return member;
}

/**
 * Checks if a member already exists.
 *
 * @param   member The member's id.
 * @return TRUE if the member exists, FALSE if not.
 */
gboolean
CCL_member_exists(gint member)
{
  if (g_datalist_id_get_data(&(ccl->members), member))
    return TRUE;
  else
    return FALSE;
}

/**
 * Find a member by name.
 *
 * @param   name The name of the member we are looking for.
 * @return The member's id if found, -1 if not found.
 */
gint
CCL_member_find(const gchar * name)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint id = -1;
 
  cmd = sqlite3_mprintf("select id from members where name = %Q;", name);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    id = sqlite3_column_int(stmt, 0);
  
  sqlite3_finalize(stmt);

  return id;
}

/**
 * Gets the name of a member.
 *
 * @param   member The member's id.
 * @return The name of the member, or NULL if that member doesn't exist.
 */
const gchar *
CCL_member_name_get(gint member)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);

  g_return_val_if_fail(m, NULL);
  
  return m->name;
}

/**
 * Sets the name of a member.
 *
 * @param   member The member's id.
 * @param   name The new name.
 */
void
CCL_member_name_set(gint member, const gchar * name)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);

  g_return_if_fail(m);
  g_return_if_fail(-1 == CCL_member_find(name));
  g_return_if_fail(strlen(name) > 0);

  g_free(m->name);
  m->name = g_strdup(name);
  _CCL_member_store(member);
}

/**
 * Gets the tarif of a member.
 *
 * @param   member The member's id.
 * @return The tarif of the member, 0 if none set.
 */
gint
CCL_member_tarif_get(gint member)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);

  g_return_val_if_fail(m, 0);
  
  return m->tarif;
}

/**
 * Sets the tarif of a member.
 *
 * @param   member The member's id.
 * @param   tarif The tarif to use, 0 to use the default.
 */
void
CCL_member_tarif_set(gint member, gint tarif)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);

  g_return_if_fail(m && (CCL_tarif_exists(tarif) || 0 == tarif));

  m->tarif = tarif;
  _CCL_member_store(member);
}

/**
 * Gets the other value of a member.
 *
 * @param   member The member's id.
 * @return The member's other value.
 */
const gchar *
CCL_member_other_get(gint member)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);

  g_return_val_if_fail(m, NULL);
  
  return m->other;
}

/**
 * Sets the other value of a member.
 *
 * @param   member The member's id.
 * @param   other The other value.
 */
void
CCL_member_other_set(gint member, const gchar * other)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);

  g_return_if_fail(m);

  if (m->other) g_free(m->other);
  m->other = g_strdup(other);
  _CCL_member_store(member);
}

/**
 * Gets the e-mail of a member.
 *
 * @param   member The member's id.
 * @return The member's e-mail.
 */
const gchar *
CCL_member_email_get(gint member)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);

  g_return_val_if_fail(m, NULL);

  return m->email;
}

/**
 * Sets the email of a member.
 *
 * @param   member The member's id.
 * @param   tarif The e-mail.
 */
void
CCL_member_email_set(gint member, const gchar * email)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);

  g_return_if_fail(m);
      
  if (m->email) g_free(m->email);
  m->email = g_strdup(email);
  _CCL_member_store(member);
}

/**
 * Sets member's associated data pointer.
 *
 * @param   member The member's id.
 * @param   data The pointer to the data.
 * @return The old data pointer.
 */
gpointer
CCL_member_data_set(gint member, gpointer data)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);
  gpointer olddata = NULL;

  g_return_val_if_fail(m, NULL);
      
  olddata = m->data; 
  m->data = data;
      
  return olddata;
}

/**
 * Gets a pointer to the member's associated data.
 *
 * @param   member The member's id.
 * @return The pointer to the data.
 */
gpointer
CCL_member_data_get(gint member)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);

  g_return_val_if_fail(m, NULL);

  return m->data;
}

/**
 * Sets member flags to the value of flags.
 *
 * @param   member The member's id.
 * @param   flags The new flags value.
 */
void
CCL_member_flags_set(gint member, gint flags)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);

  g_return_if_fail(m);

  m->flags = flags;
  _CCL_member_store(member);
}

/**
 * Gets the flags value of member.
 *
 * @param   member The member's id.
 * @return The flags value.
 */
gint
CCL_member_flags_get(gint member)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);

  g_return_val_if_fail(m, 0);

  return m->flags;
}

/**
 * Toggle flags on member on or off.
 *
 * @param   member The member's id.
 * @param   flags The flags to toggle.
 * @param   on If TRUE toggle the flags on, else toggle them off.
 */
void
CCL_member_flags_toggle(gint member, gint flags, gboolean on)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);

  g_return_if_fail(m);
      
  if (on)
    m->flags |= flags;
  else
    m->flags &= ~flags;

  _CCL_member_store(member);
}


/**
 * Gets the credit of a member.
 *
 * @param   member The member's id.
 * @return The amount of credit
 */
int
CCL_member_credit_get(gint member)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);

  g_return_val_if_fail(m, -1);
  
  return m->credit;
}

/**
 * Sets the credit of a member.
 *
 * @param   member The member's id.
 * @param   credit The new credit.
 */
void
CCL_member_credit_set(gint member, int credit)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);

  g_return_if_fail(m);

  m->credit = credit;
  _CCL_member_store(member);
}


/**********************************************************/

gboolean
_CCL_member_restore(gint member)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  int retval=0;

  g_return_val_if_fail(m, FALSE);
      
  cmd = sqlite3_mprintf("select name, tarif, email, other, credit, flags \n"
			"from members where id = %d;", member);
  retval = sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  /*  retval = sqlite3_exec(ccl->db, cmd, NULL, NULL, &errstr);*/
#ifdef DEBUG
  {
    gchar *errstr = NULL;
    printf("_CCL_member_restore(): cmd=%s\n, [retval=%X] %s\n", cmd, 
	   retval, errstr);
  }
#endif
  sqlite3_free(cmd);
  if (sqlite3_step(stmt) == SQLITE_ROW)
    {
      if (m->name) g_free(m->name);
      if (m->email) g_free(m->email);
      if (m->other) g_free(m->other);
      
      m->name = g_strdup((gchar *)sqlite3_column_text(stmt, 0));
      m->tarif = sqlite3_column_int(stmt, 1);
      m->email = g_strdup((gchar *)sqlite3_column_text(stmt, 2));
      m->other = g_strdup((gchar *)sqlite3_column_text(stmt, 3));
      m->credit = sqlite3_column_int(stmt, 4);
      m->flags = sqlite3_column_int(stmt, 5);
    }

  sqlite3_finalize(stmt);

  return TRUE;
}

/* Static */
static gboolean
_CCL_member_store(gint member)
{
  CCL_member *m = g_datalist_id_get_data(&(ccl->members), member);
  gchar *cmd = NULL, *errstr = NULL;

  g_return_val_if_fail(m, FALSE);
      
  cmd = sqlite3_mprintf("update members\n"
			"set tarif = %d,\n"
			"    name = %Q,\n"
			"    email = %Q,\n"
			"    other = %Q,\n"
			"    credit = %d,\n"
			"    flags = %d\n"
			"where id = %d;", m->tarif, m->name, m->email,
			m->other, (unsigned int)m->credit, m->flags, member);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, &errstr);
#ifdef DEBUG
  printf("_CCL_member_store(): cmd = %s\nerrstr = %s\n", cmd, errstr);
#endif
  sqlite3_free(cmd);
  sqlite3_free(errstr);

  return TRUE;
}

void
_destroy_member(gpointer data)
{
  CCL_member *member = (CCL_member *) data;

  if (member->name) g_free(member->name);
  if (member->email) g_free(member->email);
  if (member->other) g_free(member->other);

  g_free(member);
}

void
_CCL_member_init(CCL_member * member, const gchar * name)
{
  member->name = g_strdup(name);
  member->tarif = 0;
  member->email = NULL;
  member->other = NULL;
  member->credit = 0.0;
  member->flags = 0;
  member->data = NULL;
}
