#include <fox-1.6/fx.h>
using namespace FX;

#include "cclcfox.h"
#include "gui.h"
#include "locker.h"
#ifdef WIN32
#  include <windows.h>
#  include <winuser.h>
#else
#  include <pthread.h>
#  include <sys/signal.h>
#endif

//#define DEBUG 1
//#define DEBUG_PASS 1
//#define DEBUG_UPD 1
extern CCLCFox    *cclcfox;
extern ClientWin  *clientwin;
extern Locker     *locker;
extern Grabber    *grabber;

struct CHUNK *chunk;
void print_hash(unsigned char *hash, int len);

int thread_msg_in = 0;
#ifdef WIN32
DWORD      th=0, chatthr;
DWORD WINAPI dispMessage(LPVOID message);
#else
pthread_t    th=0, chatthr;
void *dispMessage(void * message);
#endif

int updateState = OFFUPDATESTATE;
unsigned long MkahawaVersion = (((MK_MAJOR_VER << 8)|MK_MINOR_VER) << 8) | MK_RELEASE;

void
onEventCallback(FXuint cmd,void * data,FXuint size,void * userdata)
{
  cclcfox->execCommand(cmd,data,size);
}

long
setAdminPass(char *pass, int plen)
{

  FXString path = FXSystem::getHomeDirectory() + "/.mkahawa";
  FXString fname = "mkahawa.inf";

#ifdef DEBUG_PASS
  printf("setAdminPass(): pass = %s\n", pass);
#endif
  if (!FXStat::exists(path))
    if (!FXDir::create(path, FXIO::OwnerFull))
      return 0;
  path += "/" + fname;
  if (!FXStat::exists(path))
    if (!FXFile::create(path,FXIO::OwnerFull))
      return 0;
  FILE *fp = fopen(path.text(), "w");
  if (fp){
    FXuchar digest[CCLC_MD5_DIGEST_LENGTH];

    memset(digest, 0, CCLC_MD5_DIGEST_LENGTH);
    CCLC_MD5((FXuchar*)(pass), plen, digest);
    fwrite(digest, CCLC_MD5_DIGEST_LENGTH , 1, fp);
#ifdef DEBUG_PASS
    printf("setAdminPass()\n");
    printf("Testpass Hash: ");
    print_hash((unsigned char *)digest, CCLC_MD5_DIGEST_LENGTH);
    printf("\n");
#endif
    fclose(fp);
  }
  
  return 1;
}

void 
print_hash(unsigned char *hash, int len)
{
  int i;
  for (i=0; i<len; i++)
    printf("%02X ", hash[i]);

  return;
}

bool
checkPass(char *pass, int plen)
{
  char pstr[64];
  FILE *fp;
  FXString pname;

  pname = FXSystem::getHomeDirectory() + "/.mkahawa/mkahawa.inf";
  fp = fopen(pname.text(), "r");
  if (fp){
    FXuchar digest[CCLC_MD5_DIGEST_LENGTH];

    CCLC_MD5((FXuchar*)(pass), plen, digest);
    fread(pstr, CCLC_MD5_DIGEST_LENGTH, 1, fp);
    if (plen > CCLC_MD5_DIGEST_LENGTH) plen = CCLC_MD5_DIGEST_LENGTH;
    if (!memcmp(pstr, digest, plen))
      return TRUE;
    fclose(fp);
#ifdef DEBUG_PASS
    printf("Realpass Hash: ");
    print_hash((unsigned char *)digest, CCLC_MD5_DIGEST_LENGTH);
    printf("\n");
    printf("Testpass Hash: ");
    print_hash((unsigned char *)pstr, CCLC_MD5_DIGEST_LENGTH);
    printf("\n");
#endif
  }
  
  return FALSE;
}


void
onDisconnectCallback(void * userdata)
{
  cclcfox->shutdownNetworking();
}

