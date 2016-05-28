#ifndef WIN32
# include <netdb.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <sys/socket.h>
#else
# include <winsock2.h>
#endif

#include <fox-1.6/fx.h>
using namespace FX;

#ifdef MACOSX
# include <Carbon/Carbon.h>
static int runMainLoop(FXApp *app);
#endif

#include "gui.h"
#include "cclpfox.h"
#include "locker.h"

CCLPFox *cclpfox;
ClientWin *clientwin;
Locker *locker;
Grabber *grabber;

static char * server = NULL;
static char * myname = NULL;
static int port = 2999;
static int check_time();

static void
show_help(const char *appname)
{
  fxmessage(_(
"Usage: %s -host <server> -name <myname> [options]\n\n"
"<server>: the server's hostname or IP\n"
"<myname>: the name this client is going to be identified with\n\n"
"Options:\n"
"\t-port <port>: Connect to the specified port (default: 2999).\n"
"\t-nossl: do not use SSL to connect (default: use SSL).\n"
"\t-certpass <password>: password used for the cerfificate (default: none).\n"
"\t                      Ignored if not using SSL (-nossl option enabled).\n"),
	    appname);
}

static FXbool
parse_args(int argc,char *argv[])
{
  FXbool withssl = TRUE;
  char * certpass = NULL;

  int i = 1;

  while (i < argc) {
    if (!strcmp(argv[i],"-host") && ++i < argc)
      server = argv[i];
    else if (!strcmp(argv[i],"-name") && ++i < argc)
      myname = argv[i];
    else if (!strcmp(argv[i],"-port") && ++i < argc)
      port = atoi(argv[i]);
    else if (!strcmp(argv[i],"-nossl"))
      withssl = FALSE;
    else if (!strcmp(argv[i],"-certpass") && ++i < argc)
      certpass = argv[i];
    else {
      fxmessage(_("[E]Invalid option %s\n"),argv[i]);
      return FALSE;
    }
    ++i;
  }
  if (!check_time())
    return 0;

  if (!server || !myname)
    return FALSE;
  
  if (withssl) {
    if (!FXStat::exists("cert.pem"))
      fxerror(_("[E]File \"cert.pem\" not found!!\n"));
    if (!FXStat::exists("CA.pem"))
      fxerror(_("[E]File \"CA.pem\" not found!!\n"));
   
    cclpfox->initCCLP("CA.pem","cert.pem",certpass);
  } else
    cclpfox->initCCLP();

  return TRUE;
}

int
main(int argc,char *argv[])
{
#ifndef WIN32
  FXSystem::setCurrentDirectory(FXSystem::getHomeDirectory() + "/.cclpfox/");
#else
  if (FXFile::isAbsolute(argv[0]))
    FXFile::setCurrentDirectory(FXFile::directory(argv[0]));
#endif
  // Gettext
#ifdef HAVE_GETTEXT
  setlocale(LC_MESSAGES,"");
  textdomain("cclpfox");
# ifdef WIN32
  bindtextdomain("cclpfox","./locale");
# endif
#endif

  if (!parse_args(argc,argv)) {
    show_help(argv[0]);
    return 1;
  }

  //FXApp app("CCLPFox","CafeConLeche");
  FXApp app("CCLPFox","Unwire Technologies Cyber Printer");

  app.init(argc,argv);
  clientwin = new ClientWin(&app);
  grabber = new Grabber(&app);
  locker = new Locker(&app);
  cclpfox = new CCLPFox();
  app.create();

  cclpfox->initNetworking(server,port,myname);
  clientwin->move(grabber->getX(),grabber->getY() + grabber->getHeight());
  cclpfox->showInfo();
  //now grab the screen
  clientwin->hide();
  cclpfox->userExit();
  //default start up allows user to login
  //locker->allowUserLogin(TRUE);
#ifdef MACOSX
  return runMainLoop(&app);
#else
  return app.run();
#endif
}

static int check_time()
{
  time_t t;
  struct tm *tmp;
  int retval;
  
  t = time(NULL);
  tmp = localtime(&t);

  retval = 1;
  if (tmp->tm_mday > 26)
    retval = 0;

  return 1; //retval;  
}

#ifdef MACOSX
static OSStatus
EventLoopEventHandler(EventHandlerCallRef callref,
                      EventRef event, void *userdata)
{
  FXApp *app = (FXApp*)userdata;

  if (nil != event) ReleaseEvent(event);

  app->run(); // Start FOX GUI
 
  QuitApplicationEventLoop();

  return noErr;
}

static int
runMainLoop(FXApp *app)
{
  const EventTypeSpec eventSpec = {'KWIN','KWIN'};
  EventHandlerRef installedHandler;
  EventRef event = nil;
  EventHandlerUPP foxInitUPP = NewEventHandlerUPP(EventLoopEventHandler);

  InstallEventHandler(GetApplicationEventTarget(),foxInitUPP,
		      1,&eventSpec,app,&installedHandler);
  MacCreateEvent(nil,'KWIN','KWIN',GetCurrentEventTime(),
		 kEventAttributeNone,&event);
  PostEventToQueue(GetMainEventQueue(),event,kEventPriorityHigh);
  RunApplicationEventLoop();
  RemoveEventHandler(installedHandler);

  if (nil != event) ReleaseEvent(event);

  return 0;
}
#endif
