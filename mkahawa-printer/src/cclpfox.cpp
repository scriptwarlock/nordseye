#include <fox-1.6/fx.h>
using namespace FX;

#include "cclpfox.h"
#include "gui.h"
#include "locker.h"
#ifdef WIN32
#  include <windows.h>
#  include <winuser.h>
#endif

//#define DEBUG 1
//#define DEBUG_PRINT 1
extern CCLPFox *cclpfox;
extern ClientWin *clientwin;
extern Locker *locker;
extern Grabber *grabber;

struct CHUNK *chunk;

void
onEventCallback(FXuint cmd,void * data,FXuint size,void * userdata)
{
  cclpfox->execCommand(cmd,data,size);
}

void
onDisconnectCallback(void * userdata)
{
  cclpfox->shutdownNetworking();
}

FXDEFMAP(CCLPFox) CCLPFoxMap[] =
{
  FXMAPFUNC(SEL_TIMEOUT,CCLPFox::ID_TIMER,CCLPFox::onTimer),
  FXMAPFUNC(SEL_TIMEOUT,CCLPFox::ID_CHECKEVENTS,CCLPFox::onCheckEvents),
  FXMAPFUNC(SEL_TIMEOUT,CCLPFox::ID_POLLPRINT,CCLPFox::onPollPrinting)
};

FXIMPLEMENT(CCLPFox,FXObject,CCLPFoxMap,ARRAYNUMBER(CCLPFoxMap))

CCLPFox *CCLPFox::cclp = NULL;

CCLPFox::CCLPFox()
{
  if (CCLPFox::cclp)
    fxerror("An instance of Client is already loaded\n");
  //fxerror("Ya existe una instancia de CCLPFox\n");
  active = FALSE;
  stime = 0;
  networking = FALSE;
  timeout = 0;
  clientwin->getApp()->addTimeout(this,CCLPFox::ID_TIMER,1000);
  CCLPFox::cclp = this;
  clientwin->disableHelpBtn();
  updata = NULL;
}

CCLPFox::~CCLPFox()
{
  shutdownNetworking();
  CCLC_shutdown();
  CCLPFox::cclp = NULL;
}

void
CCLPFox::initCCLP()
{
  CCLC_init();
  CCLC_set_on_event_callback(onEventCallback,NULL);
  CCLC_set_on_disconnect_callback(onDisconnectCallback,NULL);
}

FXbool
CCLPFox::initCCLP(const char * cafile,const char * certfile,
		  const char * certpass)
{
  int error;
  
  initCCLP();

  if (!CCLC_SSL_init(cafile,certfile,certpass,&error)) {
    switch (error) {
    case CCLC_ERROR_BAD_PASSWORD:
      fxerror("[E]Bad certificate password\n");
      exit(1);
      break;
    default:
      fxmessage("[!]Couldn't init SSL\n");
      return FALSE;
      break;
    }
  }
  
  return TRUE;
}

FXbool
CCLPFox::initNetworking(const char * server,FXushort port, const char * myname)
{
  static const char *sname = server;
  static FXushort sport = port;
  static const char *cname = myname;
  int error;

  if (CCLC_networking_init(sname,sport,cname,&error)) {
    clientwin->getApp()->addTimeout(this,CCLPFox::ID_CHECKEVENTS,100);
    clientwin->getApp()->addTimeout(this,CCLPFox::ID_POLLPRINT,5000);
    CCLC_send_cmd(CC_GETSTATUS,NULL,0);
    networking = TRUE;
  } else {
    switch (error) {
      case CCLC_ERROR_CONNECT_FAIL:
	fxmessage("[!]Could not connect to the server\n");
	networking = FALSE;
	break;
    }
  }

  return networking;
}

FXbool
CCLPFox::shutdownNetworking()
{
  CCLC_networking_shutdown();
  clientwin->setOwed("--.--");
  clientwin->setProducts("--.--");
  networking = FALSE;
  
  return TRUE;
}

