#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ccls.h"
#include "ccl_private.h"

/*
#define DEBUG_LIBCCL = 1
#define DEBUG = 1
*/
extern CCL *ccl;

/* Static functions */
static gboolean _CCL_employee_store(gint employee);

/* Public interface */

/**
 * Create a new employee.
 *
 * @param   name The employee's name.
 * @return The new employee's id.
 *
 * If an employee with this name already exists, his id will be returned.
 */
gint
CCL_employee_new(char *usr, char *name, char *pwd, char *phone, 
		 char *email, int lvl, int superid)
{
  gchar *cmd = NULL, *errstr=NULL;
  gint id;

  id = CCL_employee_find(usr);
  if (-1 == id)
    {
      CCL_employee *employee = g_new0(CCL_employee, 1);
      _CCL_employee_init(employee, usr, name, pwd, phone, email, lvl, superid);
      cmd = sqlite3_mprintf("insert into employees\n"
			    "(empusr, empname, emppass, phone, \n"
			    "email, emplevel, hiredate, superid, flags) \n "
			    "values(%Q, %Q, %Q, %Q, %Q, %ld, %ld, %d, %d);",
			    usr, name, pwd, phone, email,
			    lvl, time(NULL), superid, 0);
#ifdef DEBUG_LIBCCL
      printf ("CCL_employee_new: cmd = %s\n", cmd);
#endif
      sqlite3_exec(ccl->db, cmd, NULL, NULL, &errstr);
      sqlite3_free(cmd);
      id = sqlite3_last_insert_rowid(ccl->db);
      g_datalist_id_set_data_full(&(ccl->employees), id, employee,
				  _destroy_employee);
    }
  else{
    cmd = sqlite3_mprintf("update employees \n"
			  "set emplevel = %d,\n"
			  "    empname = %Q,\n"
			  "    emppass = %Q,\n"
			  "    email = %Q,\n"
			  "    phone = %Q,\n"
			  "    hiredate = %ld,\n"
			  "    superid = %d,\n"
			  "    flags = %ld\n"
			  "where id = %d;", lvl, name, pwd, email,
			  phone, time(NULL), superid, 0, id);
#ifdef DEBUG_LIBCCL
    printf ("CCL_employee_new: cmd = %s\n", cmd);
#endif
    sqlite3_exec(ccl->db, cmd, NULL, NULL, &errstr);
    sqlite3_free(cmd);
  }

  return id;
}

/**
 * Gets the nth employee on the list.
 *
 * @param   nth The index of the employee to get (starting at 0).
 * @return The employee's id, or -1 if there aren't more employees.
 *
 * Use this to iterate on the employees, for example when building a list of
 * all the employees.
 */
gint
CCL_employee_get_nth(guint nth)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint i;
  gint employee = -1;

  cmd = sqlite3_mprintf("select id from employees order by id asc;");
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  for (i = nth; i > 0; i--)
    sqlite3_step(stmt);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    employee = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);
  
  return employee;
}

/**
 * Checks if a employee already exists.
 *
 * @param   employee The employee's id.
 * @return TRUE if the employee exists, FALSE if not.
 */
gboolean
CCL_employee_exists(int employee)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint id = -1, retval; 
 
  cmd = sqlite3_mprintf("select id from employees where id = %d;", employee);
  retval = sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  if (sqlite3_step(stmt) == SQLITE_ROW)
    id = sqlite3_column_int(stmt, 0);
  
  sqlite3_finalize(stmt);
#ifdef DEBUG
  printf("**CCL_employee_exists(): id=%d, retval=%d\n", id, retval);
  printf("**CCL_employee_exists(): cmd = %s\n", cmd);
#endif
  sqlite3_free(cmd);

  if (id <= 0) 
    return FALSE;
  else
    return TRUE;

  /*if (g_datalist_id_get_data(&(ccl->employees), employee))
   return TRUE;
  else
    return FALSE;*/
}

/**
 * Find an employee by user name.
 *
 * @param   name The name of the employee we are looking for.
 * @return The employee's id if found, -1 if not found.
 */
gint
CCL_employee_find(const gchar * usrname)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint id = -1;
 
  cmd = sqlite3_mprintf("select id from employees where empusr = %Q;", usrname);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    id = sqlite3_column_int(stmt, 0);
  
  sqlite3_finalize(stmt);

  return id;
}

