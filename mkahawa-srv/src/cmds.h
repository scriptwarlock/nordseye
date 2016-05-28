/* 
 * cmds.h
 * commands sent by the server to the client from client to server
 */

#ifndef CMDS_H
#define CMDS_H
/* Commands sent by the server */
#define CS_STOP		  1UL   /* End session */
#define CS_START	  2UL   /* Start session */
#define CS_PAUSE	  3UL   /* Pause session */
#define CS_RESUME	  4UL   /* Resume last session */
#define CS_MONITOROFF	  5UL   /* Turn off client's monitor */
#define CS_SETTIMEOUT	  6UL   /* Set the clients timeout */
#define CS_SHUTDOWN	  7UL   /* Shutdown client's computer */
#define CS_REBOOT	  8UL   /* Reboot client's computer */
#define CS_SETTIME	  9UL   /* Set client's time */
#define CS_SETOWED	  10UL  /* Set client's owed */
#define CS_SETADDITIONAL  11UL  /* Set client's additional owed */
#define CS_QUITCLIENT	  12UL  /* Tells the client to close itself */
#define CS_DISPLAYMESSAGE 13UL  /* Shows a message */
#define CS_UNLOCKSCREEN	  14UL	/* Unlock the screen */
#define CS_LOCKSCREEN	  15UL	/* Lock the screen */
#define CS_ENABLEPASSWORDEDIT 16UL/* Enable password edit */
#define CS_ALLOWMEMBERLOGIN 17UL/* Enable member login */
#define CS_ALLOWUSERLOGIN 18UL	/* Enable user login */
#define CS_CHATSERVER     19UL  /* Chat from server*/
#define CS_CALLASSIST     20UL  /* Respond to call assistant */
#define CS_ENABLEASSIST   21UL  /* Respond to call assistant */
#define CS_UPDATE         22UL  /* Start update */
#define CS_UPDATEDATA     23UL  /* Send update data */
#define CS_UPDATEEND      24UL  /* End update */ 
#define CS_SETADMINPASS   25UL  /* Set client admin password */ 
#define CS_REQVERSION     26UL  /* Request Version */
#define CS_ALERTCLIENT    27UL  /* Send an alert / transient message */
#define CS_ALLOWTICKETLOGIN    28UL  /* enable login by ticket */
#define CS_TICKETLOGIN    29UL  /* login by ticket */
#define CS_SETPOLLINTERVAL   30UL  /* Set poll interval */
#define CS_WARNCLIENT     31UL  /* Set poll interval */


/* Commands sent by the client */
#define CC_USEREXIT	  1UL	/* The user ended the session */
#define CC_USERLOGIN	  2UL	/* The user wants to start a new session */
#define CC_GETSTATUS	  4UL	/* Request the status (time, owed, etc) */
#define CC_GETTIME	  5UL	/* Request the used time */
#define CC_GETOWED	  6UL	/* Request the amount owed by the user */
#define CC_GETTIMEOUT	  7UL	/* Request the timeout */
#define CC_MEMBERLOGIN	  8UL	/* Login with member id */
#define CC_SETMEMBERPASSWORD 9UL/* Change the password for this member */
#define CC_MEMBERLOGINWITHNAME 10UL /* Login with member name */
#define CC_USERPRINTED    11UL  /* Report user printing */
#define CC_CHATCLIENT     12UL  /* Chat from client */
#define CC_CALLASSIST     13UL  /* Call assistant */
#define CC_UPDATE         14UL  /* Respond to CS_UPDATE */
#define CC_UPDATEDATA     15UL  /* Respond to CS_UPDATEDATA */
#define CC_UPDATEEND      16UL  /* Respond to CS_UPDATEEND */
#define CC_VERSION        17UL  /* Respond to CS_REQVERSION */
#define CC_ALERTSERVER    18UL  /* Send alert to Server */
#define CC_TICKETLOGIN    19UL  /* Start logging in by ticket */
#define CC_MAXCMDNR       19UL  /* Maximum Command ID*/


#define MK_MAJOR_VER 1
#define MK_MINOR_VER 0
#define MK_RELEASE   2

unsigned long MkahawaVersion = (((MK_MAJOR_VER << 8)|MK_MINOR_VER) << 8) | MK_RELEASE;

#endif /* ifndef CMDS_H */
