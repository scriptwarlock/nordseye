#ifndef CCLS_H
#define CCLS_H

#include <time.h>

#define CCL_SUN		(1<<0) /** Sunday */
#define CCL_MON		(1<<1) /** Monday */
#define CCL_TUE		(1<<2) /** Tuesday */
#define CCL_WED		(1<<3) /** Wednesday */
#define CCL_THU		(1<<4) /** Thursday */
#define CCL_FRI		(1<<5) /** Friday */
#define CCL_SAT		(1<<6) /** Saturday */

/**
 * If the stock amount of a product is equal to DELETEDPRODUCT, then that
 * product is treated as deleted.
 */
#define CCL_DELETEDPRODUCT	(0x80000000)

enum
{
  /** Inactive status (no one is using it) */
  CCL_INACTIVE = 0,
  /** Active status (someone is using it) */
  CCL_ACTIVE,
  /** Paused status (someone was using it, but now it is suspended) */
  CCL_PAUSED
};

/* Data Categories */
enum
{
  CCL_DATA_NONE = 0,	      /** The data is assigned to nothing */
  CCL_DATA_CLIENT = -1,	      /** The data is assigned to a client */
  CCL_DATA_MEMBER = -2,	      /** The data is assigned to a member */
  CCL_DATA_TARIF = -3,	      /** The data is assigned to a tarif */
  CCL_DATA_PRODUCT = -4,      /** The data is assigned to a product */
  CCL_DATA_PRICE = -5,	      /** The data is assigned to a price */
  CCL_DATA_LOGSESSION = -6,   /** The data is assigned to a logged session */
  CCL_DATA_LOGPRODUCT = -7,   /** The data is assigned to a logged product */
  CCL_DATA_LOGEXPENSE = -8,   /** The data is assigned to a logged expense */
  CCL_DATA_EMPLOYEE = -9,     /** The data is assigned to an employee */
  CCL_DATA_USRLVL = -10,       /** The data is assigned to a user level */
  CCL_DATA_SETTINGS = -11     /** The data is assigned to settings */
};


/* Rules for searching the sessions log */
#define CCL_SR_ID	  (1UL<<0) /** Search by entry id */
#define CCL_SR_SESSION	  (1UL<<1) /** Search by session id */
#define CCL_SR_CLIENT	  (1UL<<2) /** Search by client id */
#define CCL_SR_MEMBER	  (1UL<<3) /** Search by member id */
#define CCL_SR_STIMEMIN	  (1UL<<4) /** Search by start time (lower limit) */ 
#define CCL_SR_STIMEMAX	  (1UL<<5) /** Search by start time (higher limit) */
#define CCL_SR_ETIMEMIN	  (1UL<<6) /** Search by end time (lower limit) */
#define CCL_SR_ETIMEMAX	  (1UL<<7) /** Search by end time (higher limit) */
#define CCL_SR_TIMEMIN	  (1UL<<8) /** Search by total time (lower limit) */
#define CCL_SR_TIMEMAX	  (1UL<<9) /** Search by total time (higher limit) */
#define CCL_SR_PRICEMIN	  (1UL<<10) /** Search by price paid (lower limit) */
#define CCL_SR_PRICEMAX	  (1UL<<11) /** Search by price paid (higher limit) */
#define CCL_SR_FLAGS	  (1UL<<12) /** Search by flags */
#define CCL_SR_DAYTIME_RANGE (1UL<<13) /** Search by daytime range */
#define CCL_SR_DAYS	  (1UL<<14) /** Search by days */
#define CCL_SR_FLAGSNOT	  (1UL<<15) /** Search by negation of flags */
#define CCL_SR_PRODUCT	  (1UL<<16) /** Search by product */
#define CCL_SR_STIME_RANGE (CCL_SR_STIMEMIN|CCL_SR_STIMEMAX) /** STIMEMIN+STIMEMAX */
#define CCL_SR_ETIME_RANGE (CCL_SR_ETIMEMIN|CCL_SR_ETIMEMAX) /** ETIMEMIN+ETIMEMAX */
#define CCL_SR_TIME_RANGE (CCL_SR_TIMEMIN|CCL_SR_TIMEMAX) /** TIMEMIN+TIMEMAX */
#define CCL_SR_PRICE_RANGE (CCL_SR_PRICEMIN|CCL_SR_PRICEMAX) /** PRICEMIN+PRICEMAX */
#define CCL_SR_FLAGS_ANY  (1UL<<17) /** Contains any of the specified flags */
#define CCL_SR_FLAGSNOT_ANY (1UL<<18) /** Any of the specified flags is missing */
#define CCL_SR_EMPLOYEE	  (1UL<<19) /** Search by employee */