/**
 * Find an employee with the password.
 *
 * @param   name The name of the employee we are looking for.
 * @param   pwd The password of the employee we are looking for.
 * @return >1 if exists, 0 if not valid
 */
int
CCL_employee_validate(const gchar * usrname, const gchar * pwd)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint valid = 0;
 
  /*_simple_encrypt((char *)pwd, strlen(pwd));*/
  cmd = sqlite3_mprintf("select id from employees "
			"where empusr=%Q and emppass=%Q and flags=0;", 
			usrname, pwd);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    valid = sqlite3_column_int(stmt, 0);
  
  sqlite3_finalize(stmt);
#ifdef DEBUG_LIBCCL
  printf("Command: %s -> %d\n", cmd, valid);
#endif

  return valid;
}

/**
 * Gets the user name of an employee.
 *
 * @param   employee The employee's id.
 * @return The name of the employee, or NULL if that employee doesn't exist.
 */
const gchar *
CCL_employee_usrname_get(gint id)
{
  CCL_employee *e = g_datalist_id_get_data(&(ccl->employees), id);

  g_return_val_if_fail(e, NULL);
  
  return e->usrname;
}

/**
 * Sets the name of a employee.
 *
 * @param   employee The employee's id.
 * @param   name The new name.
 */
void
CCL_employee_usrname_set(gint employee, const gchar * usrname)
{
  CCL_employee *m = g_datalist_id_get_data(&(ccl->employees), employee);

  g_return_if_fail(m);
  g_return_if_fail(-1 == CCL_employee_find(usrname));
  g_return_if_fail(strlen(usrname) > 0);

  g_free(m->usrname);
  m->name = g_strdup(usrname);
  _CCL_employee_store(employee);
}

/**
 * Gets the user level of an employee.
 *
 * @param   employee The employee's id.
 * @return The user level of the employee, 0 if none set.
 */
gint
CCL_employee_usrlvl_get(gint employee)
{
  CCL_employee *m = g_datalist_id_get_data(&(ccl->employees), employee);

  g_return_val_if_fail(m, 0);
  
  return m->usrlvl;
}

/**
 * Sets the tarif of a employee.
 *
 * @param   employee The employee's id.
 * @param   usrlvl The user level to use, 0 to use the default.
 */
void
CCL_employee_usrlvl_set(gint employee, gint usrlvl)
{
  CCL_employee *m = g_datalist_id_get_data(&(ccl->employees), employee);

  g_return_if_fail(m && ((usrlvl>=0) || 0 == usrlvl));

  m->usrlvl = usrlvl;
  _CCL_employee_store(employee);
}

/*
 * Gets the email value of a employee.
 *
 * @param   employee The employee's id.
 * @return The employee's other value.
 */
const gchar *
CCL_employee_phone_get(gint employee)
{
  CCL_employee *e = g_datalist_id_get_data(&(ccl->employees), employee);

  g_return_val_if_fail(e, NULL);
  
  return e->phone;
}

/**
 * Sets the phone of a employee.
 *
 * @param   employee The employee's id.
 * @param   The phone.
 */
void
CCL_employee_phone_set(gint employee, const gchar * phone)
{
  CCL_employee *m = g_datalist_id_get_data(&(ccl->employees), employee);

  g_return_if_fail(m);
      
  if (m->phone) g_free(m->phone);
  m->phone = g_strdup(phone);
  _CCL_employee_store(employee);
}


/**
 * Gets the password value of a employee.
 *
 * @param   employee The employee's id.
 * @return The employee's other value.
 */
const gchar *
CCL_employee_password_get(gint employee)
{
  CCL_employee *m = g_datalist_id_get_data(&(ccl->employees), employee);

  g_return_val_if_fail(m, NULL);
  
  return m->passwd;
}


/**
 * Sets the password of a employee.
 *
 * @param   employee The employee's id.
 * @param   name The new name.
 */
void
CCL_employee_password_set(gint employee, const gchar * pwd)
{
  CCL_employee *m = g_datalist_id_get_data(&(ccl->employees), employee);

  g_return_if_fail(m);
  g_return_if_fail(strlen(pwd) > 0);

  g_free(m->passwd);
  m->passwd = g_strdup(pwd);
  _CCL_employee_store(employee);
}

