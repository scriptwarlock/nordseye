#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "ccls.h"
#include "ccl_private.h"


/*#define DEBUG 1*/
/*#define DEBUG_A*/
/*#define DEBUG_T */
/* Static functions */
static void _init_db(sqlite3 * db);
static gint _loadMembersCB(gpointer ptr, gint argc,
			   gchar ** argv, gchar ** colnames);
static gint _loadClientsCB(gpointer ptr, gint argc,
			   gchar ** argv, gchar ** colnames);
static gint _loadEmployeesCB(gpointer ptr, gint argc,
			   gchar ** argv, gchar ** colnames);
static void _shutdown_client_connection(gint clientid);
static void _FindClientByFdFunc(GQuark key_id, gpointer data,
				gpointer user_data);
static void _ShutdownClientConnectionFunc(GQuark key_id, gpointer data,
					  gpointer user_data);
static SSL_CTX * _initialize_ssl_ctx(const gchar * cafile,
				     const gchar * keyfile,
				     const gchar * password, gint * err);
static gint _greatest_product_id();


int do_ccl_server(void *arg);
int do_cnxn_proc(void *arg);


CCL *ccl = NULL;

/* Public interface */

/**
 * Initializes CCL.
 *
 * @param   dbfile The filename of the database to use.
 * @return  FALSE on failure, TRUE if initialized correctly.
 *
 * You need to call this before doing anything else with CCL.
 */
gboolean
CCL_init(const gchar * dbfile)
{
  if (ccl)
    return TRUE;
  if (NULL == (ccl = g_new0(CCL, 1)))
    return FALSE;
  if (SQLITE_OK != sqlite3_open(dbfile, &(ccl->db)))
    return FALSE;
  _init_db(ccl->db);
  ccl->events.listenfd = INVALID_SOCKET;
  ccl->perminafter = 10;
  ccl->tarif = NULL;
  g_datalist_init(&(ccl->members));
  g_datalist_init(&(ccl->clients));
  g_datalist_init(&(ccl->employees));

  SSL_library_init();
  SSL_load_error_strings();

  sqlite3_exec(ccl->db,
	       "select id, name from members;",
	       _loadMembersCB, NULL, NULL);
  sqlite3_exec(ccl->db,
	       "select id, name from clients;",
	       _loadClientsCB, NULL, NULL);
  /*  sqlite3_exec(ccl->db,
	       "select id, empusr from employees;",*/
  sqlite3_exec(ccl->db,
  "select id, empusr, empname, emppass, phone, email, "
  "emplevel, hiredate, superid from employees;",
  _loadEmployeesCB, NULL, NULL);

#ifdef DEBUG
  printf("CCL_Init(): Initialized the CCL Library\n");
#endif
  return TRUE;
}

/**
 * Shutdowns CCL.
 *
 * This functions frees all memory allocated by CCL.
 *
 * @return FALSE
 *
 * Call this after you are no longer going to use CCL.
 */
gboolean
CCL_shutdown(void)
{
  g_datalist_clear(&(ccl->clients));
  sqlite3_close(ccl->db);
  CCL_networking_shutdown();
  g_free(ccl->events.certpass);
  if (ccl->events.ssl_ctx) SSL_CTX_free(ccl->events.ssl_ctx);
  g_free(ccl);

  return FALSE;
}

/**
 * Sets the callback to be used when a message from a client is received.
 *
 * @param   callback The function to be called when an event occurs.
 * @param   userdata The data to be passed as an argument to the callback.
 */
void
CCL_set_on_event_callback(CCL_on_event_cb callback, void * userdata)
{
  ccl->events.on_event = callback;
  ccl->events.on_event_data = userdata;
}

/**
 * Sets the callback to be used when a client connects.
 *
 * @param   callback The function to be called when a client connects.
 * @param   userdata The data to be passed as an argument to the callback.
 */
void
CCL_set_on_connect_callback(CCL_on_connect_cb callback, void * userdata)
{
  ccl->events.on_connect = callback;
  ccl->events.on_connect_data = userdata;
}

/**
 * Sets the callback to be used when a client disconnects.
 *
 * @param   callback The function to be called when a client disconnects.
 * @param   userdata The data to be passed as an argument to the callback.
 */
void
CCL_set_on_disconnect_callback(CCL_on_disconnect_cb callback, void * userdata)
{
  ccl->events.on_disconnect = callback;
  ccl->events.on_disconnect_data = userdata;
}

/**
 * Initialize SSL support.
 *
 * @param   cafile CA cert filename.
 * @param   certfile Certificate filename.
 * @param   certpass Certificate password.
 * @param[out] error The error code will be stored here.
 * @return  FALSE on failure, TRUE otherwise.
 *
 * error can be NULL if you don't care about it.
 * Call this before you initialize CCL networking if you want SSL support.
 */