/********************* Error codes ************************/
#define CCL_ERROR_NO_ERROR		0 /** No error ocurred */
#define CCL_ERROR_BAD_PASSWORD		1 /** Bad password */
#define CCL_ERROR_COULD_NOT_LOAD_VL	2 /** Couldn't load verify locations */

/******************* MD5 Digest Length ********************/
#define CCL_MD5_DIGEST_LENGTH		16 /** MD5 digest length (in bytes) */

/******************** Callback Types **********************/
/**
 * Callback used when a message from a client is received
 *
 * @param   client The id of the client.
 * @param   cmd The command.
 * @param   data Data associated with the command.
 * @param   datasize The size of the data.
 * @param   userdata User supplied data.
 */
typedef void (*CCL_on_event_cb) (int client, unsigned cmd, void * data,
				 unsigned datasize, void * userdata);
/**
 * Callback used when a client connects.
 *
 * @param   client The id of the client.
 * @param   userdata User supplied data.
 */
typedef void (*CCL_on_connect_cb) (int client, void * userdata);

/**
 * Callback used when a client disconnects.
 *
 * @param   client The id of the client.
 * @param   userdata User supplied data.
 */
typedef void (*CCL_on_disconnect_cb) (int client, void * userdata);

struct _CCL_log_search_rules /** Log search rules struct */
{
  unsigned rulemask;
  int id;
  int session;
  int client;
  int member;
  int employee;
  int product;
  time_t stime_min, stime_max;
  time_t etime_min, etime_max;
  time_t time_min, time_max;
  unsigned daytime_min, daytime_max;
  unsigned days;
  unsigned price_min, price_max;
  int flags;
  int flags_not;
};
typedef struct _CCL_log_search_rules CCL_log_search_rules;

struct _CCL_log_session_entry  /** Logged clients session struct */
{
  int id;
  int client;
  int member;
  int employee;
  time_t stime;
  time_t etime;
  int time;
  unsigned price;
  int flags;
};
typedef struct _CCL_log_session_entry CCL_log_session_entry;

struct _CCL_log_product_entry /** Logged sold product entry */
{
  int id;
  int session;
  int client;
  int member;
  int employee;
  int product;
  unsigned amount;
  unsigned price;
  time_t time;
  int flags;
};
typedef struct _CCL_log_product_entry CCL_log_product_entry;

struct _CCL_ticket_entry /** ticket entry */
{
  int       id;
  time_t    pdate;
  time_t    stdate;
  time_t    enddate;
  unsigned  faceval;
  unsigned  curval;
  unsigned  credit;
  int       tariff;
  int       flags;
  char      name[32];
};
typedef struct _CCL_ticket_entry CCL_ticket_entry;

struct _CCL_log_expense_entry /** Logged expense entry */
{
  const char description[128];
  time_t time;
  unsigned cash;
  int flags;
};

typedef struct _CCL_log_expense_entry CCL_log_expense_entry;