/*
 * Gets the e-mail of a employee.
 *
 * @param   employee The employee's id.
 * @return The employee's e-mail.
 */
const gchar *
CCL_employee_email_get(gint employee)
{
  CCL_employee *m = g_datalist_id_get_data(&(ccl->employees), employee);

  g_return_val_if_fail(m, NULL);

  return m->email;
}

/**
 * Sets the email of a employee.
 *
 * @param   employee The employee's id.
 * @param   tarif The e-mail.
 */
void
CCL_employee_email_set(gint employee, const gchar * email)
{
  CCL_employee *m = g_datalist_id_get_data(&(ccl->employees), employee);

  g_return_if_fail(m);
      
  if (m->email) g_free(m->email);
  m->email = g_strdup(email);
  _CCL_employee_store(employee);
}

/* 
 * Gets the name of a employee.
 *
 * @param   employee The employee's id.
 * @return The employee's name
 */
const gchar *
CCL_employee_name_get(gint employee)
{
  CCL_employee *e = g_datalist_id_get_data(&(ccl->employees), employee);

  g_return_val_if_fail(e, NULL);

  return e->name;
}

/**
 * Sets the name of a employee.
 *
 * @param   employee The employee's id.
 * @param   The name.
 */
void
CCL_employee_name_set(gint employee, const gchar * name)
{
  CCL_employee *e = g_datalist_id_get_data(&(ccl->employees), employee);

  g_return_if_fail(e);
      
  if (e->name) g_free(e->name);
  e->name = g_strdup(name);
  _CCL_employee_store(employee);
}

/**
 * Sets employee's associated data pointer.
 *
 * @param   employee The employee's id.
 * @param   data The pointer to the data.
 * @return The old data pointer.
 */
gpointer
CCL_employee_data_set(gint employee, gpointer data)
{
  CCL_employee *m = g_datalist_id_get_data(&(ccl->employees), employee);
  gpointer olddata = NULL;

  g_return_val_if_fail(m, NULL);
      
  olddata = m->data; 
  m->data = data;
      
  return olddata;
}

/**
 * Gets a pointer to the employee's associated data.
 *
 * @param   employee The employee's id.
 * @return The pointer to the data.
 */
gpointer
CCL_employee_data_get(gint employee)
{
  CCL_employee *m = g_datalist_id_get_data(&(ccl->employees), employee);

  g_return_val_if_fail(m, NULL);

  return m->data;
}

/**
 * Sets employee flags to the value of flags.
 *
 * @param   employee The employee's id.
 * @param   flags The new flags value.
 */
void
CCL_employee_flags_set(gint employee, gint flags)
{
  CCL_employee *m = g_datalist_id_get_data(&(ccl->employees), employee);

  g_return_if_fail(m);

  m->flags = flags;
  _CCL_employee_store(employee);
}

/**
 * Gets the flags value of employee.
 *
 * @param   employee The employee's id.
 * @return The flags value.
 */
gint
CCL_employee_flags_get(gint employee)
{
  CCL_employee *m = g_datalist_id_get_data(&(ccl->employees), employee);

  g_return_val_if_fail(m, 0);

  return m->flags;
}

/**
 * Toggle flags on employee on or off.
 *
 * @param   employee The employee's id.
 * @param   flags The flags to toggle.
 * @param   on If TRUE toggle the flags on, else toggle them off.d
 */
void
CCL_employee_flags_toggle(gint employee, gint flags, gboolean on)
{
  CCL_employee *m = g_datalist_id_get_data(&(ccl->employees), employee);

  g_return_if_fail(m);
      

#ifdef DEBUG_LIBCCL
  printf("CCL_employee_flags_toggle()-post: %s: Flags: %08X\n", m->usrname, m->flags);
#endif
  if (on)
    m->flags |= flags;
  else
    m->flags &= ~flags;

#ifdef DEBUG_LIBCCL
  printf("CCL_employee_flags_toggle()-post: %s: Flags: %08X\n", m->usrname, m->flags);
#endif

  _CCL_employee_store(employee);
}

/**********************************************************/