FXbool
CCLPFox::isLocked()
{
  return locker->shown();
}

FXbool
CCLPFox::isInfoShown()
{
  return grabber->shown();
}

void
CCLPFox::showInfo()
{
  grabber->show();
  clientwin->show();
}

void
CCLPFox::hideInfo()
{
  clientwin->hide();
  grabber->hide();
}

void
CCLPFox::start()
{
  stime = time(NULL);
  /*clientwin->setTime("00:00");
  //clientwin->setOwed("0.00");
  //clientwin->setProducts("0.00");
  //clientwin->enableHelpBtn();*/
  timeout = 0;
  active = TRUE;
  initPrintPolling();
  clientwin->getApp()->addTimeout(this,CCLPFox::ID_POLLPRINT,5000);
}

void
CCLPFox::stop()
{
  active = FALSE;
/*  clientwin->disableHelpBtn();*/
/* Close the print polling */
  if (can_print_poll){
    can_print_poll = 0;
    if (cups_fd>0) close(cups_fd);
    cups_fd = -1;  
  }
}

void
CCLPFox::userExit()
{
  if (active){
    CCLC_send_cmd(CC_USEREXIT,NULL,0);
#ifdef DEBUG
    printf("User Exited\n");
#endif
  }
  else{
    /*lockScreen();*/
#ifdef DEBUG
    printf ("Only locking the screen\n");
#endif
  }
}

void
CCLPFox::resume()
{
  active = TRUE;
}

void
CCLPFox::lockScreen()
{
  locker->lock();
  hideInfo();
}

void
CCLPFox::unlockScreen()
{
  locker->unlock();
  showInfo();
}

void
CCLPFox::shutdownSystem()
{
#ifndef WIN32
  system("/sbin/halt");
#else
  HANDLE hToken;
  TOKEN_PRIVILEGES tkp;

  // Get a token for this process.
  OpenProcessToken(GetCurrentProcess(),
		   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,&hToken);
  // Get the LUID for the shutdown privilege.
  LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,&tkp.Privileges[0].Luid);
  tkp.PrivilegeCount = 1;	// one privilege to set
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  // Get the shutdown privilege for this process.
  AdjustTokenPrivileges(hToken,FALSE,&tkp,0,(PTOKEN_PRIVILEGES) NULL,0);

  OSVERSIONINFO osversion;

  osversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&osversion);
  if (osversion.dwPlatformId == VER_PLATFORM_WIN32_NT)	// NT/2000/XP
    ExitWindowsEx(EWX_POWEROFF | EWX_FORCE,0);
  else
    ExitWindowsEx(EWX_POWEROFF | EWX_FORCE,0);
#endif
}

void
CCLPFox::rebootSystem()
{
#ifndef WIN32
  system("/sbin/reboot");
#else
  HANDLE hToken;
  TOKEN_PRIVILEGES tkp;

  // Get a token for this process.
  OpenProcessToken(GetCurrentProcess(),
		   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,&hToken);
  // Get the LUID for the shutdown privilege.
  LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,&tkp.Privileges[0].Luid);
  tkp.PrivilegeCount = 1;	// one privilege to set
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  // Get the shutdown privilege for this process.
  AdjustTokenPrivileges(hToken,FALSE,&tkp,0,(PTOKEN_PRIVILEGES) NULL,0);

  OSVERSIONINFO osversion;

  osversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&osversion);
  if (osversion.dwPlatformId == VER_PLATFORM_WIN32_NT)	// NT/2000/XP
    ExitWindowsEx(EWX_REBOOT | EWX_FORCE,0);
  else				// 9x/Me
    ExitWindowsEx(EWX_REBOOT | EWX_FORCE,0);
#endif
}

void
CCLPFox::turnOffMonitor()
{
#ifndef WIN32
  system("xset dpms force off");
#else
  HWND hWnd = (HWND) (clientwin->id());

  SendMessage(hWnd,WM_SYSCOMMAND,SC_MONITORPOWER,2);
#endif
}