/**********************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
int	      CCL_init(const char * dbfile);
int	      CCL_shutdown(void);
void	      CCL_set_on_event_callback(CCL_on_event_cb callback,
					void * userdata);
void	      CCL_set_on_connect_callback(CCL_on_connect_cb callback,
					  void * userdata);
void	      CCL_set_on_disconnect_callback(CCL_on_disconnect_cb callback,
					     void * userdata);
int	      CCL_SSL_init(const char * cafile, const char * certfile,
			   const char * certpass, int * error);
int	      CCL_networking_init(unsigned short port, int * error);
int	      CCL_networking_shutdown(void);
int	      CCL_check_events(void);
int           CCL_set_settings(void *pSettings);

int	      CCL_product_new(const char * category, const char * name,
			      unsigned price);
void	      CCL_product_delete(int product);
int	      CCL_product_price_set(int product, unsigned price);
int	      CCL_product_get_nth(unsigned nth);
int	      CCL_product_info_get(int product, char **category,
				   char **name, unsigned * price);
int	      CCL_product_id_get(char *name, int *id);
void	      CCL_product_flags_set(int product, int flags);
int	      CCL_product_flags_get(int product);
void	      CCL_product_flags_toggle(int product, int flags, int on);
int	      CCL_product_exists(int product);
int	      CCL_product_sell(int product, unsigned amount,
			       unsigned totalprice, int flags, int empid);
void	      CCL_product_stock_set(int product, int amount);
int	      CCL_product_stock_get(int product);
int           CCL_pay_account(int member, double price, int empid);


void	      CCL_perminafter_set(int mins);
int	      CCL_perminafter_get(void);
/**********************************************************/
void	      CCL_data_set_string(int cid, int id, const char * key,
				  const char * value);
char *	      CCL_data_get_string(int cid, int id, const char * key,
				  const char * defval);
void	      CCL_data_set_int(int cid, int id, const char * key, int value);
int	      CCL_data_get_int(int cid, int id, const char * key, int defval);
void	      CCL_data_set_blob(int cid, int id, const char * key,
				void * value, int size);
void *	      CCL_data_get_blob(int cid, int id, const char * key, int * size);
int	      CCL_data_key_exists(int cid, int id, const char * key);
void	      CCL_data_key_delete(int cid, int id, const char * key);
int	      CCL_data_find_by_key_sval(int cid, const char * key,
					const char * value);
int	      CCL_data_find_by_key_ival(int cid, const char * key,
					int value);
/**********************************************************/
unsigned      CCL_htonl(unsigned val);
unsigned short
	      CCL_htons(unsigned short val);
unsigned      CCL_ntohl(unsigned val);
unsigned short
	      CCL_ntohs(unsigned short val);
void	      CCL_free(const void * mem);
unsigned char *
	      CCL_MD5(const unsigned char * d, unsigned long n,
		      unsigned char * md);
/**********************************************************/
int	      CCL_client_new(const char * name);
int	      CCL_client_exists(int client);
int	      CCL_client_find(const char * name);
int	      CCL_client_get_nth(unsigned nth);
void	      CCL_client_reset(int client);
const char *  CCL_client_name_get(int client);
int	      CCL_client_status_get(int client);
void	      CCL_client_start(int client);
void	      CCL_client_stop(int client);
void	      CCL_client_unstop(int client);
void	      CCL_client_pause(int client);
void	      CCL_client_unpause(int client);
void	      CCL_client_swap(int lclient, int rclient);
void	      CCL_client_product_add(int client, int product, unsigned num);
void	      CCL_client_product_sub(int client, int product, unsigned num);
unsigned      CCL_client_product_amount_get(int client, int product);
void	      CCL_client_product_amount_set(int client, int product,
					    unsigned num);
int	      CCL_client_product_get_nth(int client, unsigned nth,
					 int * product, unsigned * num);
void	      CCL_client_timeout_set(int client, time_t timeout);
time_t	      CCL_client_timeout_get(int client);
time_t	      CCL_client_time_used(int client);
time_t	      CCL_client_time_left(int client);
time_t	      CCL_client_stime_get(int client);
time_t	      CCL_client_etime_get(int client);
int	      CCL_client_intervals_get_num(int client);
int	      CCL_client_interval_get_nth(int client, unsigned nth,
					  time_t * stime, time_t * etime);