FXDEFMAP(CCLCFox) CCLCFoxMap[] =
{
  FXMAPFUNC(SEL_TIMEOUT,CCLCFox::ID_TIMER,CCLCFox::onTimer),
  FXMAPFUNC(SEL_TIMEOUT,CCLCFox::ID_CHECKEVENTS,CCLCFox::onCheckEvents),
  FXMAPFUNC(SEL_TIMEOUT,CCLCFox::ID_POLLPRINT,CCLCFox::onPollPrinting)
};

FXIMPLEMENT(CCLCFox,FXObject,CCLCFoxMap,ARRAYNUMBER(CCLCFoxMap))

CCLCFox *CCLCFox::cclc = NULL;

CCLCFox::CCLCFox()
{
  if (CCLCFox::cclc)
    fxerror("An instance of Mkahawa Client is already loaded\n");
  active      = FALSE;
  stime       = 0;
  networking  = FALSE;
  client_timeout  = 0;
  poll_interval = 5000;
  clientwin->getApp()->addTimeout(this,CCLCFox::ID_TIMER,1000);
  CCLCFox::cclc = this;
  clientwin->disableHelpBtn();
  updata     = NULL;
  login_mode = 0;
  op_mode    = 0;
}

CCLCFox::~CCLCFox()
{
  shutdownNetworking();
  CCLC_shutdown();
  CCLCFox::cclc = NULL;
}

void
CCLCFox::initCCLC()
{
  CCLC_init();
  CCLC_set_on_event_callback(onEventCallback,NULL);
  CCLC_set_on_disconnect_callback(onDisconnectCallback,NULL);
}