void
CCLPFox::showMessage(char * message)
{
  //FXMessageBox::information(grabber->getRoot(),MBOX_OK,_("Message"),message);

  FXDialogBox dialog(clientwin->getApp(),_("Message from Operator"), 
		     DECOR_TITLE|DECOR_BORDER, 200, 200);
  FXLabel lbl2(&dialog,_(message));

  FXButton okbtn(&dialog,_("   Ok   "),NULL,&dialog,FXDialogBox::ID_ACCEPT);
  FXButton cancelbtn(&dialog,_("Cancel"),NULL,&dialog,FXDialogBox::ID_CANCEL);

  dialog.show();
}

void
CCLPFox::setOwed(FXuint owed)
{
  char buf[64];
  double lowed;
  
#ifdef DEBUG
  printf ("Owed = %d\n", owed);
#endif
  owed = (owed+50) / 100; 
  lowed = (double)owed;  
  snprintf(buf,64,"%.2f", lowed);
  clientwin->setOwed(buf);
}

void
CCLPFox::setProducts(FXuint owed)
{
  char buf[64];

  snprintf(buf,64,"%.2f",owed / 100.0);
  clientwin->setProducts(buf);
}

void
CCLPFox::userStart()
{
  CCLC_send_cmd(CC_USERLOGIN,NULL,0);
#ifdef DEBUG
  printf ("UserStart a session\n");
#endif
}

void
CCLPFox::unlockWithPass(int id,FXString password)
{
  char val[sizeof(id) + CCLC_MD5_DIGEST_LENGTH * sizeof(FXuchar)];
  FXuchar digest[CCLC_MD5_DIGEST_LENGTH];

  CCLC_MD5((FXuchar*)(password.text()),password.length(),digest);
  ((FXuint*)val)[0] = CCLC_htonl(id);
  memcpy(((FXuint*)val)+1,digest,CCLC_MD5_DIGEST_LENGTH); 
  
  CCLC_send_cmd(CC_MEMBERLOGIN,val,sizeof(val) * sizeof(char));
}


void
CCLPFox::unlockWithPass(FXString login,FXString password)
{
  const char *login_name = login.text();
  char val[strlen(login_name) * sizeof(char) + 1
	   + CCLC_MD5_DIGEST_LENGTH * sizeof(FXuchar)];
  FXuchar digest[CCLC_MD5_DIGEST_LENGTH];

  CCLC_MD5((FXuchar*)(password.text()),password.length(),digest);
  memcpy(val,login_name,strlen(login_name) + 1);
  memcpy(val + strlen(login_name) + 1,digest,CCLC_MD5_DIGEST_LENGTH); 
  
  CCLC_send_cmd(CC_MEMBERLOGINWITHNAME,val,sizeof(val) * sizeof(char));
}

void
CCLPFox::reportPrinting(char *cupsLogLine, int lnLen)
{
  CCLC_send_cmd(CC_USERPRINTED, cupsLogLine, lnLen);
#ifdef DEBUG
  printf("Printing Reported\n");
#endif
}

void
CCLPFox::exitProgram()
{
#ifdef WIN32
  OSVERSIONINFO osversion;

  osversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&osversion);
  if (osversion.dwPlatformId == VER_PLATFORM_WIN32_NT) {	// NT/2000/XP
    HWND hwnd = FindWindow("Shell_traywnd",NULL);

    EnableWindow(hwnd,TRUE);
    HKEY hk;
    const char *key =
      "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System";
    if (RegOpenKey(HKEY_CURRENT_USER,key,&hk) != ERROR_SUCCESS)
      RegCreateKey(HKEY_CURRENT_USER,key,&hk);
    RegDeleteValue(hk,"DisableTaskMgr");
  }
#endif
  exit(0);
}

