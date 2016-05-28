// cclpfox.h
//

#ifndef CCLPFOX_H
#define CCLPFOX_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "update.h"

#include "cmds.h"
#include "update.h"

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

class CCLPFox : public FXObject
{
FXDECLARE(CCLPFox)
private:
  static CCLPFox   *cclp;
protected:
  FXbool	    active;
  FXbool	    networking;
  time_t	    stime;
  int		    timeout;
  int               ack_assist;
  UpdateInfo        cui;
public:
  CCLPFox();
  ~CCLPFox();
public:
  void initCCLP();
  FXbool initCCLP(const char *cafile, const char *certfile,
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
  void setProducts(FXuint owed);
  void showMessage(char *message);
  void execCommand(FXuint cmd,const void *data,FXuint datasize);
  void unlockWithPass(int id,FXString password);
  void unlockWithPass(FXString login,FXString password);
  void reportPrinting(char *cupsLogLine, int lnLen);
  void exitProgram();
  void setPassword(FXuchar * digest);
  void initPrintPolling();
  void pollPrinting();
  void askForHelp();
  long storeUpdateChunk(char *chunk, long len);
  long doUpdate();
  long authUpdate(void *hdr, long len);
  char *updata;
  long process_update_data();
public:
  long onTimer(FXObject*,FXSelector,void*);
  long onCheckEvents(FXObject*,FXSelector,void*);
  long onPollPrinting(FXObject*, FXSelector, void*);
public:
  enum {
    ID_TIMER = 0,ID_CHECKEVENTS,
    ID_LAST, ID_POLLPRINT
  };
  int can_print_poll;
  int cups_fd;
  struct stat fst, tstat; 
  char *cupslogfile;
};
#endif