FXbool
CCLCFox::initCCLC(const char * cafile,const char * certfile,
		  const char * certpass)
{
  int error;
  
  initCCLC();

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
CCLCFox::initNetworking(const char * server,FXushort port, const char * myname)
{
  static const char *sname = server;
  static FXushort sport = port;
  static const char *cname = myname;
  int error;

  if (CCLC_networking_init(sname,sport,cname,&error)) {
    clientwin->getApp()->addTimeout(this,CCLCFox::ID_CHECKEVENTS,100);
    clientwin->getApp()->addTimeout(this,CCLCFox::ID_POLLPRINT,1500);
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
CCLCFox::shutdownNetworking()
{
  CCLC_networking_shutdown();
  clientwin->setOwed("--.--");
  clientwin->setProducts("--.--");
  networking = FALSE;
  
  return TRUE;
}

FXbool
CCLCFox::isLocked()
{
  return locker->shown();
}

FXbool
CCLCFox::isInfoShown()
{
  return grabber->shown();
}

void
CCLCFox::showInfo()
{
  grabber->show();
  clientwin->show();
}

void
CCLCFox::hideInfo()
{
  clientwin->hide();
  grabber->hide();
}

void
CCLCFox::start()
{
  stime = time(NULL);
  clientwin->setTime("00:00");
  clientwin->setOwed("0.00");
  clientwin->setProducts("0.00");
  clientwin->enableHelpBtn();
  client_timeout = 0;
  time_warned = 0;
  active = TRUE;
  initPrintPolling();
  clientwin->getApp()->addTimeout(this,CCLCFox::ID_POLLPRINT,5000);
  //locker->allowMemberLogin(TRUE);
  setLoginMode(0);
}

void
CCLCFox::stop()
{
  active = FALSE;
  clientwin->disableHelpBtn();
/* Close the print polling */
  if (can_print_poll){
    can_print_poll = 0;
    if (cups_fd>0) close(cups_fd);
    cups_fd = -1;  
  }
}

void
CCLCFox::userExit()
{
  if (active){
    CCLC_send_cmd(CC_USEREXIT,NULL,0);
    lockScreen();
#ifdef DEBUG
    printf("User Exited\n");
#endif
  }
  else{
    lockScreen();
#ifdef DEBUG
    printf ("Only locking the screen\n");
#endif
  }
}

void
CCLCFox::resume()
{
  active = TRUE;
}

void
CCLCFox::lockScreen()
{
  setMemberLoginState(0); //reset states
  setLoginMode(0);        
  locker->lock();
  hideInfo();
}

void
CCLCFox::unlockScreen()
{
  locker->unlock();
  showInfo();
}

void
CCLCFox::shutdownSystem()
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
CCLCFox::rebootSystem()
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
CCLCFox::turnOffMonitor()
{
#ifndef WIN32
  system("xset dpms force off");
#else
  HWND hWnd = (HWND) (clientwin->id());

  SendMessage(hWnd,WM_SYSCOMMAND,SC_MONITORPOWER,2);
#endif
}

void
CCLCFox::showMessage(void * message)
{
  FXMessageBox::information(clientwin->getRoot(),
  			MBOX_OK,_("Message From Manager"), (char *)message);
}

#ifdef WIN32
DWORD WINAPI
dispMessage(LPVOID message)
#else
void *
dispMessage(void * message)
#endif
{
  FXString msgstr((char *)message);

  thread_msg_in = 1;
  clientwin->dispMessage(msgstr, 30);
  thread_msg_in = 0;

#ifdef WIN32
  return 0;
#else	
  return (void *)0;			
#endif
}

void
CCLCFox::setClientPollInterval(FXuint itvl)
{
  poll_interval = (itvl > 1000?  itvl: 1000);
}


void
CCLCFox::setOwed(FXuint owed)
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
CCLCFox::setProducts(FXuint owed)
{
  char buf[64];

  snprintf(buf,64,"%.2f",owed / 100.0);
  clientwin->setProducts(buf);
}

void
CCLCFox::userStart()
{
  CCLC_send_cmd(CC_USERLOGIN,NULL,0);
#ifdef DEBUG
  printf ("UserStart a session\n");
#endif
}

void
CCLCFox::unlockWithPass(int id,FXString password)
{
  char val[sizeof(id) + CCLC_MD5_DIGEST_LENGTH * sizeof(FXuchar)];
  FXuchar digest[CCLC_MD5_DIGEST_LENGTH];
  CCLC_MD5((FXuchar*)(password.text()),password.length(),digest);
  ((FXuint*)val)[0] = CCLC_htonl(id);
  memcpy(((FXuint*)val)+1,digest,CCLC_MD5_DIGEST_LENGTH); 
  CCLC_send_cmd(CC_MEMBERLOGIN,val,sizeof(val) * sizeof(char));
}

#define MAX_INP_SIZE 32
void
CCLCFox::unlockWithPass(FXString tktstr)
{
  /*  char val[sizeof(id) + CCLC_MD5_DIGEST_LENGTH * sizeof(FXuchar)];
  FXuchar digest[CCLC_MD5_DIGEST_LENGTH];
  CCLC_MD5((FXuchar*)(password.text()),password.length(),digest);
  ((FXuint*)val)[0] = CCLC_htonl(id);*/
  //  memcpy(((FXuint*)val)+1,digest,CCLC_MD5_DIGEST_LENGTH); 

  if (tktstr.length() > MAX_INP_SIZE)
    tktstr.trunc(MAX_INP_SIZE);
  CCLC_send_cmd(CC_TICKETLOGIN, tktstr.text(),tktstr.length());
}

void
CCLCFox::unlockWithPass(FXString login,FXString password)
{
  if (login.length() > MAX_INP_SIZE)
    login.trunc(MAX_INP_SIZE);
  const char *login_name = login.text();
  char val[strlen(login_name) * sizeof(char) + 1
	   + CCLC_MD5_DIGEST_LENGTH * sizeof(FXuchar)];
  FXuchar digest[CCLC_MD5_DIGEST_LENGTH];

  CCLC_MD5((FXuchar*)(password.text()),password.length(),digest);
  memcpy(val,login_name,strlen(login_name) + 1);
  memcpy(val + strlen(login_name) + 1,digest,CCLC_MD5_DIGEST_LENGTH);   
  CCLC_send_cmd(CC_MEMBERLOGINWITHNAME,val,sizeof(val) * sizeof(char));
  if (!strncmp(login_name, _("administrator"), 13)){
    if (checkPass((char *)password.text(), password.length())) 
      unlockScreen();      //unlock all the same
  }
}

void
CCLCFox::reportPrinting(char *cupsLogLine, int lnLen)
{
  CCLC_send_cmd(CC_USERPRINTED, cupsLogLine, lnLen);
#ifdef DEBUG
  printf("Printing Reported\n");
#endif
}

void
CCLCFox::exitProgram()
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
CCLCFox::setLoginMode(unsigned int loginmode) 
{ 
  login_mode = loginmode;
  if (login_mode == OPMODE_TICKET  || login_mode == OPMODE_MEMBER)
    clientwin->setOwedLbl(_("Balance:"));
  else 
    clientwin->setOwedLbl(_("Owed:"));
}

void
CCLCFox::execCommand(FXuint cmd,const void *data,FXuint datasize)
{
  FXuint hdata = 0;
  long resp, chsize;

  switch (cmd) {
    case CS_STOP:
      stop();
      break;
    case CS_START:
      start();
      break;
    case CS_RESUME:
      resume();
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
      client_timeout = CCLC_ntohl(*((int *) data));
      time_warned = 0;
      break;
    case CS_DISPLAYMESSAGE:
      if (!thread_msg_in){
	dispMessage((char *)data);
      }
      break;
    case CS_QUITCLIENT:
      exitProgram();
      break;
    case CS_UNLOCKSCREEN:
      unlockScreen();
      break;
    case CS_LOCKSCREEN:
      lockScreen();
      break;
    case CS_ENABLEPASSWORDEDIT:
      clientwin->setPasswordEnabled(CCLC_ntohl((*((int *) data))));
      break;
    case CS_ALLOWMEMBERLOGIN:
      //op_mode = OPMODE_MEMBER;
      setMemberLoginState(0);
      setLoginMode(0);
      locker->allowMemberLogin(bool(CCLC_ntohl((*((int *) data)))));
      break;
    case CS_ALLOWTICKETLOGIN:
      //op_mode = OPMODE_TICKET;
      setMemberLoginState(0);
      setLoginMode(0);
      locker->allowTicketLogin(bool(CCLC_ntohl((*((int *) data)))));
      break;
    case CS_ALLOWUSERLOGIN:
      //op_mode = OPMODE_POSTPAID;
      setLoginMode(0);
      locker->allowUserLogin(bool(CCLC_ntohl((*((int *) data)))));
      break;
    case CS_ENABLEASSIST:
      clientwin->enableAssist(bool(CCLC_ntohl((*((int *) data)))));
      break;
    case CS_CHATSERVER:
#ifdef WIN32
      CreateThread(NULL, 0,
                   dispMessage,
                   (LPVOID)data,
                   0, &chatthr);
#else   
      pthread_create(&chatthr, NULL, &dispMessage, (void *)data);
#endif      
      break;
    case CS_CALLASSIST:
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
#ifdef DEBUG_UPD
      printf("Update Acknowledged: resp=%d\n", resp);
#endif
      break;
    case CS_UPDATEDATA:
      updateState = ONUPDATESTATE;
      chunk = (struct CHUNK*)data;
      resp = chunk->pos;
      chsize = chunk->blen;
      if (storeUpdateChunk((char *)data, (long)datasize)){
	resp += chsize;
	resp = CCLC_htonl(resp);
      }
      else{
	resp = CCLC_htonl(resp);
      }
      //CCLC_send_cmd(CC_UPDATEDATA, &resp, sizeof(resp));
#ifdef DEBUG_UPD
      printf("Update Data Acknowledged\n");
#endif
      break;
    case CS_UPDATEEND:
      long resp;
      char msgstr[120], *cp;
      
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
	snprintf(cp, 100, "%s", _("Was unable to write update file"));
	CCLC_send_cmd(CC_UPDATEEND, msgstr, sizeof(resp)+strlen(cp));
      }
      updateState = OFFUPDATESTATE;
#ifdef DEBUG_UPD
      printf("Update End Acknowledged\n");
#endif
      break;
    case CS_SETADMINPASS:
      setAdminPass((char*)data, datasize);
#ifdef DEBUG_PASSWORD
      printf("Set Admin Screen\n");
#endif
    case CS_SETPOLLINTERVAL:
      hdata = CCLC_ntohl(*(FXuint *) data);
      setClientPollInterval(hdata);
#ifdef DEBUG
      printf("Set Admin Screen\n");
#endif
      break;
      /*
    case CS_SETTIMEWARNING:
      hdata = CCLC_ntohl(*(FXuint *) data);
#ifdef DEBUG
      printf("Set client time warnings\n");
#endif
      break;
      */
  }
}
#define WARNING_TIME 120

long
CCLCFox::onTimer(FXObject*,FXSelector,void*)
{
  if (!networking)
    initNetworking();
  if (active) {
    char buf[8];
    int usedtime = time(NULL) - stime;
    int hours = usedtime / 3600;
    int mins = (usedtime % 3600) / 60;

    if ( hours > 0)
      snprintf(buf,8,"%.2dh%.2dm",hours,mins);
    else{
      int secs = usedtime % 60;
      snprintf(buf,8,"%.2dm%.2ds",mins,secs);
    }
    clientwin->setTime(buf);
    if (updateState != ONUPDATESTATE)  //quiet during update
      CCLC_send_cmd(CC_GETOWED,NULL,0);
    
    if (0 < client_timeout){
      //#define DEBUG_TIMEOUT
#ifdef DEBUG_TIMEOUT
      printf ("onTimer(): time_warned=%d usedtime=%d client_timeout=%d\n", 
	      time_warned, usedtime, client_timeout);
#endif      
      if (usedtime > client_timeout) // time exceeded
	userExit();
      else if ( (client_timeout - usedtime) < WARNING_TIME &&  !time_warned ){
	//WARNING_TIME to go
	int wtime  = WARNING_TIME;
	char wstr[64];
	
	sprintf(wstr, _("You have less than %d mins to browse."), wtime/60);
	time_warned = 1;
	dispMessage(wstr);
	//poll every last second
	poll_interval = 1000;
      }
    }
  }
  clientwin->getApp()->addTimeout(this,CCLCFox::ID_TIMER,poll_interval);

  return 1;
}

long
CCLCFox::onPollPrinting(FXObject*,FXSelector,void*)
{
  //int poll_len = 1500; 
  int poll_len = 3000; 

  if (active) {
    pollPrinting();
    clientwin->getApp()->addTimeout(this,CCLCFox::ID_POLLPRINT,poll_len);
#ifdef DEBUG
    printf("Polled Printing: [ Active ] Length=%d\n", poll_len);
#endif
  }
#ifdef DEBUG
  if (!active){
    printf("Polled Printing: [Inactive] Length=%d\n", poll_len);
  }
#endif
  
  return 1;
}


long
CCLCFox::onCheckEvents(FXObject*,FXSelector,void*)
{
  CCLC_check_events();
  clientwin->getApp()->addTimeout(this,CCLCFox::ID_CHECKEVENTS,100);

  return 1;
}

void
CCLCFox::setPassword(FXuchar digest[2*CCLC_MD5_DIGEST_LENGTH])
{
  CCLC_send_cmd(CC_SETMEMBERPASSWORD,digest,2*CCLC_MD5_DIGEST_LENGTH);
}

#define BUFSIZE 512
void 
CCLCFox::pollPrinting()
{
  struct stat tstat;
  int         n, lnlen;
  char        buf[BUFSIZE], *cp, *np;

  if (!can_print_poll) {
#ifdef DEBUG
    printf ("Unable to print poll. Retry initializing\n");
#endif
    if (active) initPrintPolling();
    return;
  }
  memset(&tstat, 0, sizeof(struct stat));
  fstat(cups_fd, &tstat);
  if (tstat.st_size > fst.st_size){
    n = tstat.st_size - fst.st_size;
    lseek(cups_fd, fst.st_size, SEEK_SET);
    n = read(cups_fd, buf, (n>BUFSIZE? BUFSIZE: n));
    cp = buf;
#ifdef DEBUG
    printf("Line: %s\n",buf);
#endif
    //Report printing, line by line
    while (*cp){
      np = NULL;
      //np = index(cp, '\n');
      np = strchr(cp, '\n');
	  lnlen = (np!=NULL)?(np-cp): (buf+n-cp); //line length
#ifdef DEBUG
      printf("Line length = %d\n", lnlen);
#endif
      if (lnlen < 10) break;  //misread
      reportPrinting(cp, lnlen); //send to server
      usleep(10000);
      cp=cp+lnlen;
    }
    //update fst
    fst = tstat;
  }
}

void 
CCLCFox::initPrintPolling()
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
    n = fstat(cups_fd, &fst);  
  }
  else{
#ifdef DEBUG
    printf("InitPrintPolling: Unable to Start Polling\n");
#endif
  }
}