void
CCLPFox::execCommand(FXuint cmd,const void *data,FXuint datasize)
{
  FXuint hdata = 0;
  long resp, chsize;

  switch (cmd) {
    case CS_STOP:
      /*stop();*/
      break;
    case CS_START:
      /*start();*/
      break;
    case CS_RESUME:
      /*resume();*/
      break;
    case CS_MONITOROFF:
      turnOffMonitor();
      break;
    case CS_SHUTDOWN:
      shutdownSystem();
      break;
    case CS_REBOOT:
      rebootSystem();
      break;
    case CS_SETOWED:
      hdata = CCLC_ntohl(*(FXuint *) data);
      setOwed(hdata);
      break;
    case CS_SETADDITIONAL:
      hdata = CCLC_ntohl(*(FXuint *) data);
      setProducts(hdata);
      break;
    case CS_SETTIME:
      stime = time(NULL) - CCLC_ntohl(*((int *) data));
      break;
    case CS_SETTIMEOUT:
      timeout = CCLC_ntohl(*((int *) data));
      break;
    case CS_DISPLAYMESSAGE:
      showMessage((char *) data);
      break;
    case CS_QUITCLIENT:
      exitProgram();
      break;
    case CS_UNLOCKSCREEN:
      unlockScreen();
#ifdef DEBUG
      printf("UnLocking Screen\n");
#endif
      break;
    case CS_LOCKSCREEN:
      //lockScreen();
#ifdef DEBUG
      printf("Locking Screen\n");
#endif
      break;
    case CS_ENABLEPASSWORDEDIT:
      clientwin->setPasswordEnabled(CCLC_ntohl((*((int *) data))));
      break;
    case CS_ALLOWMEMBERLOGIN:
      locker->allowMemberLogin(bool(CCLC_ntohl((*((int *) data)))));
      break;
    case CS_ALLOWUSERLOGIN:
      locker->allowUserLogin(bool(CCLC_ntohl((*((int *) data)))));
      break;
    case CS_ENABLEASSIST:
      clientwin->enableAssist(bool(CCLC_ntohl((*((int *) data)))));
      break;
    case CS_CHATSERVER:
#ifdef DEBUG
      printf("Start Chatting\n");
#endif
      FXMessageBox::information(clientwin,MBOX_OK,_("Message"), (char *)data);
      break;
    case CS_CALLASSIST:
#ifdef DEBUG
      printf("Assistant Acknowledged\n");
#endif
      ack_assist = 1;
      clientwin->enableHelpBtn();
      break;
    case CS_UPDATE:
      if (authUpdate((char *)data, (long)datasize)){
	resp = CCLC_htonl(1);
      }
      else{
	resp = CCLC_htonl(0);
      }
      CCLC_send_cmd(CC_UPDATE, &resp, sizeof(resp));
#ifdef DEBUG
      printf("Update Acknowledged: resp=%d\n", resp);
#endif
      break;
    case CS_UPDATEDATA:
      chunk = (struct CHUNK*)data;
      
      resp = chunk->pos;//CCLC_ntohl(((FXuint*) data)[0]);
      chsize = chunk->blen;//CCLC_ntohl(((FXuint*) data)[1]);
      if (storeUpdateChunk((char *)data, (long)datasize)){
	resp += chsize;
	resp = CCLC_htonl(resp);
      }
      else{
	resp = CCLC_htonl(resp);
      }
      CCLC_send_cmd(CC_UPDATEDATA, &resp, sizeof(resp));
#ifdef DEBUG
      printf("Update Data Acknowledged\n");
#endif
      //wait awhile
      usleep(1000);
      break;
    case CS_UPDATEEND:
      long resp;
      char msgstr[100], *cp;
      
#ifdef DEBUG
      printf("Update End\n");
#endif
      if (doUpdate()){ //success
	resp = 1;
	resp = CCLC_htonl(resp);
	CCLC_send_cmd(CC_UPDATEEND, &resp, sizeof(resp));
      }
      else{ //failed
	resp = 0;
	cp = msgstr;
	*cp = CCLC_htonl(resp);
	cp += sizeof(long);
	snprintf(cp, 100, "Was unable to write update file");
	CCLC_send_cmd(CC_UPDATEEND, msgstr, sizeof(resp)+strlen(cp));
      }
#ifdef DEBUG
      printf("Update End Acknowledged\n");
#endif
      break;
  }
}