gboolean
_CCL_employee_restore(gint employee)
{
  CCL_employee *m = g_datalist_id_get_data(&(ccl->employees), employee);
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;

  g_return_val_if_fail(m, FALSE);
      
  cmd = sqlite3_mprintf("select empusr, emppass, empname, emplevel, phone, email, flags \n"
			"from employees where id = %d;", employee);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);
  
  if (sqlite3_step(stmt) == SQLITE_ROW)
    {
      if (m->usrname) g_free(m->usrname);
      if (m->name) g_free(m->name);
      if (m->passwd) g_free(m->passwd);
      if (m->phone) g_free(m->phone);
      if (m->email) g_free(m->email);
      
      m->usrname = g_strdup((gchar *)sqlite3_column_text(stmt, 0));
      m->passwd = g_strdup((gchar *)sqlite3_column_text(stmt, 1));
      m->name = g_strdup((gchar *)sqlite3_column_text(stmt, 2));
      m->usrlvl = sqlite3_column_int(stmt, 3);
      m->phone = g_strdup((gchar *)sqlite3_column_text(stmt, 4));
      m->email = g_strdup((gchar *)sqlite3_column_text(stmt, 5));
      m->flags = sqlite3_column_int(stmt, 6);
    }

  sqlite3_finalize(stmt);

  return TRUE;
}

/* Static */
static gboolean
_CCL_employee_store(gint employee)
{
  CCL_employee *m = g_datalist_id_get_data(&(ccl->employees), employee);
  gchar *cmd = NULL, *errstr=NULL;
  char *pwd;
  int retval; 

  g_return_val_if_fail(m, FALSE);
      
  pwd = m->passwd;
  if (m->passwd){
    pwd = g_strdup(m->passwd);
    /*_simple_encrypt(pwd, strlen(pwd));*/
  }
  cmd = sqlite3_mprintf("update employees \n"
			"set emplevel = %d,\n"
			"    empname = %Q,\n"
			"    emppass = %Q,\n"
			"    email = %Q,\n"
			"    phone = %Q,\n"
			"    flags = %ld\n"
			"where id = %d;", m->usrlvl, m->name,
			pwd, m->email, m->phone, m->flags, employee);
  retval = sqlite3_exec(ccl->db, cmd, NULL, NULL, &errstr);
#ifdef DEBUG_LIBCCL 
  printf("_CCL_employee_store(): retval[%d] cmd = %s\n", retval, cmd);
  if (retval > 0){
    printf("_CCL_employee_store(): retval[%d] Error = %s\n", retval, errstr);
  }
#endif
  sqlite3_free(errstr);
  sqlite3_free(cmd);
  g_free(pwd);

  return TRUE;
}

/**
 * Gets information of an employee.
 *
 * @param   id The id of the employee.
 * @param[out] empusr Storage for the username
 * @param[out] empname Storage for the full employee name
 * @param[out] emppass Storage for the password
 * @param[out] phone Employee phone
 * @param[out] email email
 * @param[out] emplevel User Level (Admin, Supervisor, User)
 * @param[out] hiredate Integer for Date
 * @param[out] superid Supervisor's ID
 * @return FALSE on failure, TRUE otherwise.
 *
 * You have to free char * with CCL_free to avoid memory leaks.
 */
int
CCL_employee_info_get(int id, char **empusr, char **empname, char **emppass, 
		      char **phone, char **email, unsigned *emplevel, 
		      unsigned *hiredate, unsigned *superid, unsigned *flags)
{
  gboolean retval = FALSE;
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;

  cmd = sqlite3_mprintf("select empusr, empname, emppass, phone, "
			"email, emplevel, hiredate, superid, flags from employees\n"
			"where id = %d;", id);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    {
      if (empusr)
	*empusr = g_strdup((gchar *) sqlite3_column_text(stmt, 0));
      if (empname) 
	*empname = g_strdup((gchar *) sqlite3_column_text(stmt, 1));
      if (emppass)
	*emppass = g_strdup((gchar *) sqlite3_column_text(stmt, 2));
      if (phone)
	*phone = g_strdup((gchar *) sqlite3_column_text(stmt, 3));
      if (email)
	*email = g_strdup((gchar *) sqlite3_column_text(stmt, 4));
      if (emplevel)
	*emplevel = (guint) (sqlite3_column_int(stmt, 5)); 
      if (hiredate)
	*hiredate = (guint) (sqlite3_column_int(stmt, 6));
      if (superid)
	*superid = (guint) (sqlite3_column_int(stmt, 7));
      if (flags)
	*flags = (guint) (sqlite3_column_int(stmt, 8));

      retval = TRUE;
    }
  sqlite3_finalize(stmt);

  return retval;
}
/**
 * Gets information of an employee.
 *
 * @param   id The id of the employee.
 * @param   emppass Storage for the password
 * @param   empname Storage for the full employee name
 * @param   phone Employee phone
 * @param   email email
 * @param   emplevel User Level (Admin, Supervisor, User)
 *
 * You have to free char * with CCL_free to avoid memory leaks.
 */