/* Ask for assistance */
void 
CCLCFox::askForHelp()
{
  ack_assist = 0;
  CCLC_send_cmd(CC_CALLASSIST,NULL,0);
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
}

long
CCLCFox::authUpdate(void *hdr, unsigned long len)
{
  UpdateInfo *ui;

  if (len < sizeof(UpdateInfo))
    return 0; //size checking
  memset(&cui, 0, sizeof(UpdateInfo));
  ui = (UpdateInfo *)hdr;
  cui = *ui;
  //authenticate & verify data
#ifdef DEBUG_UPD
  print_update_info(ui);
#endif
  if (cui.fsize > 4000000) {
#ifdef DEBUG_UPD
    printf("authUpdate(): file is too large\n");
#endif
    return 0;
  }
  if (updata) {
    free(updata);
    updata = NULL;
  }
  updata = (char *)malloc(cui.fsize);
  if (!updata){
#ifdef DEBUG_UPD
    printf("authUpdate(): Unable to allocate memory\n");
#endif
    return 0;
  }
  return 1;
}

long 
CCLCFox::storeUpdateChunk(char *dat, long datlen)
{
  memcpy((updata+chunk->pos), chunk->buf, chunk->blen);
#ifdef DEBUG_UPD
  printf("storeUpdateChunk(): Update Chunk Stored: %6d [%6d]\n",
	 chunk->pos, chunk->blen);
#endif
  return 1;
}