long
CCLPFox::onTimer(FXObject*,FXSelector,void*)
{
  if (!networking)
    initNetworking();
  /*if (active) {
    char buf[8];
    int usedtime = time(NULL) - stime;
    int hours = usedtime / 3600;
    int mins = (usedtime % 3600) / 60;

    snprintf(buf,8,"%.2d:%.2d",hours,mins);
    clientwin->setTime(buf);
    CCLC_send_cmd(CC_GETOWED,NULL,0);
    if (0 < timeout && usedtime > timeout)
      userExit();
  }*/
  clientwin->getApp()->addTimeout(this,CCLPFox::ID_TIMER,5000);

  return 1;
}

long
CCLPFox::onPollPrinting(FXObject*,FXSelector,void*)
{
  int poll_len = 5000; //Normal polling is every 5 secs

  if (!can_print_poll) {
#ifdef DEBUG
    printf ("Unable to print poll. Retry initializing\n");
#endif
    poll_len = 10000;
    initPrintPolling();   /*always try to initialize po*/
    clientwin->getApp()->addTimeout(this,CCLPFox::ID_POLLPRINT,poll_len);
    return 0;
  }
  pollPrinting();
  clientwin->getApp()->addTimeout(this,CCLPFox::ID_POLLPRINT,poll_len);
#ifdef DEBUG
  /*    printf("Polled Printing: [ Active ] Length=%d\n", poll_len);*/
#endif
    /* }*/
#ifdef DEBUG
  if (!active){
    printf("Polled Printing: [Inactive] Length=%d\n", poll_len);
  }
#endif
  
  return 1;
}


long
CCLPFox::onCheckEvents(FXObject*,FXSelector,void*)
{
  CCLC_check_events();
  clientwin->getApp()->addTimeout(this,CCLPFox::ID_CHECKEVENTS,100);

  return 1;
}

void
CCLPFox::setPassword(FXuchar digest[2*CCLC_MD5_DIGEST_LENGTH])
{
  CCLC_send_cmd(CC_SETMEMBERPASSWORD,digest,2*CCLC_MD5_DIGEST_LENGTH);
}

#define BUFSIZE 1024
void 
CCLPFox::pollPrinting()
{
  struct stat tstat;
  int n, lnlen, retval=0;
  char buf[BUFSIZE+2], *cp;

  memset(&tstat, 0, sizeof(struct stat));
  //retval = fstat(cups_fd, &tstat);
  retval = stat(cupslogfile, &tstat);
  if (retval){
    /* Log file has disappeared */
    return;
  }
  if (tstat.st_ino != fst.st_ino){
    /* Log file has been renewed */
    initPrintPolling();
    fst.st_size = 0;
  }

  if (tstat.st_size > fst.st_size){
    n = tstat.st_size - fst.st_size;
    if (n == 0) return;
    if (n < 0){ /* the log file has been zeroed */
      n = tstat.st_size;
      lseek(cups_fd, 0, SEEK_SET);
    }
    else
      lseek(cups_fd, fst.st_size, SEEK_SET);
    n = read(cups_fd, buf, (n>BUFSIZE? BUFSIZE: n));
    buf[n+1] = 0;   //end the string
    cp = rindex(buf, '\n');
    lnlen = (cp - buf) + 1;
    if (lnlen > 0){
#ifdef DEBUG_PRINT
      int i;
      printf("Line Sent: \n");
      for (i=0; i<lnlen; i++)
	printf("%c", buf[i]);
      printf("\n\n");
#endif
      CCLC_send_cmd(CC_USERPRINTED, buf, lnlen);
    //manipulate to recover partially read line
      if (lnlen < n) tstat.st_size -= (n-lnlen);
      //update fst
      fst = tstat;
    }
  }
}