void	      CCL_client_flags_set(int client, int flags);
int	      CCL_client_flags_get(int client);
void	      CCL_client_flags_toggle(int client, int flags, int on);
void *	      CCL_client_data_set(int client, void * data);
void *	      CCL_client_data_get(int client);
unsigned      CCL_client_owed_terminal(int client);
unsigned      CCL_client_owed_products(int client);
void	      CCL_client_member_set(int client, int member);
int	      CCL_client_member_get(int client);
long	      CCL_client_ip_get(int client);
void	      CCL_client_send_cmd(int client, unsigned cmd,
				  const void * data, unsigned datasize);
/**********************************************************/
  int	      CCL_member_new(const char * name, int emp);
int	      CCL_member_get_nth(unsigned nth);
int	      CCL_member_exists(int member);
int	      CCL_member_find(const char * name);
const char *  CCL_member_name_get(int member);
void	      CCL_member_name_set(int member, const char * name);
int	      CCL_member_tarif_get(int member);
void	      CCL_member_tarif_set(int member, int tarif);
const char *  CCL_member_other_get(int member);
void	      CCL_member_other_set(int member, const char * other);
const char *  CCL_member_email_get(int member);
void	      CCL_member_email_set(int member, const char * email);
void *	      CCL_member_data_set(int member, void * data);
void *	      CCL_member_data_get(int member);
void	      CCL_member_flags_set(int member, int flags);
int	      CCL_member_flags_get(int member);
void	      CCL_member_flags_toggle(int member, int flags, int on);
void	      CCL_member_credit_set(int member, int credit);
int	      CCL_member_credit_get(int member);
int           CCL_member_ticket_new(int member, char *sqlstr);
int           CCL_member_ticket_del(int member, char *sqlstr);
int           CCL_member_ticket_find(const char *tktstr);
int           CCL_member_tickets_get(char *sqlstr, CCL_ticket_entry **te);
/**********************************************************/

int           CCL_employee_new(char *usr, char *name, char *pwd, char *phone, 
			       char *email, int lvl, int superid);
int	      CCL_employee_get_nth(unsigned nth);
int	      CCL_employee_exists(int employee);
int	      CCL_employee_find(const char * name);
int           CCL_employee_validate(const char * name, const char *pwd);
const char *  CCL_employee_usrname_get(int employee);
void	      CCL_employee_usrname_set(int employee, const char * name);
int	      CCL_employee_hiredate_get(int employee);
void	      CCL_employee_hiredate_set(int employee, int hiredate);
const char *  CCL_employee_password_get(int employee);
void	      CCL_employee_password_set(int employee, const char * pwd);
const char *  CCL_employee_email_get(int employee);
void	      CCL_employee_email_set(int employee, const char * email);
const char *  CCL_employee_phone_get(int employee);
void	      CCL_employee_phone_set(int employee, const char * phone);
const char *  CCL_employee_name_get(int employee);
void	      CCL_employee_name_set(int employee, const char * name);
int           CCL_employee_usrlvl_get(int employee);
void	      CCL_employee_usrlvl_set(int employee, int lvl);
void *	      CCL_employee_data_set(int employee, void * data);
void *	      CCL_employee_data_get(int employee);
void	      CCL_employee_flags_set(int employee, int flags);
int	      CCL_employee_flags_get(int employee);
void	      CCL_employee_flags_toggle(int employee, int flags, int on);
int           CCL_employee_info_get(int empid, char **empusr, char **empname, char **emppass, 
				    char **phone, char **email, unsigned *emplevel, 
				    unsigned *hiredate, unsigned *superid, unsigned *flags);
void          CCL_employee_info_set(int id, char *emppass, char *empname, char *phone, 
				    char *email, long emplevel);

/**********************************************************/
void	      CCL_log_expense(const char description[128], unsigned cash,
			      int flags);
int	      CCL_log_expenses_get(CCL_log_search_rules * sr,
				   CCL_log_expense_entry ** ee);
  int	      CCL_log_session(int client, unsigned price, int flags, int emp);
