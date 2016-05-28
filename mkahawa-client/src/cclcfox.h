// cclcfox.h
//

#ifndef CCLCFOX_H
#define CCLCFOX_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "update.h"
#include "cmds.h"

#include <cclc.h>

#ifdef HAVE_GETTEXT
# include <libintl.h>
# include <locale.h>
# define _(String) gettext(String)
# define N_(String) (String)
#else
# define _(String) (String)
# define N_(String) (String)
#endif

#define OPMODE_POSTPAY   1
#define OPMODE_TICKET    2
#define OPMODE_MEMBER    4

#define MEMBER_LOGIN_NAME 1
#define MEMBER_LOGIN_PWD  2

class CCLCFox : public FXObject
{
FXDECLARE(CCLCFox)
private:
  static CCLCFox   *cclc;
protected:
  FXbool	    active;
  FXbool	    networking;
  time_t	    stime;
  int		    client_timeout;
  int               time_warned;
  int               ack_assist;
  unsigned int      poll_interval;
  UpdateInfo        cui;
public:
  CCLCFox();
  ~CCLCFox();
public:
  void initCCLC();
  FXbool initCCLC(const char *cafile, const char *certfile,
		  const char * certpass = NULL);
  FXbool initNetworking(const char *sname = NULL,FXushort sport = 0,
			const char *myname = NULL);
  FXbool shutdownNetworking();
  FXbool isLocked();
  FXbool isInfoShown();
  void showInfo();
  void hideInfo();
  void start();
  void stop();
  void userExit();
  void userStart();
  void resume();
  void lockScreen();
  void unlockScreen();
  void shutdownSystem();
  void rebootSystem();
  void turnOffMonitor();
  void setOwed(FXuint owed);
  void setClientPollInterval(FXuint itvl);
  void setProducts(FXuint owed);
  void showMessage(void *message);
  void execCommand(FXuint cmd,const void *data,FXuint datasize);
  void unlockWithPass(int id,FXString password);
  void unlockWithPass(FXString tktstr);
  void unlockWithPass(FXString login,FXString password);
  void reportPrinting(char *cupsLogLine, int lnLen);
  void exitProgram();
  void setPassword(FXuchar * digest);
  void initPrintPolling();
  void pollPrinting();
  void askForHelp();
  long storeUpdateChunk(char *chunk, long len);
  long doUpdate();
  long authUpdate(void *hdr, unsigned long len);
  char *updata;
  long process_update_data();
  void setOpMode(unsigned int opmode) { op_mode = opmode; }
  unsigned long getOpMode() { return op_mode; };
  void setLoginMode(unsigned int loginmode);
  unsigned long getLoginMode() { return login_mode; }
  void setMemberLoginState(unsigned int state) { mbr_login_state = state; }
  unsigned long getMemberLoginState() { return mbr_login_state; }
public:
  long onTimer(FXObject*,FXSelector,void*);
  long onCheckEvents(FXObject*,FXSelector,void*);
  long onPollPrinting(FXObject*, FXSelector, void*);

public:
  enum {
    ID_TIMER = 0,ID_CHECKEVENTS,
    ID_LAST, ID_POLLPRINT
  };
  int          can_print_poll;
  int          cups_fd;
  struct stat  fst, tstat; 
  char        *cupslogfile;
  unsigned int op_mode;
  unsigned int login_mode;
  unsigned int mbr_login_state;
};
#endif