void
CCL_employee_info_set(int id, char *emppass, char *empname, 
		      char *phone, char *email, long emplevel)
{
  CCL_employee *e = g_datalist_id_get_data(&(ccl->employees), id);
  gchar *cmd = NULL, *errstr = NULL;
  int retval;

  g_return_if_fail(e);
  if (e->passwd!=NULL) g_free(e->passwd);
  if (e->name!=NULL) g_free(e->name);
  if (e->phone!=NULL) g_free(e->phone);
  if (e->email!=NULL) g_free(e->email);
  
  e->passwd = g_strdup(emppass);
  e->name = g_strdup(empname);
  e->phone = g_strdup(phone);
  e->email = g_strdup(email);
  e->usrlvl = emplevel;
  
  /* cmd = sqlite3_mprintf("update employees \n"
			"set emppass = %Q,\n"
			"    empname = %Q,\n"
			"    phone = %Q,\n"
			"    email = %Q,\n" 
			"    usrlvl = %ld\n"
			"where id = %d;", emppass, empname, 
			phone, email, emplevel, id);
  */
  cmd = sqlite3_mprintf("update employees\n"
			"set emppass=%Q,\n"
			"    empname=%Q,\n"
			"    email=%Q,\n"
			"    phone=%Q,\n"
			"    emplevel=%ld\n"
			"where id=%d;", 
			emppass, empname, email, phone, emplevel, id);
  retval = sqlite3_exec(ccl->db, cmd, NULL, NULL, &errstr);
#ifdef DEBUG_LIBCCL 
  printf("_CCL_employee_info_set(): [retval = %d]\ncmd = %s\n", retval, cmd);
  if (retval > 0){
    printf("_CCL_employee_info_set(): [retval = %d] Error = %s\n", retval, 
	   errstr);
  }    
#endif
  sqlite3_free(errstr);
  sqlite3_free(cmd);
}

void
_destroy_employee(gpointer data)
{
  CCL_employee *employee = (CCL_employee *) data;

  if (employee->name) g_free(employee->name);
  if (employee->phone) g_free(employee->phone);
  if (employee->email) g_free(employee->email);
  if (employee->passwd) g_free(employee->passwd);
  if (employee->usrname) g_free(employee->usrname);

  g_free(employee);
}

void
_CCL_employee_init(CCL_employee * employee, char *usr, char *name, char *pwd, 
		   char *phone, char *email, int lvl, int superid)
{

  employee->usrname = g_strdup(usr);

  if (name) employee->name = g_strdup(name);
  if (phone) employee->phone = g_strdup(phone);
  if (email) employee->email = g_strdup(email);
  if (pwd) employee->passwd = g_strdup(pwd);
  employee->usrlvl = lvl;
  employee->flags = 0;
  employee->superid = superid;
  employee->data = NULL;
#ifdef DEBUG_LIBCCL
  printf ("username =%s -> %s\n", usr, employee->usrname);
  printf ("name =%s -> %s\n", name, employee->name);
  printf ("phone =%s -> %s\n", phone, employee->phone);
  printf ("email =%s -> %s\n", email, employee->email);
  printf ("passwd =%s -> %s\n", pwd, employee->passwd);
  printf ("usrlvl =%d -> %d\n", lvl, employee->usrlvl);
  printf ("superid =%d -> %d\n", superid, employee->superid);
#endif
}

void
_CCL_employee_init1(CCL_employee * employee, const gchar *usr)
{
  employee->usrname = g_strdup(usr);

  employee->name = NULL;
  employee->phone = NULL;
  employee->email = NULL;
  employee->passwd = NULL;
  employee->usrlvl = 0;
  employee->flags = 0;
  employee->superid = 0;
  employee->data = NULL;
}