long
CCLCFox::process_update_data()
{
  return 1;
}

long 
CCLCFox::doUpdate()
{
  FXString   pname;
  FXString   fname;
  FILE      *fp;
  int        retval = 0;
  FXuint     fmode;

  fname = FXString(cui.dest_path) + FXString(cui.dest_fname);
#ifdef DEBUG_UPD
  printf("doUpdate(): Filename: %s\n", fname.text());
#endif
  if (FXStat::exists(fname)){
    int i;
    for (i=0; (FXStat::exists(fname+"."+FXStringVal(i))); i++);
    FXFile::rename(fname, fname+"."+FXStringVal(i));
  }
  //now authenticate / decrypt
  if (!process_update_data()) return 0;
  //then write file
  fp = fopen(fname.text(), "w");
  if (!fp){
#ifdef DEBUG_UPD
    printf("doUpdate(): Unable to open file: %s\n", fname.text());
#endif
    return 0;
  }
  retval = fwrite(updata, cui.fsize, 1, fp);
  fclose(fp);
  fmode = (FXuint) cui.fmode;
#ifdef DEBUG_UPD
  printf("doUpdate(): Update Completed: %s [mode=%O]\n", fname.text(), 
	 fmode);
#endif
  FXStat::mode(fname, fmode);
  return retval;
}