void 
CCLPFox::initPrintPolling()
{
  int n;
  can_print_poll = 0;
  cupslogfile = "/var/log/cups/page_log";

  cups_fd = -1;
  cups_fd = open(cupslogfile, O_RDONLY);
  if (cups_fd > 0) {
    can_print_poll = 1;
    /* get the initial size of cups */
    memset(&fst, 0, sizeof(fst));
    //n = fstat(cups_fd, &fst);  
    n = stat(cupslogfile, &fst);  
  }
  else{
#ifdef DEBUG
    printf("InitPrintPolling: Unable to Start Polling\n");
#endif
  }
}

/* Ask for assistance */
void 
CCLPFox::askForHelp()
{
  ack_assist = 0;
  //send the help message
  CCLC_send_cmd(CC_CALLASSIST,NULL,0);
  /*CCLC_send_cmd(CC_CALLASSIST,buf,10);*/
#ifdef DEBUG
  printf("askForHelp: Called the assistant\n");
#endif
}

void 
print_update_info(UpdateInfo *ui)
{
  printf("update_name: [%s]\n", ui->update_name);
  printf("dest_fname: [%s]\n", ui->dest_fname);
  printf("dest_path: [%s]\n", ui->dest_path);
  printf("File Size: [%ld]\n", ui->fsize);
  /*
  char dest_fname[24];
  char dest_path[128];
  long fsize;
  long fmode;
  long vernr;
  char verstr[16];
  long key;
  long lbuf;
  char cbuf[24];
  */
}

long
CCLPFox::authUpdate(void *hdr, long len)
{
  UpdateInfo *ui;

  if (len < sizeof(UpdateInfo))
    return 0; //size checking

  memset(&cui, 0, sizeof(UpdateInfo));
  ui = (UpdateInfo *)hdr;
  cui = *ui;
  //authenticate & verify data
#ifdef DEBUG
  print_update_info(ui);
#endif
  if (cui.fsize > 4000000) {
#ifdef DEBUG
    printf("authUpdate(): file is too large\n");
#endif
    return 0;
  }
  //allocate memory
  if (updata) {
    free(updata);
    updata = NULL;
  }
      
  updata = (char *)malloc(cui.fsize);
  if (!updata){
#ifdef DEBUG
    printf("authUpdate(): Unable to allocate memory\n");
#endif
    return 0;
  }
  
  return 1;
}

long 
CCLPFox::storeUpdateChunk(char *dat, long datlen)
{
  memcpy((updata+chunk->pos), chunk->buf, chunk->blen);
#ifdef DEBUG
  printf("storeUpdateChunk(): Update Chunk Stored: %6d [%6d]\n",
	 chunk->pos, chunk->blen);
#endif
  return 1;
}

long
CCLPFox::process_update_data()
{
  return 1;
}

long 
CCLPFox::doUpdate()
{
  FXString   pname;// = cui.dest_path;
  FXString   fname;// =  pname +"/"+ cui.dest_fname;
  FILE      *fp;
  int        retval = 0;
  FXuint     fmode;
  char       md[50];

  fname = FXString(cui.dest_path) + FXString(cui.dest_fname);
#ifdef DEBUG
  printf("doUpdate(): Filename: %s\n", fname.text());
#endif
  //If the file already exists, change the name
  if (FXStat::exists(fname)){
    FXFile::rename(fname, fname+".old");
  }
  //now authenticate / decrypt
  if (!process_update_data()) return 0;
  //then write file
  fp = fopen(fname.text(), "w");
  if (!fp){
#ifdef DEBUG
    printf("doUpdate(): Unable to open file: %s\n", fname.text());
#endif
    return 0;
  }
  retval = fwrite(updata, cui.fsize, 1, fp);
  fclose(fp);
  fmode = (FXuint) cui.fmode;
#ifdef DEBUG
  printf("doUpdate(): Update Completed: %s [mode=%O]\n", fname.text(), 
	 fmode);
#endif
  FXStat::mode(fname, fmode);
  return retval;
}