int	      CCL_log_session_find(int client, time_t stime, time_t etime);
void	      CCL_log_session_clear(int session);
void	      CCL_log_session_set_flags(int session, int flags);
void	      CCL_log_session_set_price(int session, unsigned price);
void	      CCL_log_session_set_member(int session, int member);
int	      CCL_log_sessions_get(CCL_log_search_rules * sr,
				   CCL_log_session_entry ** se);
int	      CCL_log_session_intervals_get(int session, time_t ** intervals);
void	      CCL_log_product_set_flags(int id, int flags);
void	      CCL_log_product_set_price(int id, unsigned price);
int	      CCL_log_products_get(CCL_log_search_rules * sr,
				   CCL_log_product_entry ** pe);
/**********************************************************/
int	      CCL_tarif_new(unsigned hr, unsigned min, unsigned days,
			    unsigned hourprice, unsigned incprice, 
			    unsigned fafter, char *name);
void	      CCL_tarif_delete(int tarif);
int	      CCL_tarif_get_nth(unsigned nth);
void	      CCL_tarif_rebuild(void);
void	      CCL_tarif_rebuild_all(void);
int	      CCL_tarif_set(int tarif);
int	      CCL_tarif_get(void);
int           CCL_tarif_name_exists(char *name);
char         *CCL_tarif_name_get(int tarif);
int	      CCL_tarif_exists(int tarif);
unsigned      CCL_tarif_calc(time_t start, time_t end, int permin);
unsigned      CCL_tarif_calc_with_tarifpart(int id, unsigned mins,
					    int permin);
/**********************************************************/
int	      CCL_tarifpart_new(unsigned hr, unsigned min, unsigned days,
				unsigned hourprice, unsigned incprice);
void	      CCL_tarifpart_delete(int id);
int	      CCL_tarifpart_get_nth(unsigned nth);
int	      CCL_tarifpart_exists(int id);
int	      CCL_tarifpart_id_get(unsigned hr, unsigned min, unsigned days);
int	      CCL_tarifpart_conflicts(unsigned hr, unsigned min,
				      unsigned days, unsigned * conflicts); 
int	      CCL_tarifpart_info_get(int id, unsigned *hr, unsigned *min, unsigned *days, 
				     unsigned * hourprice, unsigned *incprice);
int	      CCL_tarifpart_stime_set(int id, unsigned hr, unsigned min);
void	      CCL_tarifpart_stime_get(int id, unsigned * hr, unsigned * min);
int	      CCL_tarifpart_days_set(int id, unsigned days);
unsigned      CCL_tarifpart_days_get(int id);
void	      CCL_tarifpart_hourprice_set(int id, unsigned price);
unsigned      CCL_tarifpart_hourprice_get(int id);
void	      CCL_tarifpart_flags_set(int client, int flags);
int	      CCL_tarifpart_flags_get(int client);
void	      CCL_tarifpart_fafter_set(int tarif, unsigned fafter);
unsigned      CCL_tarifpart_fafter_get(int tarif);
void	      CCL_tarifpart_incprice_set(int tarif, unsigned incprice);
unsigned      CCL_tarifpart_incprice_get(int tarif);
void	      CCL_tarifpart_flags_toggle(int client, int flags, int on);
void	      CCL_tarifpart_price_add(int id, unsigned mins, unsigned price);
void	      CCL_tarifpart_price_del(int id, unsigned mins);
void	      CCL_tarifpart_price_clear(int id);
unsigned      CCL_tarifpart_price_get(int id, unsigned mins, int strict);
int	      CCL_tarifpart_price_exists(int id, unsigned mins);
unsigned      CCL_tarifpart_price_get_nearest(int id, unsigned mins);
unsigned      CCL_tarifpart_price_get_last(int id);
int	      CCL_tarifpart_price_get_nth(int id, unsigned nth,
					  unsigned * mins, unsigned * price);
#ifdef __cplusplus
}
#endif

#endif