gboolean
CCL_SSL_init(const gchar * cafile, const gchar * certfile,
	     const gchar * certpass, gint * error)
{
  if (!ccl->events.ssl_ctx)
    if (!(ccl->events.ssl_ctx = _initialize_ssl_ctx(cafile, certfile, certpass,
						    error)))
      return FALSE;

  return TRUE;
}

/**
 * Initialize networking.
 *
 * @param   port The port where we are going to listen for connections.
 * @param[out] error The error code will be stored here.
 * @return  FALSE on failure, TRUE otherwise.
 *
 * error can be NULL if you don't care about it.
 * Call this to start listening for new connections.
 */
gboolean
CCL_networking_init(gushort port, gint * error)
{
  gchar portstr[256];
  pthread_t th;

  if (ccl->events.listenbio) /* Already initialized */
    return TRUE;

  g_snprintf(portstr, sizeof(portstr)/sizeof(gchar), "%u", port);

  ccl->events.listenbio = BIO_new_accept(portstr);
  BIO_set_bind_mode(ccl->events.listenbio, BIO_BIND_REUSEADDR_IF_UNUSED);
  BIO_do_accept(ccl->events.listenbio);
  ccl->events.listenfd = BIO_get_fd(ccl->events.listenbio, NULL);

  if (ccl->events.maxfd < ccl->events.listenfd)
    ccl->events.maxfd = ccl->events.listenfd;
  
  FD_SET(ccl->events.listenfd, &(ccl->events.readfds));
#ifdef DEBUG
  {
    char *bcp;
    pthread_t th;
    
    bcp = BIO_get_accept_port(ccl->events.listenbio);
    printf("** CCL_networking_init(): BIO_get_accept_port: %s **\n", bcp);
    
  }
#endif
  /* now start listening */
  ccl->tid = pthread_create(&th, NULL, do_ccl_server, portstr);
  
  return TRUE;
}

/**
 * Shutdowns networking.
 *
 * @return TRUE
 *
 * Call this when you no longer need networking support.
 */
gboolean
CCL_networking_shutdown(void)
{
  g_datalist_foreach(&(ccl->clients), _ShutdownClientConnectionFunc, NULL);

  if (ccl->events.listenbio)
    {
      BIO_free(ccl->events.listenbio);
      ccl->events.listenbio = NULL;
      ccl->events.listenfd = INVALID_SOCKET;
    }

  return TRUE;
}

/**
 * Make some internal settings
 *
 * @param  pointer to settings structure
 * @return 1 if successful
 */
gint
CCL_set_settings(void *pSettings)
{
  ccl->rndoff = (int)pSettings;

  return 1;
}


/**
 * Sets after how many minutes start to calculate the price by minute.
 *
 * @param   mins The amount of minutes.
 */
void
CCL_perminafter_set(gint mins)
{
  ccl->perminafter = mins;
}

/**
 * Gets after how many minutes start to calculate the price by minute.
 *
 * @return The amount of minutes.
 */
gint
CCL_perminafter_get(void)
{
  return ccl->perminafter;
}

/**
 * Creates a new product.
 *
 * @param   category The name of the category to wich this products belongs to.
 * @param   name The name of the product.
 * @param   price The price of the product (in cents).
 * @return The id of the created product.
 */
gint
CCL_product_new(const gchar * category, const gchar * name, guint price)
{
  gchar *cmd = NULL;
  gint id;
  
  id = 1 + _greatest_product_id();
  
  if (0 >= id) id = 1; /* Make the first one have id 1 instead of 0 */
  
  cmd = sqlite3_mprintf("insert into products\n"
			"(id, name, category, price, stock)\n"
			"values(%d, %Q, %Q, %u.%.2u, %u);",
			id, name, category, price / 100, price % 100, 0);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);

  return id;
}

/**
 * Deletes a product.
 *
 * @param   product The id of the product to be deleted.
 */
void
CCL_product_delete(gint product)
{
  gchar *cmd = NULL;
  
  /*cmd = sqlite3_mprintf("update products set stock = %d\n"
    "where id = %d;", CCL_DELETEDPRODUCT, product);*/
  cmd = sqlite3_mprintf("update products set flags = %d\n"
			"where id = %d;", CCL_DELETEDPRODUCT, product);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Sets the price of a product.
 *
 * @param   product The id of the product.
 * @param   price The new price.
 * @return False on failure, TRUE otherwise.
 */
gboolean
CCL_product_price_set(gint product, guint price)
{
  gchar *cmd = NULL;

  if (!CCL_product_exists(product))
    return FALSE;
  else
    {
      cmd = sqlite3_mprintf("update products\n"
			    "set price = %u.%.2u where id = %d;",
			    price / 100, price % 100, product);
      sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
      sqlite3_free(cmd);

      return TRUE;
    }
}

/**
 * Gets the nth product on the list.
 *
 * @param   nth The index of the product to get (starting at 0).
 * @return The id of the product, or -1 if there aren't more products.
 *
 * Use this to iterate on the products, for example when building a list of
 * all the products.
 */
gint
CCL_product_get_nth(guint nth)
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint i;
  gint id = -1;

  /*cmd = sqlite3_mprintf("select id from products where stock != %d\n"
    "order by id asc;", CCL_DELETEDPRODUCT);*/
  cmd = sqlite3_mprintf("select id from products where flags != %d\n"
			"order by id asc;", CCL_DELETEDPRODUCT);
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
 * Gets information of a product.
 *
 * @param   product The id of the product.
 * @param[out] category The category of the product will be stored here.
 * @param[out] name The name of the product will be stored here.
 * @param[out] price The price of the product will be stored here.
 * @return FALSE on failure, TRUE otherwise.
 *
 * Any of category, name, and price, can be NULL if you don't care about them.
 * You have to free name and category with CCL_free to avoid memory leaks.
 */
gboolean
CCL_product_info_get(gint product, char **category, char **name, guint * price)
{
  gboolean retval = FALSE;
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;

  cmd = sqlite3_mprintf("select category, name, price from products\n"
			"where id = %d;", product);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    {
      if (category)
	*category = g_strdup((gchar *) sqlite3_column_text(stmt, 0));
      if (name)
	*name = g_strdup((gchar *) sqlite3_column_text(stmt, 1));
      if (price)
	*price = (guint) (100 * sqlite3_column_double(stmt, 2));
      retval = TRUE;
    }
  sqlite3_finalize(stmt);

  return retval;
}
/**
 * Gets product ID of a product given a name
 * Checks whether the product exists in the Database.
 *
 * @param   Name of product
 * @param[out] id of the product.
 * @return FALSE on failure, TRUE otherwise.
 *
 * Any of category, name, and price, can be NULL if you don't care about them.
 * You have to free name and category with CCL_free to avoid memory leaks.
 */
gboolean
CCL_product_id_get(char *name, gint *pid)
{
  gboolean retval = FALSE;
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;

  cmd = sqlite3_mprintf("select id from products\n"
			"where name = '%s';", name);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    {
      if (pid)
	*pid = (gint) sqlite3_column_int(stmt, 0);
      retval = TRUE;
    }
  sqlite3_finalize(stmt);

  return retval;
}

/**
 * Sets a product's flags to the value of flags.
 *
 * @param   product The product's id.
 * @param   flags The new flags value.
 */
void
CCL_product_flags_set(gint product, gint flags)
{
  gchar *cmd = NULL;

  g_return_if_fail(CCL_product_exists(product));
  
  cmd = sqlite3_mprintf("update products\n"
			"set flags = %d where id = %d;",
			flags, product);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Gets the flags value of a product.
 *
 * @param   product The product's id.
 * @return The flags value.
 */
gint
CCL_product_flags_get(gint product)
{
  gint flags = 0;
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;

  g_return_val_if_fail(CCL_product_exists(product), 0);

  cmd = sqlite3_mprintf("select flags from from products\n"
			"where id = %d;", product);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    flags = sqlite3_column_int(stmt, 0);
  
  sqlite3_finalize(stmt);

  return flags;
}

/**
 * Toggle a product's flags on or off.
 *
 * @param   product The product's id.
 * @param   flags The flags to toggle.
 * @param   on If TRUE toggle the flags on, else toggle them off.
 */
void
CCL_product_flags_toggle(gint product, gint flags, gboolean on)
{
  gchar *cmd = NULL;

  g_return_if_fail(CCL_product_exists(product));

  if (on)
    cmd = sqlite3_mprintf("update products set flags = (flags | %d)\n"
			  "where id = %d;", flags, product);
  else
    cmd = sqlite3_mprintf("update products set flags = (flags & (~%d))\n"
			  "where id = %d;", flags, product);

  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Checks if a product already exists.
 *
 * @param   product The product's id.
 * @return TRUE if the product exists, FALSE if not.
 */
gboolean
CCL_product_exists(gint product)
{
  return CCL_product_info_get(product, NULL, NULL, NULL);
}

/**
 * Sell a product.
 *
 * @param   product The id of the product.
 * @param   amount How many of that product.
 * @param   totalprice The price of the sum of products.
 * @param   flags Flags.
 * @return The id of the log entry, or -1.
 *
 * Use this when you sell a product, but to someone that isn't using a
 * workstation.
 */
int
CCL_product_sell(gint product, guint amount, guint totalprice, gint flags, gint empid)
{
  gchar *cmd = NULL;
  gint id = -1;

  if (CCL_product_exists(product))
    {
      cmd = sqlite3_mprintf("insert into productslog (session,client,member,"
			    "product,amount,price,time,empid,flags)\n"
			    "values(%d, %d, %d, %d, %u, %u, %ld, %d, %d);",
			    0, 0, 0, product, amount, totalprice, time(NULL), empid,
			    flags);
      sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
      sqlite3_free(cmd);
      id = sqlite3_last_insert_rowid(ccl->db);
      if (CCL_product_stock_get(product) != CCL_DELETEDPRODUCT)
	CCL_product_stock_set(product,CCL_product_stock_get(product) - amount);
    }

  return id;
}

/**
 * Accept Account Payments.
 *
 * @param   product The id of the product.
 * @param   price Amount of money paid
 * @param   empid employee id
 * @return The id of the log entry, or -1. 
 *
 */
int
CCL_pay_account(gint member, gdouble price, gint empid)
{
  gchar *cmd = NULL, *errstr = NULL;
  gint id = -1, product;
  gint flags = 0;

  CCL_product_id_get("Account Payment", &product);
  if (product)
    {
      int newcredit=0.;

      newcredit = CCL_member_credit_get(member) + price;
      if ((newcredit)>=0){
	guint intcred = newcredit; 
	/*update*/
	CCL_member_credit_set(member, newcredit);
	/*log*/
	cmd = sqlite3_mprintf("insert into productslog (session,client,member,"
			      "product,amount,price,time,empid,flags)\n"
			      "values(%d, %d, %d, %d, %u, %u, %ld, %d, %d);",
			      0, 0, member, product, 1, intcred, time(NULL), empid, flags);
	sqlite3_exec(ccl->db, cmd, NULL, NULL, &errstr);
	id = sqlite3_last_insert_rowid(ccl->db);
#ifdef DEBUG
	printf("CCL_pay_account(): cmd=%s, price=%d\n errstr=%s\n", cmd, newcredit,
	       errstr);
#endif
	sqlite3_free(cmd);
	sqlite3_free(errstr);
      }

    }

  return id;
}

/**
 * Set the stock of a given product.
 *
 * @param   product The id of the product.
 * @param   amount How many of this product.
 */
void
CCL_product_stock_set(gint product, gint amount)
{
  gchar *cmd = NULL;

  cmd = sqlite3_mprintf("update products set stock = %d\n"
			"where id = %d;", amount, product);
  sqlite3_exec(ccl->db, cmd, NULL, NULL, NULL);
  sqlite3_free(cmd);
}

/**
 * Get the stock of a given product.
 *
 * @param   product The id of the product.
 * @return How many of this product do you have on stock.
 */
gint
CCL_product_stock_get(gint product)
{
  gint amount = 0;
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;

  cmd = sqlite3_mprintf("select stock from products where id = %d;", product);
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    amount = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);

  return amount;
}



int do_cnxn_proc(void *arg)
{
  gint client = (int) arg;
  
  CCL_client *c = g_datalist_id_get_data(&(ccl->clients), client);
  gboolean connection_closed = FALSE;
  guint cmd = 0;
  guint size = 0;
  void *data = NULL;
  gint bytes = 0;
  BIO *bio = c->bio;
  

#ifdef DEBUG_T
  printf ("Starting Thread: Client = %d\n", client);
#endif
  for (;;){
    /*read command from client*/
    bytes = _recvall(bio, &cmd, sizeof(cmd));
#ifdef DEBUG_T
    printf ("Client = %d: Read Command: %04X\n", client, cmd);
#endif
    if (0 >= bytes || bytes != sizeof(cmd))
      {
	connection_closed = TRUE;
      }
    
    if (!connection_closed)
      {
	/*read command's data size from client*/
	bytes = _recvall(bio, &size, sizeof(size));
	if (0 >= bytes || bytes != sizeof(size))
	  connection_closed = TRUE;
      }
    cmd = CCL_ntohl(cmd);
    size = CCL_ntohl(size);
#ifdef DEBUG_T
    printf ("Client = %d: Command [%04X] Size: %d\n", client, cmd, size);
#endif
    data = NULL;
    if (!connection_closed && 0 < cmd && 0 < size && size < MAX_CLIENT_PACKET_SIZE)
      {
	/*read command's data from client*/
	data = g_malloc0(size);
	bytes = _recvall(bio, data, size);
	if (0 >= bytes || bytes != size)
	  connection_closed = TRUE;
#ifdef DEBUG_T
	printf ("Client = %d: Command [%04X] Read Size: %d\n", client, cmd, bytes);
#endif
      }
    if (ccl->events.on_event)
      ccl->events.on_event(client, cmd, data, size,
			   ccl->events.on_event_data);
    if (data)
      g_free(data);
    /* If the connection broke, or was closed */
    if (connection_closed)
      {
	_shutdown_client_connection(client);
	if (ccl->events.on_disconnect)
	  ccl->events.on_disconnect(client,
				    ccl->events.on_disconnect_data);
	/*Break from the loop - and end thread*/
	break;  
      }
  }
#ifdef DEBUG_T
  printf ("Stopping Thread: Client = %d\n", client);
#endif
}

int do_ccl_server(void *arg)
{
  SSL *ssl;
  BIO *newbio;
  fd_set rfd;
  struct timeval delta;
  /* Now i am going to check for new connections */
  int fd = ccl->events.listenfd;

  while (1 == BIO_do_accept(ccl->events.listenbio))
    {
      newbio = BIO_pop(ccl->events.listenbio);
      
#ifdef DEBUG
      {
	char *bcp = NULL;
	struct sockaddr sa;
	  int fd = BIO_get_fd(newbio, NULL);
	  socklen_t salen = sizeof(struct sockaddr);
	  int i;
	  
	  getpeername(fd, &sa, &salen);
	  bcp = BIO_get_accept_port(newbio);
	  printf("** CCL_check_events(): BIO_get_accept_port: %s **\n", bcp);
	  for (i=0; i<salen; i++)
	    printf("%02X ", ((unsigned char *)&sa)[i]);
	  printf("\n");
	  if (!bcp) free(bcp);
	  
      }
#endif
      if (ccl->events.ssl_ctx)
	{
	  ssl = SSL_new(ccl->events.ssl_ctx);
	  SSL_set_fd(ssl, BIO_get_fd(newbio, NULL));
	    BIO_set(newbio, BIO_f_ssl());
	    BIO_set_ssl(newbio, ssl, 0);
	    BIO_set_ssl_mode(newbio, FALSE);
	}
      FD_SET(BIO_get_fd(newbio, NULL), &rfd);
      
      /* Only perform the handshake if this is a genuine connection, 
       * I dont want to block forever, if the connection is not valid */
      if (!select(BIO_get_fd(newbio, NULL) + 1, &rfd, NULL, NULL, &delta))
	BIO_free(newbio);
      else if (!ccl->events.ssl_ctx || 1 == BIO_do_handshake(newbio))
	{
	  gchar *name = NULL;
	  gint size;
	  CCL_client *client = NULL;
	  gint id;
	  
	  if (BIO_read(newbio, &size, sizeof(size))>0){
	    size  = ntohl(size);
	    if (size > 256)  size = 256;  /* sanity check */
	    name = g_malloc0(size);
	    /* read the client name */ 
	    BIO_read(newbio, name, size);
	    
	    id = CCL_client_new(name);
	    g_free(name);
	    
	    client = g_datalist_id_get_data(&(ccl->clients), id);		
	    /* Obtain Client IP address */
	    {
	      socklen_t salen = sizeof(struct sockaddr);
	      struct sockaddr sa;
	      int v1=0, v2=0, v3=0, v4=0;
	      int fd = BIO_get_fd(newbio, NULL);
	      
	      getpeername(fd, &sa, &salen);
	      v1 = (int)(((unsigned char *)&sa)[4]);
	      v2 = (int)(((unsigned char *)&sa)[5]);
	      v3 = (int)(((unsigned char *)&sa)[6]);
	      v4 = (int)(((unsigned char *)&sa)[7]);
	      /*client->ipaddr = *(guint32 *)cp;*/
	      client->ipaddr = (v4<<24) | (v3<<16) | (v2<<8) | v1;
#ifdef DEBUG_A
	      printf("v1=%02X, v2=%02X, v3=%02X, v4=%02X, ipaddr=%08X\n", 
		     v1, v2, v3, v4, client->ipaddr);
#endif
	    }
	    /* If a connection for this client already exists, lets
	     * make sure, that our old connection still exists.
	     * If not, then free it, and set it as disconnected */
	    if (INVALID_SOCKET != client->sockfd) 
	      {
		fd_set wfd;
		
		FD_ZERO(&wfd);
		FD_SET(client->sockfd, &wfd);
		if (!select(client->sockfd + 1, NULL, &wfd, NULL, &delta))
		  {
		    BIO_free(client->bio);
		    client->sockfd = INVALID_SOCKET;
		  }
	      }
	    
	    /*Now open the new connection */
	    if (INVALID_SOCKET == client->sockfd)
	      {
		pthread_t th;
		
		client->bio = newbio;
		client->sockfd = BIO_get_fd(newbio, NULL);
		
		if (ccl->events.on_connect)
		  ccl->events.on_connect(id, ccl->events.on_connect_data);
		
		client->tid = pthread_create(&th, NULL, do_cnxn_proc, (void *)id);
		
	      }
	    else
	      BIO_free(newbio);
	  }
	}
    }
}

/**
 * Check for incoming connections, disconnectons, or messages from the clients.
 *
 * @return FALSE if no events where procesed, TRUE otherwise.
 *
 * When an event occurs, the respective callback is going to be called.
 * This should be called repeatedly in your program if it supports networking.
 */
gboolean
CCL_check_events(void)
{
  /*do nothing*/
  
  return TRUE;
}

/**********************************************************/

static void
_init_db(sqlite3 * db)
{
  sqlite3_stmt *stmt = NULL;
  gboolean DATA = FALSE;
  gboolean CLIENTS = FALSE;
  gboolean MEMBERS = FALSE;
  gboolean PRICES = FALSE;
  gboolean PRODUCTS = FALSE;
  gboolean TARIFS = FALSE;
  gboolean SESSIONSLOG = FALSE;
  gboolean PRODUCTSLOG = FALSE;
  gboolean EXPENSESLOG = FALSE;
  gboolean EMPLOYEES = FALSE;
  gboolean TICKETS = FALSE;
  int retval;

  sqlite3_prepare(db,
		  "select name from"
		  "  (select * from sqlite_master union all"
		  "   select * from sqlite_temp_master)"
		  "where type='table' order by name;", -1, &stmt, NULL);

  while (sqlite3_step(stmt) == SQLITE_ROW)
    {
      if (!g_ascii_strcasecmp((gchar *) sqlite3_column_text(stmt, 0),
				   "tbldata"))
	DATA = TRUE;
      else if (!g_ascii_strcasecmp((gchar *) sqlite3_column_text(stmt, 0),
				   "clients"))
	CLIENTS = TRUE;
      else if (!g_ascii_strcasecmp((gchar *) sqlite3_column_text(stmt, 0),
				   "members"))
	MEMBERS = TRUE;
      else if (!g_ascii_strcasecmp((gchar *) sqlite3_column_text(stmt, 0),
				   "prices"))
	PRICES = TRUE;
      else if (!g_ascii_strcasecmp((gchar *) sqlite3_column_text(stmt, 0),
				   "products"))
	PRODUCTS = TRUE;
      else if (!g_ascii_strcasecmp((gchar *) sqlite3_column_text(stmt, 0),
				   "tarifs"))
	TARIFS = TRUE;
      else if (!g_ascii_strcasecmp((gchar *) sqlite3_column_text(stmt, 0),
				   "sessionslog"))
	SESSIONSLOG = TRUE;
      else if (!g_ascii_strcasecmp((gchar *) sqlite3_column_text(stmt, 0),
				   "productslog"))
	PRODUCTSLOG = TRUE;
      else if (!g_ascii_strcasecmp((gchar *) sqlite3_column_text(stmt, 0),
				   "expenseslog"))
	EXPENSESLOG = TRUE;
      else if (!g_ascii_strcasecmp((gchar *) sqlite3_column_text(stmt, 0),
				   "employees"))
	EMPLOYEES = TRUE;
      else if (!g_ascii_strcasecmp((gchar *) sqlite3_column_text(stmt, 0),
				   "tickets"))
	TICKETS = TRUE;
    }
  
  sqlite3_finalize(stmt);
 
  if (!DATA){
    char *errmsg;

    retval = sqlite3_exec(db,
		 "create table tbldata (\n"
		 "    cid integer default 0 not null,\n"
		 "    id integer default 0 not null,\n"
		 "    key varchar(256) not null,\n"
		 "    value blob,\n"
		 "    unique(cid, id, key));", NULL, NULL, &errmsg);
    /*		 "    unique(cid, id, key));", NULL, NULL, NULL); */
#ifdef DEBUG
    printf("**CCL_Init(): Create 'tbldata' table:  retval[%2d]\n", retval);
    if (retval > 0){
      printf("**CCL_Init(): ** Error [%s]\n", errmsg);
      sqlite3_free(errmsg);
    }
#endif    
  }
  if (!EMPLOYEES){
    gchar *cmd = NULL;
    char *errmsg;

    sqlite3_exec(db,
		 "create table employees (\n"
		 "    id integer primary key,\n"
		 "    empusr varchar(20) not null unique,\n"
		 "    empname varchar(50),\n"
		 "    emppass varchar(20),\n"
		 "    phone varchar(20),\n"
		 "    email varchar(50),\n"
		 "    emplevel integer,\n"
		 "    hiredate integer,\n"
		 "    superid integer,\n"
		 "    flags integer default 0 not null);", NULL, NULL, NULL);
    
    cmd = sqlite3_mprintf("insert into employees (id, empusr, empname, emppass, \n"
			  "emplevel) values (1, %Q, %Q, %Q, %ld);", 
			  "admin", "Administrator", "admin", 0x7FFFFFFF);

    retval = sqlite3_exec(db, cmd, NULL, NULL, &errmsg);
    if (retval>0){
      printf("CCL_Init(): cmd=%s\nerrmsg: %s\n", cmd, errmsg);
      sqlite3_free(errmsg);
    }
    sqlite3_free(cmd);
  }
  if (!CLIENTS)
    sqlite3_exec(db,
		 "create table clients (\n"
		 "    id integer primary key,\n"
		 "    name varchar(128) not null unique,\n"
		 "    status integer not null,\n"
		 "    intervals blob,\n"
		 "    products blob,\n"
		 "    timeout integer not null,\n"
		 "    member integer not null,\n"
		 "    flags integer default 0 not null);", NULL, NULL, NULL);
  if (!MEMBERS)
    sqlite3_exec(db,
		 "create table members (\n"
		 "    id integer primary key autoincrement,\n"
		 "    name varchar(128) not null unique,\n"
		 "    sdate integer not null,\n"
		 "    tarif integer default 0 not null,\n"
		 "    email varchar(128),\n"
		 "    other varchar(128),\n"
		 "    empid integer not null,\n"
		 "    credit integer default 0,\n"
		 "    flags integer default 0 not null);", NULL, NULL, NULL);

  if (!TICKETS)
    sqlite3_exec(db,
		 "create table tickets (\n"
		 "    id integer primary key,\n"
		 "    name varchar(32) not null unique,\n"
		 "    pdate integer not null,\n"
		 "    tarif integer default 0 not null,\n"
		 "    stdate integer not null,\n"
		 "    expdate integer not null,\n"
		 "    empid integer not null,\n"
		 "    faceval integer default 0,\n"
		 "    credit integer default 0,\n"
		 "    flags integer default 0 not null);", NULL, NULL, NULL);

  if (!PRICES)
    sqlite3_exec(db,
		 "create table prices (\n"
		 "    id integer primary key,\n"
		 "    tarifpart integer not null,\n"
		 "    mins integer not null,\n"
		 "    price number not null,\n"
		 "    flags integer default 0 not null,\n"
		 "    unique(tarifpart, mins));", NULL, NULL, NULL);
  if (!PRODUCTS)
    sqlite3_exec(db,
		 "create table products (\n"
		 "    id integer primary key,\n"
		 "    name varchar(128) not null,\n"
		 "    category varchar(128) not null,\n"
		 "    price number not null,\n"
		 "    stock integer,\n"
		 "    flags integer default 0 not null);", NULL, NULL, NULL);

  if (!TARIFS){
    gchar *cmd = NULL;

    sqlite3_exec(db,
		 "create table tarifs (\n"
		 "    id integer primary key,\n"
		 "    tarif integer not null,\n"
		 "    days integer not null,\n"
		 "    stime time not null,\n"
		 "    hourprice number not null,\n"
		 "    incprice number not null,\n"
		 "    fafter integer default 10 not null,\n"
		 "    tname varchar(30), \n"
		 "    flags integer default 0 not null);", NULL, NULL, NULL);
    /*Default Tariff*/
    cmd = sqlite3_mprintf("insert into tarifs (id, tarif, days, \n"
			  "stime, hourprice, incprice, fafter, tname) \n"
			  "values (1, 1, 127, 00:00, 6000, 100, 10, %Q);", 
			  "Default");

    sqlite3_exec(db, cmd, NULL, NULL, NULL);
    sqlite3_free(cmd);
  }

  if (!SESSIONSLOG)
    sqlite3_exec(db,
		 "create table sessionslog (\n"
		 "    id integer primary key,\n"
		 "    client integer not null,\n"
		 "    empid integer not null,\n"
		 "    member integer not null,\n"
		 "    stime integer not null,\n"
		 "    etime integer not null,\n"
		 "    time integer not null,\n"
		 "    price integer not null,\n"
		 "    intervals blob not null,\n"
		 "    flags integer default 0 not null);", NULL, NULL, NULL);
  if (!PRODUCTSLOG)
    sqlite3_exec(db,
		 "create table productslog (\n"
		 "    id integer primary key,\n"
		 "    session integer not null,\n"
		 "    client integer not null,\n"
		 "    empid integer not null,\n"
		 "    member integer not null,\n"
		 "    product integer not null,\n"
		 "    amount integer not null,\n"
		 "    price number not null,\n"
		 "    time integer not null,\n"
		 "    flags integer default 0 not null);", NULL, NULL, NULL);
  if (!EXPENSESLOG)
    sqlite3_exec(db,
		 "create table expenseslog (\n"
		 "    id integer primary key,\n"
		 "    description varchar(127) not null,\n"
		 "    time integer not null,\n"
		 "    cash integer not null,\n"
		 "    empid integer not null,\n"
		 "    flags integer default 0 not null);", NULL, NULL, NULL);
}

static gint
_loadMembersCB(void *ptr, int argc, char **argv, char **colnames)
{
  gint id = atoi(argv[0]);
  CCL_member *member = g_new0(CCL_member, 1);

#ifdef DEBUG
  {
    int i; 
    printf("_LoadMembersCB(): argc=%d\n", argc);
    for (i=0; i<argc; i++) 
      printf("[%d = %s]", i, argv[i]);
    printf("\n");
  }
#endif
  _CCL_member_init(member, argv[1]);
  g_datalist_id_set_data_full(&(ccl->members), id, member, _destroy_member);
  _CCL_member_restore(id);

  return 0;
}

static gint
_loadEmployeesCB(void *ptr, int argc, char **argv, char **colnames)
{
  gint id = atoi(argv[0]);
  CCL_employee *employee = g_new0(CCL_employee, 1);
  int superid, lvl;

  lvl = (argv[6]==NULL? 0: atoi(argv[6]));
  superid = (argv[8]==NULL? 0: atoi(argv[8]));


#ifdef DEBUG
  {
    int i; 
    printf("_LoadEmployeesCB(): argc=%d\n", argc);
    for (i=0; i<argc; i++) 
      printf("[%d = %s]", i, argv[i]);
    printf("\n");
  }
#endif
  /* _CCL_employee_init1(employee, argv[1]);*/
  _CCL_employee_init(employee, argv[1], argv[2], argv[3], argv[4], argv[5],
		      lvl, superid);
  g_datalist_id_set_data_full(&(ccl->employees), id, employee, _destroy_employee);
  _CCL_employee_restore(id);

  return 0;
}

static gint
_loadClientsCB(void *ptr, int argc, char **argv, char **colnames)
{
  gint id = atoi(argv[0]);
  CCL_client *client = g_new0(CCL_client, 1);

  _CCL_client_init(client, argv[1]);
  g_datalist_id_set_data_full(&(ccl->clients), id, client, _destroy_client);
  _CCL_client_restore(id);

  return 0;
}

static void
_shutdown_client_connection(gint client)
{
  CCL_client *c = (CCL_client *) g_datalist_id_get_data(&ccl->clients,
							client);

  g_assert(c);

  _shutdown_connection(c);
}

static void
_FindClientByFdFunc(GQuark key_id, gpointer data, gpointer user_data)
{
  CCL_client *client = (CCL_client *) data;
  gint *fd_cid = (gint *) user_data;

  if (fd_cid[0] == client->sockfd)
    fd_cid[1] = key_id;
}

static void
_ShutdownClientConnectionFunc(GQuark key_id, gpointer data, gpointer user_data)
{
  CCL_client *client = (CCL_client *) data;

  _shutdown_connection(client);
}

static gint
_cert_verify(gint ok, X509_STORE_CTX * x509_ctx)
{
  gint error = X509_STORE_CTX_get_error(x509_ctx); 

  ERR_load_X509_strings();
  if (!ok)
    printf("error: %d\n%s\n", error, X509_verify_cert_error_string(error));

  return 1;
}

static gint
_passwd_cb(gchar * buf, gint num, gint rw, void * password)
{
  if (!password) password = "_nopassword";

  strncpy(buf, (gchar *) password, num);
  buf[num - 1] = '\0';

  return strlen(buf);
}

static SSL_CTX *
_initialize_ssl_ctx(const char * cafile, const gchar * keyfile,
		    const gchar * certpass, gint * err)
{
  SSL_CTX *ctx;

  if (err) *err = CCL_ERROR_NO_ERROR;

  if (ccl->events.certpass) g_free(ccl->events.certpass);
  ccl->events.certpass = certpass ? g_strdup(certpass) : NULL;

  /* Create our context*/
  ctx = SSL_CTX_new(SSLv23_server_method());
  SSL_CTX_set_default_passwd_cb(ctx, _passwd_cb);
  SSL_CTX_set_default_passwd_cb_userdata(ctx, ccl->events.certpass);

  /* Load our keys and certificates*/
  SSL_CTX_use_certificate_file(ctx, keyfile, SSL_FILETYPE_PEM);
  
  if (1 != SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM))
    {
      if (err) *err = CCL_ERROR_BAD_PASSWORD;
      goto error;
    }
 
  if (1 != SSL_CTX_check_private_key(ctx))
    { 
      if (err) *err = CCL_ERROR_BAD_PASSWORD;
      goto error;
    }
  
  if (1 != SSL_CTX_load_verify_locations(ctx, cafile, NULL))
    {
      if (err) *err = CCL_ERROR_COULD_NOT_LOAD_VL;
      goto error;
    }

  SSL_CTX_set_verify(ctx,
		     SSL_VERIFY_PEER|SSL_VERIFY_CLIENT_ONCE
		     |SSL_VERIFY_FAIL_IF_NO_PEER_CERT, _cert_verify);
  
  return ctx;

error:
  SSL_CTX_free(ctx);

  return NULL;
}

static gint
_greatest_product_id()
{
  gchar *cmd = NULL;
  sqlite3_stmt *stmt = NULL;
  gint id = -1;

  cmd = sqlite3_mprintf("select max(id) from products;");
  sqlite3_prepare(ccl->db, cmd, -1, &stmt, NULL);
  sqlite3_free(cmd);

  if (sqlite3_step(stmt) == SQLITE_ROW)
    id = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);

  return id;
}


