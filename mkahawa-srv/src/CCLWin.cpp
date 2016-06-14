#include <ccls.h>
#include <fox-1.6/fx.h>
#include <unistd.h>
#ifdef WIN32
#  include <windows.h>
#  include <winuser.h>
#else
#  include <pthread.h>
#endif

using namespace FX;
using namespace std;

#include "cclfox.h"
#include "ProductsFrame.h"
#include "CashingFrame.h"
#include "TarifFrame.h"
#include "MembersFrame.h"
#include "EmployeesFrame.h"
#include "NotpaidFrame.h"
#include "LogFrame.h"
#include "ReportFrame.h"

#include "SettingsBox.h"
#include "QTicketsBox.h"


#include "CCLIconItem.h"
#include "MSGWin.h"
#include "CCLWin.h"
#include "cmds.h"
#include "icons.h"
#include "verifiers.h"
#include <ctype.h>

long sendUpdateChunks(int client);
void *dispMessage(void * message);
void applySettings(unsigned long settings);


FXGIFIcon *dbIcon01;
FXGIFIcon *dbIcon0;
FXGIFIcon *dbIcon1;
FXGIFIcon *dbIcon2;
FXGIFIcon *dbIcon3;

//#define DEBUG
//#define DEBUG_GUI
//#define DEBUG_UPD
//#define DEBUG_TICKET
//#define DEBUG_CREDIT
//#define DEBUG_PRINT
//#define DEBUG_WARN
//#define  DEBUG_CALLBACK
//#define DEBUG_SETTINGS

#define WARNTENMINUTE    10
#define WARNFIVEMINUTE   5
#define WARNONEMINUTE    1
#define MINCREDIT        100

extern FXSettings         *passwords;
extern CCLWin             *mainwin;
extern ProductsFrame      *productsframe;
extern CashingFrame       *cashingframe;
extern TarifFrame         *tarifframe;
extern NotpaidFrame       *notpaidframe;
extern LogFrame           *logframe;
extern MembersFrame       *membersframe;
extern EmployeesFrame     *employeesframe;
extern ReportFrame        *reportframe;

extern QTicketsBox        *ticketsframe;
extern SettingsBox        *settingsframe;

CyberSettings             *cyber_settings = NULL;



#ifdef WIN32
DWORD    updth, msgth;
#else
pthread_t    updth, msgth;
#endif

#define MAX_CLIENTS  256


int          print_job[MAX_CLIENTS];
int          job_pages[MAX_CLIENTS];
int          warn_lvl[MAX_CLIENTS];
FXuint       prev_price[MAX_CLIENTS];
char         *cybername = (char *)"Nordseye Cyber Manager";

FXDEFMAP(CCLWin) CCLWinMap[] = {
	FXMAPFUNC(SEL_TIMEOUT,CCLWin::ID_CHECKEVENTS,CCLWin::onCheckEvents),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_START,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_STOP,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_UNSTOP,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_SETMEMBER,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_MONITOROFF,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_REBOOT,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_POWEROFF,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_ALLMONITOROFF,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_ALLREBOOT,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_ALLPOWEROFF,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_UPDATECLIENT,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_ALLUPDATECLIENT,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_ALLADMINPASS,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_SWAP,CCLWin::onSwap),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_TIME,CCLWin::onTime),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_PAUSE,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_ALLOWUSERLOGIN,CCLWin::onAllowUserLogin),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_ALLOWMEMBERLOGIN,CCLWin::onAllowMemberLogin),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_ALLOWTICKETLOGIN,CCLWin::onAllowTicketLogin),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_ENABLEASSIST,CCLWin::onEnableAssist),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_ALLALLOWUSERLOGIN,CCLWin::onAllAllowUserLogin),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_ALLALLOWMEMBERLOGIN,CCLWin::onAllAllowMemberLogin),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_ALLALLOWTICKETLOGIN,CCLWin::onAllAllowTicketLogin),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_ALLENABLEASSIST,CCLWin::onEnableAllAssist),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_QUITCLIENT,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_ABOUT,CCLWin::onAbout),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_NEWCLIENT,CCLWin::onNewClient),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_DELCLIENT,CCLWin::onDelClient),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_EMPLOGIN,CCLWin::onEmpLogin),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_MSGCLIENT,CCLWin::onMsgClient),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_CYBERSET,CCLWin::onCyberSet),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_SHUTTER,CCLWin::onListShutter),
	FXMAPFUNC(SEL_SELECTED,CCLWin::ID_CLIENTSLIST,CCLWin::onClientSelected),
	FXMAPFUNC(SEL_SELECTED,CCLWin::ID_CLIENTSLIST2,CCLWin::onClientSelected2),
	FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,CCLWin::ID_CLIENTSLIST,CCLWin::onShowClientMenu),
	FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,CCLWin::ID_CLIENTSLIST2,CCLWin::onShowClientMenu2),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_10MIN,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_20MIN,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_30MIN,CCLWin::onCommand),
	FXMAPFUNC(SEL_COMMAND,CCLWin::ID_60MIN,CCLWin::onCommand),
	FXMAPFUNC(SEL_TIMEOUT,CCLWin::ID_TIMERTICK,CCLWin::onTimerTick)
};

FXIMPLEMENT(CCLWin,FXMainWindow,CCLWinMap,ARRAYNUMBER(CCLWinMap))

FXShutterItem *clientshutter;

int
citemSortFunc(const FXIconItem * l,const FXIconItem * r)
{
	const FXString lname = CCL_client_name_get((long) l->getData());
	const FXString rname = CCL_client_name_get((long) r->getData());

	return compare(lname,rname);
}

int
clitemSortFunc(const FXFoldingItem * l,const FXFoldingItem * r)
{
	const FXString lname = CCL_client_name_get((long) l->getData());
	const FXString rname = CCL_client_name_get((long) r->getData());

	return compare(lname,rname);
}

void *
dispMessage(void * message)
{
	mainwin->dispMessage((char *)message);
	free (message);
	return 0;
}

int
CCLWin::dispMessage(char * message)
{
	FXMessageBox::information(getRoot(),
	                          MBOX_OK,_("Information Update"), (char *)message);
	free (message);
	return 0;
}

/****** CCLWin ******/
CCLWin::CCLWin(FXApp * app)
	:FXMainWindow(app, cybername,NULL,NULL,DECOR_ALL,0,50,800,550)
{
	toSwap = -1;
	// App Icons
	bigicon = new FXGIFIcon(getApp(),icon32_gif);
	miniicon = new FXGIFIcon(getApp(),icon16_gif);
	setIcon(bigicon);
	setMiniIcon(miniicon);
	//Font
	FXFont  *font =new FXFont(app,"fixed,105,,,,iso10646-1");
	new FXToolTip(app,0);
	//
	FXHorizontalFrame *infoframe =
	    new FXHorizontalFrame(this,LAYOUT_FILL_X|LAYOUT_SIDE_BOTTOM|FRAME_SUNKEN,
	                          0,0,0,0,0,0,0,0,10,0);
	vsplitter = new FXSplitter(this,FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_FILL_Y|
	                           SPLITTER_REVERSED,0,0,0,0);
	//FXSplitter *vsplitter1;
	FXShutter *lshutter =
	    new FXShutter(vsplitter,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y,
	                  0,0,0,0,0,0,0,0,0,0);
	FXShutterItem *litem1 = new FXShutterItem(lshutter,_("Clients"),NULL,
	        LAYOUT_FILL_X|LAYOUT_FILL_Y,
	        0,0,0,0,0,0,0,0);
	clientshutter = litem1;
	FXShutterItem *litem2 = new FXShutterItem(lshutter,_("Log"),NULL,
	        LAYOUT_FILL_X|LAYOUT_FILL_Y,
	        0,0,0,0,0,0,0,0);
	FXShutterItem *litem3 = new FXShutterItem(lshutter,_("Report"),NULL,
	        LAYOUT_FILL_X|LAYOUT_FILL_Y,
	        0,0,0,0,0,0,0,0);
	/********* Clients area**********/
	FXVerticalFrame *vframe =
	    new FXVerticalFrame(litem1->getContent(),FRAME_SUNKEN|LAYOUT_FILL_X|
	                        LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,0);

	// Toolbar
	ctoolbar = new FXToolBar(vframe,FRAME_RAISED|LAYOUT_TOP|LAYOUT_FILL_X,
	                         0,0,0,0,0,0,0,0,0,0);
	playicon =   new FXGIFIcon(getApp(),play_gif);
	pauseicon =  new FXGIFIcon(getApp(),pause_gif);
	stopicon =   new FXGIFIcon(getApp(),stop_gif);
	unstopicon = new FXGIFIcon(getApp(),continue_gif);
	swapicon =   new FXGIFIcon(getApp(),swap_gif);
	timeicon =   new FXGIFIcon(getApp(),time_gif);
	msgicon =    new FXGIFIcon(getApp(),msgbtn_gif);
	emplogicon = new FXGIFIcon(getApp(),emplogbtn_gif);
	delicon =    new FXGIFIcon(getApp(),delbtn_gif);
	newicon =    new FXGIFIcon(getApp(),newbtn_gif);
	csicon =    new FXGIFIcon(getApp(),cyber_settings_gif);
	//the buttons
	playbutton =
	    new FXButton(ctoolbar,_("\tStart a Session"),playicon,this,ID_START,
	                 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,
	                 0,0,0,0,0,0,0,0);
	pausebutton =
	    new FXButton(ctoolbar,_("\tPause Session"),pauseicon,this,ID_PAUSE,
	                 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,
	                 0,0,0,0,0,0,0,0);
	stopbutton =
	    new FXButton(ctoolbar,_("\tStop Session"),stopicon,this,ID_STOP,
	                 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,
	                 0,0,0,0,0,0,0,0);
	unstopbutton =
	    new FXButton(ctoolbar,_("\tContinue Session"),unstopicon,this,ID_UNSTOP,
	                 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,
	                 0,0,0,0,0,0,0,0);
	swapbutton =
	    new FXButton(ctoolbar,_("\tSwap Sessions"),swapicon,this,ID_SWAP,
	                 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,
	                 0,0,0,0,0,0,0,0);
	timebutton =
	    new FXButton(ctoolbar,_("\tSet Timeout"),timeicon,this,ID_TIME,
	                 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,
	                 0,0,0,0,0,0,0,0);
	msgbutton =
	    new FXButton(ctoolbar,_("\tMessage Client"),msgicon,this,ID_MSGCLIENT,
	                 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,
	                 0,0,0,0,0,0,0,0);
	csbutton =
	    new FXButton(ctoolbar,_("\tCyber Settings"),csicon,this,ID_CYBERSET,
	                 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_RIGHT,
	                 0,0,0,0,0,0,0,0);
	loginbutton =
	    new FXButton(ctoolbar,_("\tStaff Login"),emplogicon,this,ID_EMPLOGIN,
	                 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_RIGHT,
	                 0,0,0,0,0,0,0,0);
	newbutton =
	    new FXButton(ctoolbar,_("\tNew Client"),newicon,this,ID_NEWCLIENT,
	                 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_RIGHT,
	                 0,0,0,0,0,0,0,0);
	delbutton =
	    new FXButton(ctoolbar,_("\tDelete Client"),delicon,this,ID_DELCLIENT,
	                 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_RIGHT,
	                 0,0,0,0,0,0,0,0);
	// Iconlist
	FXSplitter *hsplitter =
	    new FXSplitter(vframe,FRAME_RAISED|SPLITTER_VERTICAL|LAYOUT_FILL_X|
	                   LAYOUT_FILL_Y|SPLITTER_REVERSED,0,0,0,0);
	//Different list views
	FXShutter *lshutter1 =
	    new FXShutter(hsplitter,this, ID_SHUTTER,LAYOUT_FILL_X|LAYOUT_FILL_Y,
	                  0,0,0,0,0,0,0,0,0,0);
	FXShutterItem *lvitem1 = new FXShutterItem(lshutter1,_("Client Icons"),NULL,
	        LAYOUT_FILL_X|LAYOUT_FILL_Y,
	        0,0,0,0,0,0,0,0);
	FXShutterItem *lvitem2 = new FXShutterItem(lshutter1,_("Client Details"),NULL,
	        LAYOUT_FILL_X|LAYOUT_FILL_Y,
	        0,0,0,0,0,0,0,0);
	clientslist = new FXIconList(lvitem1->getContent(),this,ID_CLIENTSLIST,
	                             LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|
	                             ICONLIST_AUTOSIZE|ICONLIST_BIG_ICONS|
	                             ICONLIST_SINGLESELECT|ICONLIST_COLUMNS);
	clientslist->setSortFunc(citemSortFunc);
	bpcicons[CCL_INACTIVE] = new FXGIFIcon(getApp(),pc00_gif);
	bpcicons[CCL_ACTIVE] = new FXGIFIcon(getApp(),pc01_gif);
	bpcicons[CCL_PAUSED] = new FXGIFIcon(getApp(),pc02_gif);
	bpcicons[3] = new FXGIFIcon(getApp(),pc03_gif);
	bpcicons[4] = new FXGIFIcon(getApp(),updIcon_gif);
	disconicon = new FXGIFIcon(getApp(),pc04s_gif);
	clientslist->appendHeader(_("Name"),NULL,100);
	clientslist2 = new FXFoldingList(lvitem2->getContent(),this,ID_CLIENTSLIST2,
	                                 LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|
	                                 FOLDINGLIST_SHOWS_LINES|
	                                 FOLDINGLIST_SINGLESELECT|ICONLIST_COLUMNS);
	clientslist2->setSortFunc(clitemSortFunc);
	clientslist2->appendHeader(_("Client"),NULL,80);
	clientslist2->appendHeader(_("Status"),NULL,80);
	clientslist2->appendHeader(_("Start"),NULL,90);
	clientslist2->appendHeader(_("End"),NULL,50);
	clientslist2->appendHeader(_("Length"),NULL,50);
	clientslist2->appendHeader(_("Session Cost"),NULL,60);
	clientslist2->appendHeader(_("Other"),NULL,50);

	//vsplitter1->setSplit(2, 0);
	//  if (prgrs) delete prgrs;
	//prgrs = new FXProgressDialog(this, "Updating...", "Update in Progress",
	//		       PROGRESSDIALOG_NORMAL, 600, 400, 200, 50);

	prgrs = new FXProgressBar(hsplitter,NULL,0,PROGRESSBAR_NORMAL,0,0,300,10,
	                          DEFAULT_PAD,DEFAULT_PAD,DEFAULT_PAD,DEFAULT_PAD);
	//prgrs->setBarSize(10);
	//prgrs->setBarColor(FXRGB(50,70,190));
	//prgrs->setBarBGColor(FXRGB(200,200,230));
	//prgrs->hide();


	FXToolBar *speedbar;
	speedbar = new FXToolBar(hsplitter,FRAME_RAISED|LAYOUT_TOP|LAYOUT_FILL_X,
	                         0,0,0,0,0,0,0,0,0,0);
	hsplitter->setSplit(2,37);
	min10icon =   new FXGIFIcon(getApp(),min10btn_gif);
	min20icon =  new FXGIFIcon(getApp(),min20btn_gif);
	min30icon =   new FXGIFIcon(getApp(),min30btn_gif);
	min60icon =   new FXGIFIcon(getApp(),min60btn_gif);

	dbIcon01 = new FXGIFIcon(getApp(), dbutton01_gif);
	dbIcon0 = new FXGIFIcon(getApp(), dbutton0_gif);
	dbIcon1 = new FXGIFIcon(getApp(), dbutton1_gif);
	dbIcon2 = new FXGIFIcon(getApp(), dbutton2_gif);
	dbIcon3 = new FXGIFIcon(getApp(), dbutton3_gif);

	//FXGIFIcon *minemptyicon =   new FXGIFIcon(getApp(),minemptybtn_gif);
	min10btn =
	    new FXButton(speedbar,_("\tGrant 10 Minutes"),min10icon,this,ID_10MIN,
	                 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,
	                 0,0,0,0,0,0,0,0);
	min20btn =
	    new FXButton(speedbar,_("\tGrant 20 Minutes"),min20icon,this,ID_20MIN,
	                 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,
	                 0,0,0,0,0,0,0,0);
	min30btn =
	    new FXButton(speedbar,_("\tGrant 30 Minutes"),min30icon,this,ID_30MIN,
	                 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,
	                 0,0,0,0,0,0,0,0);
	min60btn =
	    new FXButton(speedbar,_("\tGrant 1 Hour"),min60icon,this,ID_60MIN,
	                 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,
	                 0,0,0,0,0,0,0,0);
	/*  new FXButton(speedbar,_("Test"),minemptyicon,this,ID_60MIN,
	       BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,
	       0,0,0,0,0,0,0,0); */
	notpaidframe = new NotpaidFrame(hsplitter);
	hsplitter->setSplit(1,10);
	//  hsplitter->setSplit(2,100);
	/* Log */
	logframe = new LogFrame(litem2->getContent());
	reportframe = new ReportFrame(litem3->getContent());
	/****************************************************************/
	FXLabel *labelhandle;
	FXVerticalFrame *ivframe;

	FXFont *i_fonthandle = new FXFont(getApp(),"arial",13,FXFont::Bold);
	ivframe = new FXVerticalFrame(infoframe,0,0,0,0,0,0,0,0,0,0,0);
	labelhandle = new FXLabel(ivframe,_("Time:"));
//  labelhandle->setFont(i_fonthandle);
	i_time = new FX7Segment(ivframe,"00:00:00",
	                        FRAME_SUNKEN|SEVENSEGMENT_SHADOW);
	i_time->setTextColor(FXRGB(230,230,230));
	i_time->setBackColor(FXRGB(0,0,0));
	ivframe = new FXVerticalFrame(infoframe,0,0,0,0,0,0,0,0,0,0,0);
	labelhandle = new FXLabel(ivframe,_("Total:"));
//  labelhandle->setFont(i_fonthandle);
	i_owes = new FX7Segment(ivframe,"0.00",
	                        FRAME_SUNKEN|SEVENSEGMENT_SHADOW);
	i_owes->setTextColor(FXRGB(50,255,50));
	i_owes->setBackColor(FXRGB(0,0,0));
	ivframe = new FXVerticalFrame(infoframe,0,0,0,0,0,0,0,0,0,0,0);
	labelhandle = new FXLabel(ivframe,_("Terminal:"));
//  labelhandle->setFont(i_fonthandle);
	i_terminal = new FX7Segment(ivframe,"0.00",
	                            FRAME_SUNKEN|SEVENSEGMENT_SHADOW);
	i_terminal->setTextColor(FXRGB(255,50,50));
	i_terminal->setBackColor(FXRGB(0,0,0));
	ivframe = new FXVerticalFrame(infoframe,0,0,0,0,0,0,0,0,0,0,0);
	labelhandle = new FXLabel(ivframe,_("Products:"));
	//labelhandle->setFont(i_fonthandle);
	i_products = new FX7Segment(ivframe,"0.00",
	                            FRAME_SUNKEN|SEVENSEGMENT_SHADOW);
	i_products->setTextColor(FXRGB(255,50,50));
	i_products->setBackColor(FXRGB(0,0,0));
	labelhandle = NULL;
	// Client menu
	clmenu = new FXMenuPane(clientslist);
	clmenu_caption = new FXMenuCaption(clmenu,"");
	new FXMenuSeparator(clmenu);
	new FXMenuCommand(clmenu,_("Blank Screen"),NULL,this,ID_MONITOROFF);
	new FXMenuCommand(clmenu,_("Set Member"),NULL,this,ID_SETMEMBER);
	new FXMenuCommand(clmenu,_("Reboot Machine"),NULL,this,ID_REBOOT);
	new FXMenuCommand(clmenu,_("Shutdown Machine"),NULL,this,ID_POWEROFF);
	new FXMenuCommand(clmenu,_("Update Client"),NULL,this,ID_UPDATECLIENT);
	clmenu_allowuserlogin_check =
	    new FXMenuCheck(clmenu,_("User May Start Session"),this,
	                    ID_ALLOWUSERLOGIN);
	clmenu_allowmemberlogin_check =
	    new FXMenuCheck(clmenu,_("Member May Start Session"),this,
	                    ID_ALLOWMEMBERLOGIN);
	clmenu_allowticketlogin_check =
	    new FXMenuCheck(clmenu,_("Login by Ticket"),this,
	                    ID_ALLOWTICKETLOGIN);
	clmenu_enableassist_check =
	    new FXMenuCheck(clmenu,_("Allow Assistance Request"),this,
	                    ID_ENABLEASSIST);

	// ALL Clients Menu
	clsmenu = new FXMenuPane(clientslist);
	clsmenu_caption = new FXMenuCaption(clsmenu,"Apply to All Clients");
	new FXMenuSeparator(clsmenu);
	new FXMenuCommand(clsmenu,_("Blank Screen"),NULL,this,ID_ALLMONITOROFF);
	new FXMenuCommand(clsmenu,_("Reboot Machine"),NULL,this,ID_ALLREBOOT);
	new FXMenuCommand(clsmenu,_("Shutdown Client"),NULL,this,ID_ALLPOWEROFF);
	new FXMenuCommand(clsmenu,_("Update Client"),NULL,this,ID_ALLUPDATECLIENT);
	new FXMenuCommand(clsmenu,_("Admin Password"),NULL,this,ID_ALLADMINPASS);
	clsmenu_allowuserlogin_check =
	    new FXMenuCheck(clsmenu,_("User May Start Session"),this,
	                    ID_ALLALLOWUSERLOGIN);
	clsmenu_allowmemberlogin_check =
	    new FXMenuCheck(clsmenu,_("Member May Start Session"),this,
	                    ID_ALLALLOWMEMBERLOGIN);
	clsmenu_allowticketlogin_check =
	    new FXMenuCheck(clsmenu,_("Login by Ticket"),this,
	                    ID_ALLALLOWTICKETLOGIN);
	clsmenu_enableassist_check =
	    new FXMenuCheck(clsmenu,_("Allow Assistance Request"),this,
	                    ID_ALLENABLEASSIST);

	/***********************************************************************/

	rshutter = new FXShutter(vsplitter,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y,
	                         0,0,0,0,0,0,0,0,0,0);
	FXShutterItem *ritem1 =
	    new FXShutterItem(rshutter,_("Products"),NULL,LAYOUT_FILL_X|LAYOUT_FILL_Y,
	                      0,0,0,0,0,0,0,0);
	productsframe = new ProductsFrame(ritem1->getContent());
	FXShutterItem *ritem2 =
	    new FXShutterItem(rshutter,_("Cashing"),NULL,LAYOUT_FILL_X|LAYOUT_FILL_Y,
	                      0,0,0,0,0,0,0,0);
	cashingframe = new CashingFrame(ritem2->getContent());
	FXShutterItem *ritem3 =
	    new FXShutterItem(rshutter,_("Tariff"),NULL,LAYOUT_FILL_X|LAYOUT_FILL_Y,
	                      0,0,0,0,0,0,0,0);
	tarifframe = new TarifFrame(ritem3->getContent());
	FXShutterItem *ritem4 =
	    new FXShutterItem(rshutter,_("Members"),NULL,LAYOUT_FILL_X|LAYOUT_FILL_Y,
	                      0,0,0,0,0,0,0,0);
	membersframe = new MembersFrame(ritem4->getContent());
	FXShutterItem *ritem6 =
	    new FXShutterItem(rshutter,_("Tickets"),NULL,LAYOUT_FILL_X|LAYOUT_FILL_Y,
	                      0,0,0,0,0,0,0,0);
	ticketsframe = new QTicketsBox(ritem6->getContent());

	FXShutterItem *ritem5 =
	    new FXShutterItem(rshutter,_("Staff"),NULL,LAYOUT_FILL_X|LAYOUT_FILL_Y,
	                      0,0,0,0,0,0,0,0);
	employeesframe = new EmployeesFrame(ritem5->getContent());

	vsplitter->setSplit(1,400);
	if (clientslist->getCurrentItem() != -1)
		onClientSelected(NULL,0,NULL);

	CCL_set_on_event_callback(onEventCallback,NULL);
	CCL_set_on_connect_callback(onConnectCallback,NULL);
	CCL_set_on_disconnect_callback(onDisconnectCallback,NULL);
	getApp()->addTimeout(this,ID_TIMERTICK,500);
	getApp()->addTimeout(this,ID_CHECKEVENTS,100);
	loginstat = 0;
	//init printing and warning structures
	for (int i=0; i<MAX_CLIENTS; i++) {
		print_job[i]=0;
		//warn_lvl[i]=;
		warn_lvl[i]=2;  //default to 2 minute warning
		job_pages[i]=0;
		prev_price[i]=0;
	}
	upfp = NULL;
	curUpdClient = 0;
	//Set active List Type
	ListType = LV_ICONLIST;
	//Load defaults
	applySettings(0);
	//settingsbox->loadSettings();
}

CCLWin::~CCLWin()
{
	delete bigicon;
	delete miniicon;
	delete timeicon;
	delete stopicon;
	delete unstopicon;
	delete swapicon;
	delete playicon;
	delete pauseicon;
	delete disconicon;
	delete newicon;
	delete delicon;
	delete emplogicon;
	delete msgicon;
	delete min10icon;
	delete min20icon;
	delete min30icon;
	delete min60icon;

	/*  delete min10btn;
	delete min20btn;
	delete min30btn;
	delete min60btn;
	*/
	for (int i = 0; i < 4; i++)
		delete bpcicons[i];
	//delete i_fonthandle;
}

void
CCLWin::create()
{
	FXMainWindow::create();

	bpcicons[CCL_INACTIVE]->create();
	bpcicons[CCL_ACTIVE]->create();
	bpcicons[CCL_PAUSED]->create();
	bpcicons[3]->create();
	bpcicons[4]->create();
	disconicon->create();
	clientslist->sortItems();
	clientslist2->sortItems();

	show();
}

FXbool
CCLWin::close(FXbool notify)
{
  if (FXMessageBox::question(this,MBOX_YES_NO,_("Exit Mkahawa"),
			     _("Do you really want to quit Nordseye?")) == MBOX_CLICKED_YES)
{
    FXMainWindow::close(TRUE);
    return TRUE;
  }
  return FALSE;
}

void CCLWin::loadClients()
{
	int client;

	for (FXuint i = 0; -1 != (client = CCL_client_get_nth(i)); i++) {
		if (!(CCL_client_flags_get(client) & CLIENT_DELETED)) {
			appendClient(client);
		}
	}

}

int CCLWin::appendClient(int client)
{
	FXString text = CCL_client_name_get(client);
	int idx = clientslist->findItem(text);
	FXFoldingItem *i = clientslist2->findItem(text);
	char buf[128];

	if (-1 != idx)
		//is this already in the icons list?
		return idx;

	CCL_client_flags_toggle(client,CLIENT_DELETED,FALSE);
	int status = CCL_client_status_get(client);
	CCLIconItem *item = new CCLIconItem(text,bpcicons[status],NULL,
	                                    (void *)client,disconicon);

	CCL_client_flags_toggle(client, CLIENT_CONNECTED, FALSE);
	item->setShowDisconIcon(TRUE);
	getClientInfoStr(client, buf, 128);
	idx = clientslist->appendItem(item,TRUE);
	i = clientslist2->appendItem(NULL, buf, NULL, NULL, (void *)client);
	clientslist->sortItems();
	clientslist2->sortItems();

	return idx;
}

void
CCLWin::deleteClient(int client)
{
	int idx = clientslist->findItem(CCL_client_name_get(client));
	FXFoldingItem *item = clientslist2->findItemByData((void *)client);

	if (-1 != idx)
		clientslist->removeItem(idx);
	if (item != NULL)
		clientslist2->removeItem(item);
}

void
CCLWin::setClientDisconnected(int client,FXbool disconnected)
{
	int idx = clientslist->findItem(CCL_client_name_get(client));

	if (-1 != idx) {
		((CCLIconItem*)clientslist->getItem(idx))->setShowDisconIcon(disconnected);
		clientslist->updateItem(idx);
		if (client == curUpdClient)   curUpdClient = 0;
		CCL_client_flags_toggle(client, CLIENT_CONNECTED, !disconnected);
	}
}

void
CCLWin::send_cmd_to_all(long cmd, char *arg, int arglen)
{
	int i, client;

	for (i=0; (-1 != (client=CCL_client_get_nth(i)));  i++) {
		CCL_client_send_cmd(client, cmd, arg, arglen);
	}
}

void
CCLWin::setAllClientMember(int client)
{
	int i, member = CCL_client_member_get(client);
	FXInputDialog *dlg = new FXInputDialog(this,_("Member"),
	                                       _("Member ID (0 for none):"),
	                                       NULL,INPUTDIALOG_INTEGER);
	dlg->setLimits(0,9999);
	dlg->setText(FXStringVal(member));
	if (dlg->execute()) {
		member = FXIntVal(dlg->getText());
		if ((CCL_member_exists(member) || 0 == member)) {
			int enablepassbtn = FALSE;

			for (i=0; (-1 != (client=CCL_client_get_nth(i)));  i++) {
				CCL_client_member_set(client,member);
				if (0 != member && CCL_data_key_exists(CCL_DATA_MEMBER,member,"password"))
					enablepassbtn = TRUE;
				CCL_client_send_cmd(client,CS_ENABLEPASSWORDEDIT,
				                    &enablepassbtn,sizeof(enablepassbtn));
			}
		} else
			FXMessageBox::error(this,MBOX_OK,_("Error"),_("Invalid member"), dbIcon2);
	}
	delete dlg;
}

void
CCLWin::setClientMember(int client)
{
	int smember = CCL_client_member_get(client);
	FXDialogBox dlg(this,_("Set Member to Client"));
	FXVerticalFrame *vframe =  new FXVerticalFrame(&dlg,LAYOUT_FILL_X|LAYOUT_FILL_Y);
	FXVerticalFrame *mlistframe = new FXVerticalFrame(vframe,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
	        0,0,0,0,0,0,0,0);
	FXFoldingList *mlist = new FXFoldingList(mlistframe,NULL,0,
	        LAYOUT_FILL_X|LAYOUT_FILL_Y|FOLDINGLIST_SINGLESELECT);
	FXHorizontalFrame *hframe = new FXHorizontalFrame(vframe,FRAME_SUNKEN|LAYOUT_FILL_X,
	        0,0,0,50,0,0,0,0);
	new FXButton(hframe,_("Cancel"),dbIcon2,&dlg,FXDialogBox::ID_CANCEL,
	             BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK|LAYOUT_LEFT);
	new FXButton(hframe,_("Set Member"),dbIcon2,&dlg,FXDialogBox::ID_ACCEPT,
	             BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT);
	mlist->appendHeader(_("Member Name"),NULL,170);

	dlg.resize(200,300);

	char buf[128];
	int  id=0, count=0;
	mlist->clearItems();
	snprintf(buf,128,"** NO MEMBER **");
	mlist->appendItem(NULL,buf,NULL,NULL,(void *)id);
	for (FXuint i = 0; -1 != (id = CCL_member_get_nth(i)); i++) {
		if (!(CCL_member_flags_get(id) & (MEMBER_DELETED |MEMBER_TICKET))) {
			//if (!(CCL_member_flags_get(id) & (MEMBER_DELETED))) {
			const char *name = NULL;

			name = CCL_member_name_get(id);
			snprintf(buf,128,"%s",name);
			mlist->appendItem(NULL,buf,NULL,NULL,(void *)id);
			count++;
		}
	}

	if (dlg.execute()) {
		FXFoldingItem *sitem = mlist->getCurrentItem();

		if (sitem) {
			smember = (long)(sitem->getData());
			int enablepassbtn = FALSE;
			int credit = CCL_member_credit_get(smember);
			if (!smember) {
				return;
			} else if ( (credit-CCL_client_owed_terminal(client)) > MINCREDIT ) {
				// check for enough credit
				CCL_client_member_set(client,smember);
				if (0 != smember && CCL_data_key_exists(CCL_DATA_MEMBER,smember,"password"))
					enablepassbtn = TRUE;
				CCL_client_send_cmd(client,CS_ENABLEPASSWORDEDIT,&enablepassbtn,sizeof(enablepassbtn));
			} else { // too little credit
				FXMessageBox::error(this,MBOX_OK,_("Overdrawn Account"),
				                    _("Replenish the account first."), dbIcon2);
			}
		}
	}
	return;
}

int CCLWin::getSelectedClient()
{
	int idx = clientslist->getCurrentItem();

	if (-1 == idx)
		return -1;
	else
		return (long)(clientslist->getItemData(idx));
}

void
CCLWin::showCashing()
{
	rshutter->setCurrent(1);
}

void
CCLWin::showProducts()
{
	rshutter->setCurrent(0);
}

void
print_hash(unsigned char *hash, int len)
{
	int i;
	for (i=0; i<len; i++)
		printf("%02X ", hash[i]);

	return;
}

FXbool
CCLWin::auth(int id,FXuchar *testpass)
{
	FXuchar *realpass;
	int size;
	int valid = FALSE;

	if (id > 0) {
		realpass = (FXuchar*)CCL_data_get_blob(CCL_DATA_MEMBER,id,"password",&size);
#ifdef DEBUG
		printf("Realpass Hash: ");
		print_hash((unsigned char *)realpass, CCL_MD5_DIGEST_LENGTH);
		printf("\n");
		printf("Testpass Hash: ");
		print_hash((unsigned char *)testpass, CCL_MD5_DIGEST_LENGTH);
		printf("\n");
#endif
		valid = (CCL_member_exists(id) && realpass &&
		         !memcmp(testpass, realpass, size));
		if (realpass) CCL_free(realpass);
		//CCL_employee_info_get(id, NULL, NULL, NULL, NULL, NULL,
		//			  &(e_inf.lvl), NULL, NULL, NULL);
	}
	return valid;
}

/*
  username
  testpass: Real test password. NOT the md5hash
*/
FXbool
CCLWin::authemp(FXuchar *usrname, FXuchar *testpass)
{
	FXuchar *realpass, md5hash[CCL_MD5_DIGEST_LENGTH];
	int id, size=0;
	int valid = FALSE;

	CCL_MD5(testpass, strlen((char *)testpass), md5hash);
	id = CCL_data_find_by_key_sval(CCL_DATA_EMPLOYEE, "username", (char *)usrname);
	//Inserting the admin
	if (id<0 && !strcmp((char *)usrname,"admin")) {
		CCL_data_set_string(CCL_DATA_EMPLOYEE, 1, "username", (char *)usrname);
		memset(md5hash, 0, sizeof(md5hash));
		CCL_MD5(usrname, strlen((char *)usrname), md5hash);
#ifdef DEBUG
		printf("Password Hash: ");
		print_hash((unsigned char *)md5hash, CCL_MD5_DIGEST_LENGTH);
		printf("\n");
#endif
		CCL_data_set_blob(CCL_DATA_EMPLOYEE, 1, "password", md5hash,  CCL_MD5_DIGEST_LENGTH);
		id = CCL_data_find_by_key_sval(CCL_DATA_EMPLOYEE, "username", (char *)usrname);
#ifdef DEBUG
		printf("authemp(): user %s not found. Setting Admin: %d\n", usrname, id);
#endif
	}
	if (id > 0) { /* the user name was found, now test the password */
		realpass = (FXuchar*)CCL_data_get_blob(CCL_DATA_EMPLOYEE,id,"password",&size);
#ifdef DEBUG
		printf("Realpass Hash: ");
		print_hash((unsigned char *)realpass, CCL_MD5_DIGEST_LENGTH);
		printf("\n");
		printf("Testpass Hash: ");
		print_hash((unsigned char *)md5hash, CCL_MD5_DIGEST_LENGTH);
		printf("\n");
#endif
		e_inf.lvl = 0;
		e_inf.empID = 0;
		valid = (CCL_employee_exists(id) && realpass && !memcmp(md5hash, realpass, size));
		if (valid) {
			CCL_employee_info_get(id, NULL, NULL, NULL, NULL, NULL,
			                      &(e_inf.lvl), NULL, NULL, NULL);
			e_inf.empID = id;
		}
#ifdef DEBUG
		printf("authemp(): %s -> %d, valid = %d, pwdsize = %d\n", usrname, id,
		       valid, size);
#endif
		if (realpass) CCL_free(realpass);
	}
#ifdef DEBUG
	printf("Empusr: %s -> %d, permission = %08X\n", usrname, id,
	       e_inf.lvl);
	e_inf.lvl = 0x7FFFFFFF;
#endif
	return valid;
}

FXbool
authemp2(FXuchar *usrname, FXuchar *testpass)
{
	//FXuchar *realpass;
	char *realpass;
	int size, id;
	int valid = FALSE;

	if (strlen((char *)usrname)<=0) return FALSE;
	//id = CCL_data_find_by_key_sval(CCL_DATA_EMPLOYEE, "empusr", (char *)usrname);
	id = CCL_employee_validate((char *)usrname, (char *)testpass);
	if (id > 0) {
		valid = TRUE;
		CCL_employee_info_get(id, NULL, NULL, NULL, NULL, NULL,
		                      &(e_inf.lvl), NULL, NULL, NULL);
		e_inf.empID = id;
		if (id == 1) //admin has all permissions all the time
			e_inf.lvl = 0x7FFFFFFF;
	} else { //no ID was found

	}
#ifdef DEBUG
	printf("Empusr: %s -> %d, permission = %08X\n", usrname, id, e_inf.lvl);
#endif
	return valid;
}

struct PrintInfo {
	char prn[128];
	char usr[128];
	char jnr[10];
	char dt[50];
	char pgnr[10];
	char cps[5];
	char dash[5];
	char bill[128];
};

void
print_print_info(PrintInfo *pi)
{
	printf("%s %s %s %s] %s %s %s %s\n", pi->prn, pi->usr, pi->jnr,
	       pi->dt, pi->pgnr, pi->cps, pi->dash, pi->bill);
}

unsigned int
CCLWin::getClientIndex(int client)
{
	int num = clientslist->getNumItems();
	unsigned int idx;

	for (idx=0; idx<num; idx++) {
		if (client == (long) (clientslist->getItemData(idx)))
			break;
	}
	return idx;
}

unsigned long
getIPFromStr(char *ipstr)
{
	int v1=0, v2=0, v3=0, v4=0;
	unsigned long addr;

	if (!ipstr) return 0;
	sscanf(ipstr, "%d.%d.%d.%d", &v1, &v2, &v3, &v4);
	addr = (v4<<24) | (v3<<16) | (v2<<8) | v1;
#ifdef DEBUG
	printf("getIPFromStr(): %08X: %02X %02X %02X %02X\n", addr,
	       v1, v2, v3, v4);
#endif
	return addr;
}

/*
  cupstr   : a cups page_log lien
  client   : pointer to the printer client

  return value:  The number of pages
                *client is the client number
*/
unsigned int
CCLWin::getPageCount(char *cupstr, int *lclient)
{
	struct PrintInfo pi;
	int n = 0, pjob = 0, clidx, num, idx;
	char dt2[20];
	unsigned long cliaddr;
	char *cp;

	cp = cupstr;
	while (*cp && *cp!='\n') {
		cp++;
		n++;
	}

	memset(&pi, 0, sizeof(pi)); //zero the structure
	sscanf(cupstr, "%s %s %s %s %s %s %s %s %s",
	       pi.prn, pi.usr, pi.jnr, pi.dt, dt2, pi.pgnr,
	       pi.cps, pi.dash, pi.bill);
#ifdef DEBUG_PRINT
	print_print_info(&pi);
#endif
	//in case the print server is also a client
	if (!strncmp(pi.bill, "localhost", 9))
		cliaddr = CCL_client_ip_get(*lclient);
	else
		cliaddr = getIPFromStr(pi.bill);
	//sequential search for the client with the IP address
	num = clientslist->getNumItems();
	for (idx = 0; idx < num; idx++) {
		*lclient = (long) (clientslist->getItemData(idx));
#ifdef DEBUG
		printf("[%s = %08X]\n", CCL_client_name_get(*lclient), CCL_client_ip_get(*lclient));
#endif
		if (cliaddr == CCL_client_ip_get(*lclient)) {
			if (strncasecmp(CCL_client_name_get(*lclient), "PrintServer", 9))
				//filter out the PrintServer client
				break;
		}
	}
	if (idx == num) { //did not find any - so it is a side printout
		*lclient = 0;
	}
	clidx = *lclient % MAX_CLIENTS; //mainwin->getClientIndex(client);
	n = atoi(pi.cps);
#ifdef DEBUG
	printf("getPageCount(): Total Counted Pages: %d\n", n);
#endif
	if (n<0) n = 0;
	return n;
}

int
getWarnLevel(int wval)
{
	int i;
	int w_lvls[6] = {1,2,5,10,15,20};
	wval = (wval / 60);

	for (i=0; i<0; i++) {
		if (wval >= w_lvls[i])
			return w_lvls[i];
	}
	return 0;
}

bool
CheckWarnClient(int client, int secs, int cash)
{
	bool retval = FALSE;
	char cmsg[64];
	int w_lvl = getWarnLevel(secs);

	if (w_lvl && (warn_lvl[client] != w_lvl)) {
		//new warning
		sprintf(cmsg,"You have less than %d minutes to go!", w_lvl);
		CCL_client_send_cmd(client, CS_DISPLAYMESSAGE, cmsg, strlen(cmsg));
		warn_lvl[client] = w_lvl;
		retval = TRUE;
#ifdef DEBUG_WARN
		printf("CheckWarnClient(): Warning Changed Level: %d\n", w_lvl);
#endif
	}
#ifdef DEBUG_WARN
	printf("CheckWarnClient(): Warning Level: %d", w_lvl);
#endif
	return retval;
}

int
CCLWin::getCurrentClient()
{
	int current = clientslist->getCurrentItem();
	if (-1 == current)
		return -1;
	return (long) (clientslist->getItemData(current));
}


bool
confirmLogin(int client)
{
	char buf[64];
	bool retval = FALSE;

	/* snprintf(buf, 64, "Login requested at Client %s",
	   CCL_client_name_get(client));
	if (FXMessageBox::question(mainwin,MBOX_YES_NO,_("Login Requested"),
			     buf) == MBOX_CLICKED_YES) {
	  retval = TRUE;
	  }*/
	retval = TRUE;
	return retval;
}


void
onEventCallback(int client,FXuint cmd,void *data,FXuint size,void *userdata)
{
	time_t usedtime = CCL_client_time_used(client);
	FXuint owed = CCL_client_owed_terminal(client);
	FXuint products = CCL_client_owed_products(client);
	time_t timeout = CCL_client_timeout_get(client);
	// Values in network byte order
	FXuint nusedtime = CCL_htonl(usedtime);
	FXuint nowed = CCL_htonl(owed);
	FXuint nproducts = CCL_htonl(products);
	FXuint ntimeout = CCL_htonl(timeout);
	FXuint member = CCL_client_member_get(client);
	int owed_val=owed;
	int owed_mins = 0;
	int credit;
	long resp;


	if ( (!cmd) || (cmd>CC_MAXCMDNR)) return; //disregard stray commands

	//CheckWarnClient(client, timeout, owed);

	if (member != 0) {  /* MEMBER & TICKET */
		credit = CCL_member_credit_get(member);
		owed_val = credit - owed;
		if (owed_val > 0) { //still some credit left
			int tarif = CCL_member_tarif_get(member);
			if (!tarif)
				tarif = CCL_tarif_get();
			int hourrate = (int)CCL_tarifpart_hourprice_get(tarif);
#ifdef DEBUG_CREDIT
			printf("[tarif: %d][hourrate: %d] [owed_val: %d] [usedtime: %ld]\n",
			       tarif, hourrate, owed_val, usedtime);
#endif
			nowed = CCL_htonl(owed_val);
			if (hourrate>0)
				owed_mins = owed_val / (hourrate/60);
			//CheckWarnClient(client, owed_mins, owed_val);
		} else {    //used up the time so stop the station
			CCL_client_stop(client); //only stops counting and resets flags
			CCL_client_member_set(client, 0);       //detach member
			CCL_client_send_cmd(client,CS_STOP,NULL,0);
			CCL_client_send_cmd(client,CS_LOCKSCREEN,NULL,0);
			CCL_log_session(client,credit,PAID, mainwin->getEmployeeID());
			CCL_member_credit_set(member, 0);
			CCL_member_flags_toggle(member, MEMBER_LOGGEDIN, FALSE);
		}
		// End of member processing
	}
#ifdef DEBUG
	printf("Message cmd=%d received\n", cmd);
#endif

	switch (cmd) {
	case CC_GETSTATUS:
		updateClientStatus(client);
		break;
	case CC_GETTIME:
		CCL_client_send_cmd(client,CS_SETTIME,&nusedtime,sizeof(nusedtime));
		break;
	case CC_GETTIMEOUT:
		CCL_client_send_cmd(client,CS_SETTIMEOUT,&ntimeout,sizeof(ntimeout));
		break;
	case CC_GETOWED:
		CCL_client_send_cmd(client,CS_SETOWED,&nowed,sizeof(nowed));
		CCL_client_send_cmd(client,CS_SETADDITIONAL,&nproducts,
		                    sizeof(nproducts));
		break;
	case CC_USEREXIT:
		if (CCL_ACTIVE == CCL_client_status_get(client)) {
			CCL_client_stop(client);

			FXuint member = CCL_client_member_get(client);
			if (!member) {
				CCL_log_session(client,CCL_client_owed_terminal(client),NOTPAID,
				                mainwin->getEmployeeID());
				//Block session - until paid or specifically allowed
				mainwin->blockClient(client);
				CCL_client_send_cmd(client,CS_LOCKSCREEN,NULL,0);
				CCL_client_send_cmd(client,CS_STOP,NULL,0);
				notpaidframe->readNotPaid();
			} else {
				int    owed = CCL_client_owed_terminal(client);
				int    credit = CCL_member_credit_get(member);

				CCL_log_session(client,owed,PAID,mainwin->getEmployeeID());
				CCL_member_credit_set(member, credit-owed);
				CCL_member_flags_toggle(member, MEMBER_LOGGEDIN, FALSE);
				CCL_client_send_cmd(client,CS_LOCKSCREEN,NULL,0);
				CCL_client_send_cmd(client,CS_STOP,NULL,0);
			}
		} else { //starting up the client
			CCL_client_reset(client);
		}
		CCL_client_flags_toggle(client,USERSTOP,TRUE);
		CCL_client_member_set(client, 0);
		break;
	case CC_USERLOGIN:
		//if (CCL_client_flags_get(client) & ALLOWUSERLOGIN) {
		//if (CCL_client_flags_get(client) & USERSTOP) {
		//  CCL_client_unstop(client);
		//  notpaidframe->readNotPaid();
		//} else
		if (confirmLogin(client)) {
			if (CCL_client_status_get(client)!=CCL_PAUSED)
				job_pages[client % MAX_CLIENTS] = 0;
			CCL_client_start(client);
			CCL_client_flags_toggle(client,USERSTOP,FALSE);
			updateClientStatus(client);
		}
		//      }
		break;
	case CC_MEMBERLOGIN:
	case CC_MEMBERLOGINWITHNAME:
		if (CCL_client_flags_get(client) & ALLOWMEMBERLOGIN) {
			int memberid;
			FXuchar *md5hash;

			if (CC_MEMBERLOGIN == cmd) {
				memberid = CCL_ntohl(((FXuint*) data)[0]);
				md5hash = (FXuchar*) ((FXuint*)data+1);
#ifdef DEBUG
				printf("CC_MEMBERLOGIN: Member %s tried to log in: pass=%s\n", data, md5hash);
#endif
			} else {
				char *c;

				memberid =
				    CCL_data_find_by_key_sval(CCL_DATA_MEMBER,"login_name",(char*)data);
				for (c = (char*)data; *c != '\0'; c++)
					;
				md5hash = (FXuchar*)(c+1);
#ifdef DEBUG
				printf("CC_MEMBERLOGINWITHNAME: Member %s tried to log in: pass=%s\n", data, md5hash);
#endif
			}

			if (-1 != memberid &&
			    mainwin->auth(memberid,md5hash) &&
			    !(CCL_member_flags_get(memberid) & MEMBER_LOGGEDIN)) {
#ifdef DEBUG
				printf("CC_MEMBERLOGIN: Member %s authenticated\n", data);
#endif
				//check balance
				if (CCL_member_credit_get(memberid) > MINCREDIT) {
					if (confirmLogin(client)) {
						CCL_client_send_cmd(client,CS_UNLOCKSCREEN,NULL,0);
						if ((CCL_client_flags_get(client) & USERSTOP) &&
						    CCL_client_member_get(client) == memberid) {
							CCL_client_unstop(client);
							notpaidframe->readNotPaid();
						} else {
							if (CCL_client_status_get(client)!=CCL_PAUSED)
								job_pages[client % MAX_CLIENTS] = 0; //initialize the job_pages
							CCL_client_start(client);
						}
						CCL_client_member_set(client,memberid);
						CCL_member_flags_toggle(memberid, MEMBER_LOGGEDIN, TRUE); //mark as logged in
						CCL_client_flags_toggle(client,USERSTOP,FALSE);
						updateClientStatus(client);
					}
				} else {
					char buf[64], *cp;
					cp = _("account has insufficient credit");
					snprintf(buf,64,"%s %s",
					         CCL_member_name_get(memberid), cp);
					//FXMessageBox::information(mainwin,MBOX_OK,_("Member Login Failure"),
					//buf, dbIcon2);
				}
			} else {
#ifdef DEBUG
				printf("CC_MEMBERLOGIN: Member %s NOT authenticated\n", data);
#endif
			}
		}
		break;

		/************************************/
	case CC_TICKETLOGIN:
#ifdef DEBUG_TICKET
		printf("CC_TICKETLOGIN: Ticket Attempts to log in\n");
#endif
		if (CCL_client_flags_get(client) & ALLOWTICKETLOGIN) {
			char *cp, *c, tktstr[33];
			int memberid, i;
			//remove non-alphanumeric characters
			//bzero(tktstr, 32);
			memset(tktstr,0,32);
			cp = (char *)tktstr;
			c = (char*)data;
			//for (i=0; i<32 && *c; i++, c++){
			for (i=0; i<32 && i<size; i++, c++) {
				if (isalnum(*c))
					*cp++ = toupper(*c);
			}
			*cp = 0;  //null - terminate this string
			//check if in DB
			memberid = CCL_member_ticket_find(tktstr);
#ifdef DEBUG_TICKET
			printf("CC_TICKETLOGIN: Ticket [%s] tried to log in as [%s]\n", tktstr, data);
#endif
			if (memberid == -1) { //not found - do nothing
				return;
			}

			if (-1 != memberid ) {
#ifdef DEBUG_TICKET
				printf("CC_TICKETLOGIN: Ticket %s authenticated\n", data);
#endif
				//check balance
				if (CCL_member_credit_get(memberid) > MINCREDIT) {
					if (confirmLogin(client)) {
						CCL_client_send_cmd(client,CS_UNLOCKSCREEN,NULL,0);
						if ((CCL_client_flags_get(client) & USERSTOP) &&
						    CCL_client_member_get(client) == memberid) {
							CCL_client_unstop(client);
							notpaidframe->readNotPaid();
						} else {
							if (CCL_client_status_get(client)!=CCL_PAUSED)
								job_pages[client % MAX_CLIENTS] = 0; //initialize the job_pages
							CCL_client_start(client);
						}
						CCL_client_member_set(client,memberid);
						CCL_member_flags_toggle(memberid, MEMBER_LOGGEDIN, TRUE); //mark as logged in
						CCL_client_flags_toggle(client,USERSTOP,FALSE);
						updateClientStatus(client);
					}
				} else {
					char buf[64], *cp;
					cp = _("account has insufficient credit");
					snprintf(buf,64,"%s %s",
					         CCL_member_name_get(memberid), cp);
					//FXMessageBox::information(mainwin,MBOX_OK,_("Member Login Failure"),
					//		      buf, dbIcon2);
				}
			} else {
#ifdef DEBUG_TICKET
				printf("CC_TICKETLOGIN: Ticket %s NOT authenticated\n", data);
#endif
			}
		}
		break;


		/***********************************/

	case CC_USERPRINTED: {
		int pgcnt=0, pid=0, pclient;
		char prnname[256];
		char *cp, *buf;

#ifdef DEBUG_PRINT
		printf("User Printing: [%s]\n%s\n", CCL_client_name_get(client),
		       (char *)data);
#endif
		if (strncasecmp(CCL_client_name_get(client), "PrintServer", 11)) {
#ifdef DEBUG_PRINT
			printf("Only counts from PrintServer\n");
#endif
			break; //Only honor PrintServer printing
		}
		cp = (char *)data;
		buf = (char *)data;
		//Report printing, line by line
		while (*cp) {
			char *np;
			int lnlen;

			np = NULL;
			//np = index(cp, '\n');
			np = strchr(cp, '\n');
			lnlen = (np!=NULL)?(np-cp): (buf+size-cp); //line length
#ifdef DEBUG
			printf("[Line Length= %02d] [Buffer= %03d]\n", lnlen, size);
#endif
			if (lnlen < 20) {
				break;  //misread
			}
			{
				//reportPrinting(cp, lnlen); //send to server
				pclient = client; //the print server
				pgcnt = mainwin->getPageCount(cp, &pclient);
				sscanf(cp, "%s", prnname);
				//search for the printer product id in the database
				CCL_product_id_get(prnname, &pid);
				if (!pid) {
#ifdef DEBUG_PRINT
					printf("onEventCallBack(): Printing Product ID was not found\n");
#endif
					//create a new printer product
					pid = CCL_product_new("Printing", prnname, 0);
				}
				if (pgcnt && pid>0 && !(CCL_product_flags_get(pid) & CCL_DELETEDPRODUCT) ) {
					//the printer exists, so make the sale
					// first, handle a case where the user has stopped the session
					if (CCL_INACTIVE == CCL_client_status_get(pclient)) {
						int stime = CCL_client_stime_get(pclient);
						int etime = CCL_client_etime_get(pclient);
						int session = CCL_log_session_find(pclient, stime, etime);

						//if it is in the unpaid list
						if (notpaidframe->isUnpaid(pclient, session)) {
							CCL_client_unstop(pclient);//briefly continue session
							//add the printing product
							CCL_client_product_add(pclient, pid, pgcnt);
							CCL_client_stop(pclient);//stop and re-log session
							CCL_log_session(pclient,CCL_client_owed_terminal(pclient)
							                ,NOTPAID, mainwin->getEmployeeID());
							//show the session
							notpaidframe->readNotPaid();
							//refresh the contents of cashing frame if currently shown
							if (cashingframe->getSession() == session)
								cashingframe->setSession(0);
						}
					} else { //user's session continues
						CCL_client_product_add(pclient, pid, pgcnt);
					}
					//update the tally in products frame
					productsframe->delProduct(pid);
					productsframe->addProduct(pid);
					//Now, update display if this client is current
					if (pclient == mainwin->getCurrentClient()) {
						productsframe->updateClientProducts(pclient);
					}
					//productsframe->updateSaleProducts();
#ifdef DEBUG_PRINT
					printf("onEventCallBack(): Client: %s :- Printing [%s]: %d Pages\n",
					       CCL_client_name_get(pclient), prnname, pgcnt);
#endif
				} else if (pid > 0) {
#ifdef DEBUG_PRINT
					printf("onEventCallBack(): Printing not added\n");
#endif
				} else {
					//add this
#ifdef DEBUG_PRINT
					printf("onEventCallBack(): Printing Product ID was not found\n");
#endif
				}
			}
			cp=cp+lnlen+1;
		}
	}
	break;
	case CC_CHATCLIENT:
		/* Alert sound first*/

		/* Raise the message box */
		/*if (!clientHelpIsUp(client)){
		char msg[256];
		sprintf(msg, "Client %s asks for help.", CCL_client_name_get(client));
		FXMessageBox
		  mbox(this,_("Client Message"), _("Client asks for help."),
		       bigicon,MBOX_OK|DECOR_TITLE|DECOR_BORDER);

		mbox.execute();
		     }
		     */
		break;

	case CC_SETMEMBERPASSWORD:
		if (mainwin->auth(CCL_client_member_get(client),(FXuchar*)data)) {
			CCL_data_set_blob(CCL_DATA_MEMBER,CCL_client_member_get(client),
			                  "password",((FXuchar*)data)+CCL_MD5_DIGEST_LENGTH,
			                  CCL_MD5_DIGEST_LENGTH);
		} else {
			const char * message = _("Wrong old password.");

			CCL_client_send_cmd(client,CS_DISPLAYMESSAGE,message,strlen(message)+1);
		}
		break;
	case CC_CALLASSIST:
		char buf[100], *cp;

		cp = "requests for help.";
		sprintf(buf, "[ %s ] %s", CCL_client_name_get(client), cp);
		CCL_client_send_cmd(client,CS_CALLASSIST, NULL,0);
#ifdef DEBUG
		printf ("Responded to Call Assist\n");
#endif
		break;
	case CC_UPDATE:
		//response to CS_UPDATE
		long resp;

		resp = CCL_ntohl(((FXuint*) data)[0]);
#ifdef DEBUG
		printf ("CC_UPDATE: %d\n", resp);
#endif
		if (resp) {
			mainwin->curUpdClient = client;
			//start sending the data
			//mainwin->sendUpdateChunk(client, 0);
#ifdef WIN32
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&sendUpdateChunks, (LPVOID)client, 0, &updth);
#else
			pthread_create(&updth, NULL, (void *(*)(void *))sendUpdateChunks, (void *)client);
#endif
			//pthread_create(&updth, NULL, (void *(*)(void *))sendUpdateChunks, (void *)client);
		}
		break;
	case CC_UPDATEDATA:
		//response to updatedata
		//resp = CCL_ntohl(((FXuint*) data)[0]);
		/*if (resp) {
		//start sending the data
		mainwin->sendUpdateChunk(client, resp);
		}*/
		break;
	case CC_UPDATEEND:
		resp = CCL_ntohl(((FXuint*) data)[0]);
		mainwin->curUpdClient = 0;
		if (resp) { //success
			//release data
#ifdef DEBUG_UPD
			printf("CC_UPDATEEND: Update Success: updFileCount = [%d]\n",
			       mainwin->updFileCount);
#endif
			if (mainwin->updFileCount) {
				mainwin->updFileCount--;
				if (!mainwin->updFileCount) { //only message when complete
					char errstr[128];

					sprintf(errstr, "Client %s update was successful",
					        CCL_client_name_get(client));
#ifdef DEBUG_UPD
					printf("CC_UPDATEEND: Update Success: %s\n", errstr);
#endif
					//cp = strdup(errstr);
					//pthread_create(&msgth, NULL, dispMessage, (void *)cp);
				} else
					mainwin->doNextUpdateFile(client);
			}
		} else {
			char *cp, errstr[64];

			cp = (char *)data;
			if (size > sizeof(FXuint)) cp += sizeof(FXuint);
			snprintf(errstr, 64, "Update Failed: %s", cp);
#ifdef DEBUG_UPD
			printf("CC_UPDATEEND: Update Failed\n");
#endif
			//cp = strdup(errstr);
			//pthread_create(&msgth, NULL, &dispMessage, (void *)cp);
		}
		break;
	}
}

void
updateClientStatus(int client)
{
	if (CCL_client_status_get(client) == CCL_ACTIVE) {
		time_t usedtime = CCL_client_time_used(client);
		FXuint owed = CCL_client_owed_terminal(client);
		FXuint products = CCL_client_owed_products(client);
		time_t timeout = CCL_client_timeout_get(client);
		int member = CCL_client_member_get(client);
		int enablepassbtn = CCL_htonl(0);
		// Values in network byte order
		FXuint nusedtime = CCL_htonl(usedtime);
		FXuint nowed = CCL_htonl(owed);
		FXuint nproducts = CCL_htonl(products);
		FXuint ntimeout = CCL_htonl(timeout);
		int owed_val=owed;
		int owed_mins = 0;

		if (member != 0) {
			owed_val = (int)(CCL_member_credit_get(member) - owed);
			if (owed_val > 0) {
				int tarif = CCL_member_tarif_get(member);
				if (!tarif) tarif = CCL_tarif_get();
				int hourrate = CCL_tarifpart_hourprice_get(tarif);

				nowed = CCL_htonl(owed_val);
#ifdef DEBUG
				printf("[tarif: %d][hourrate: %d] [owed_val: %d][time: %ld]\n",
				       tarif, hourrate, owed_val, usedtime);
#endif
				if (hourrate > 0)
					owed_mins = owed_val / (hourrate/60);
			}
		}
		if (0 != member && CCL_data_key_exists(CCL_DATA_MEMBER,member,"password"))
			enablepassbtn = CCL_htonl(1);
		CCL_client_send_cmd(client,CS_START,NULL,0);
		CCL_client_send_cmd(client,CS_SETTIME,&nusedtime,sizeof(nusedtime));
		CCL_client_send_cmd(client,CS_SETTIMEOUT,&ntimeout,sizeof(ntimeout));
		CCL_client_send_cmd(client,CS_SETOWED,&nowed,sizeof(nowed));
		CCL_client_send_cmd(client,CS_SETADDITIONAL,&nproducts,sizeof(nproducts));
		CCL_client_send_cmd(client,CS_UNLOCKSCREEN,NULL,0);
		CCL_client_send_cmd(client,CS_ENABLEPASSWORDEDIT,
		                    &enablepassbtn,sizeof(enablepassbtn));
	} else {
		CCL_client_send_cmd(client,CS_STOP,NULL,0);
		CCL_client_send_cmd(client,CS_LOCKSCREEN,NULL,0);
	}

	int cli_setting = CCL_data_get_int(CCL_DATA_CLIENT, client, "client_settings", -1);
	if (cli_setting == -1) {
		//settings have not been made. Effect default settings
		unsigned int cs =  cyber_settings->getOpMode();
		if (cs & OPMODE_POSTPAID)
			CCL_client_flags_toggle(client, ALLOWUSERLOGIN, TRUE );
		if (cs & OPMODE_TICKET)
			CCL_client_flags_toggle(client, ALLOWTICKETLOGIN, TRUE);
		if (cs & OPMODE_MEMBER)
			CCL_client_flags_toggle(client, ALLOWMEMBERLOGIN, TRUE);
		mainwin->toggleClientSetting(client, CCL_client_flags_get(client));
	}
	int nallowuserlogin =
	    CCL_htonl(CCL_client_flags_get(client) & ALLOWUSERLOGIN);
	CCL_client_send_cmd(client,CS_ALLOWUSERLOGIN,&nallowuserlogin,
	                    sizeof(nallowuserlogin));
	int nallowmemberlogin =
	    CCL_htonl(CCL_client_flags_get(client) & ALLOWMEMBERLOGIN);
	CCL_client_send_cmd(client,CS_ALLOWMEMBERLOGIN,&nallowmemberlogin,
	                    sizeof(nallowmemberlogin));
	int nallowticketlogin =
	    CCL_htonl(CCL_client_flags_get(client) & ALLOWTICKETLOGIN);
	CCL_client_send_cmd(client,CS_ALLOWTICKETLOGIN,&nallowticketlogin,
	                    sizeof(nallowticketlogin));
}

void
onConnectCallback(int client,void *userdata)
{
	mainwin->appendClient(client);
	mainwin->setClientDisconnected(client,FALSE);
	unsigned long flags = CCL_client_flags_get(client);

	if (!(flags & CLIENT_BLOCKED)) {
#ifdef DEBUG_SETTINGS
		printf("onConnectionCallback(): Client [%d] - Flags [%08X]\n", client, flags);
#endif
		mainwin->unBlockClient(client);
	}
	updateClientStatus(client);
#ifdef DEBUG_CALLBACK
	printf("onConnectionCallback(): Client [%d] - Flags [%08X]\n", client, flags);
#endif
}

void
onDisconnectCallback(int client,void *userdata)
{
#ifdef DEBUG_CALLBACK
	unsigned long flags = CCL_client_flags_get(client);
	printf("onDisconnectCallback(): Client [%d] - Flags [%08X]\n", client, flags);
#endif
	mainwin->setClientDisconnected(client,TRUE);
}

long
CCLWin::onCheckEvents(FXObject*,FXSelector,void*)
{
	CCL_check_events();
	getApp()->addTimeout(this,ID_CHECKEVENTS,100);
	return 1;
}

long
CCLWin::unBlockClient(int client)
{
	int allow;
	int cli_setting = CCL_data_get_int(CCL_DATA_CLIENT, client, "client_settings", -1);

	CCL_client_flags_toggle(client, CLIENT_BLOCKED, FALSE);
	//Effect unblock
	if (cli_setting & ALLOWUSERLOGIN) {
		CCL_client_flags_toggle(client, ALLOWUSERLOGIN, TRUE);
		allow = CCL_htonl(ALLOWUSERLOGIN);
		CCL_client_send_cmd(client, CS_ALLOWUSERLOGIN, &allow, sizeof(allow));
	}
	if (cli_setting & ALLOWMEMBERLOGIN) {
		CCL_client_flags_toggle(client, ALLOWMEMBERLOGIN, TRUE);
		allow = CCL_htonl(ALLOWMEMBERLOGIN);
		CCL_client_send_cmd(client, CS_ALLOWMEMBERLOGIN, &allow, sizeof(allow));
	}
	if (cli_setting & ALLOWTICKETLOGIN) {
		CCL_client_flags_toggle(client, ALLOWTICKETLOGIN, TRUE);
		allow = CCL_htonl(ALLOWTICKETLOGIN);
		CCL_client_send_cmd(client, CS_ALLOWTICKETLOGIN, &allow, sizeof(allow));
	}
}

long
CCLWin::blockClient(int client)
{
	int allow;
	int cli_setting = CCL_data_get_int(CCL_DATA_CLIENT, client, "client_settings", -1);

	//Block session - until paid or specifically allowed
	CCL_client_flags_toggle(client, ALLOWUSERLOGIN, FALSE);
	CCL_client_flags_toggle(client, ALLOWMEMBERLOGIN, FALSE);
	CCL_client_flags_toggle(client, ALLOWTICKETLOGIN, FALSE);
	//set blocking flags
	CCL_client_flags_toggle(client, CLIENT_BLOCKED, TRUE);
	//cli_setting |= CLIENT_BLOCKED;
	//CCL_data_get_int(CCL_DATA_CLIENT, client, "client_settings", cli_setting);
	//effect blocking
	allow = CCL_htonl(0);
	CCL_client_send_cmd(client, CS_ALLOWUSERLOGIN, &allow, sizeof(allow));
	CCL_client_send_cmd(client, CS_ALLOWMEMBERLOGIN, &allow, sizeof(allow));
	CCL_client_send_cmd(client, CS_ALLOWTICKETLOGIN, &allow, sizeof(allow));
}

/*
  File selection dialog. The selected filename will be placed in *fname
  The return value is the size of the file
*/
long
CCLWin::getUpdateFileName(char *fname, int *buf)
{
	//get update filename
	FXString cwd = FXSystem::getHomeDirectory() + "/";
	char *updpath = CCL_data_get_string(CCL_DATA_NONE,0,"update/path",cwd.text());
	FXString input = updpath;//cwd.text();
	FXFileDialog *fd;
	int retval = 0;

	CCL_free(updpath);
	fd = new FXFileDialog(this, input);
	{
		FXString filename;
		filename = fd->getOpenFilename(this, _("Client Update File"),
		                               input, _("*.upd,*.upx"), 0);
		filename.trim();
		if (FXStat::exists(filename)
		    || FXMessageBox::question(this,MBOX_YES_NO,_("No File Selected"),
		                              _("Select an Update File?"),
		                              filename.text()) == MBOX_CLICKED_YES, dbIcon2) {
			snprintf(fname, 256, "%s", filename.text());
			retval = FXStat::size(filename);
			//store path
			CCL_data_set_string(CCL_DATA_NONE,0,"update/path",filename.text());
		}
	}
	delete fd;
	return retval;
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

int
CCLWin::doNextUpdateFile(int client)
{
	char buf[64], fname[64], cp[128], *bp;

	if (fgets(buf, 64, upfpx)) {
		sscanf(buf, "%s", fname);
		strncpy(cp, flname,128);
		if (bp = strrchr(cp, '/')) *(++bp)=0; //delete filename part
		strncat(cp, fname, 64);
		startUpdateClient(client, cp);
#ifdef DEBUG_UPD
		printf("doNextUpdateFile(): fname = %s\n", cp);
#endif
	}
	return 0;
}

int
CCLWin::startUpdateClient(int client, char *fname)
{
	char *msgstr;
	int retval;

	upfp = fopen(fname, "r");
	if (!upfp) {
		msgstr = _("Unable to open update file\n");
#ifdef DEBUG_UPD
		printf("startUpdateClient(): %s [%s]\n", msgstr, fname);
#endif
		return 0;
	}
	memset(&upinfo, 0, sizeof(upinfo));
	retval = fread(&upinfo, sizeof(upinfo), 1, upfp);
	if (!retval) {
		msgstr = _("Unable to read update file\n");
#ifdef DEBUG_UPD
		printf("startUpdateClient(): %s\n", msgstr);
#endif
	}
	//Initiate update
#ifdef DEBUG_UPD
	msgstr = "Initiating Update: ";
	printf("startUpdateClient(): %s -%s\n", msgstr,fname);
	print_update_info(&upinfo);
#endif
	//prgrs->execute();
	/* prgrs->raise();
	//prgrs->show();
	//repaint();
	//prgrs->repaint();
	//prgrs->setTotal(FXuint(upinfo.fsiz
	//prgrs->setProgress(0);e));
	//prgrs->setProgress(FXuint(1));
	*/
	CCL_client_send_cmd(client, CS_UPDATE, &upinfo, sizeof(upinfo));
	return retval;
}

long
CCLWin::updateClient(int client)
{
	int retval, fd;
	char fname[256], *msgstr, *cp;
	int status = CCL_client_status_get(client);
	char hdrstr[100], sgnstr[32], filecntstr[32];

	msgstr = NULL;
	/*Must be admin*/
	if (getEmployeeID() != 1) {
		msgstr = _("Only Admin can update clients.");
#ifdef DEBUG_UPD
		printf("updateClient(): %s [employee = %d]\n", msgstr, getEmployeeID());
#endif
	}
	if (!(CCL_client_flags_get(client) & CLIENT_CONNECTED)) {
		msgstr = _("Only connected Clients can be updated");
#ifdef DEBUG_UPD
		printf("updateClient(): %s [client = %d]\n", msgstr, client);
#endif
		return 0;
	}
	if (curUpdClient) { // a client is being updated
		FXMessageBox::information(getRoot(),MBOX_OK,_("Update in Progress"),
		                          _("Updates can only be made one at a time."));
		return 0;
	}
	if (status & CCL_ACTIVE) {
		FXMessageBox::information(getRoot(),MBOX_OK,_("Active Client"),
		                          _("End the client session before updating."));
		return -1;
	}
	//obtain the update filename
	if (!(retval = getUpdateFileName(fname, &fd)) ) {
		msgstr = _("Update File not found\n");
#ifdef DEBUG_UPD
		printf("updateClient(): %s [%s]\n", msgstr, fname);
#endif
		return 0;
	}
	//open the file
	if(!(upfpx = fopen(fname, "r"))) {
		msgstr = _("Unable to open file\n");
#ifdef DEBUG_UPD
		printf("updateClient(): %s\n", msgstr);
		printf("updateClient(): Filename [%s]\n", fname);
#endif
		return 0;
	}
	//CALL THE UPDATER THREAD HERE
	//check signature and decided how to update
	if (fgets(hdrstr, 100, upfpx)) {
		if (!strncmp(hdrstr, "UTUF", 4)) { //an update file
			fclose(upfpx);
			updFileCount = 1;
			startUpdateClient(client, fname);
		} else if (!strncmp(hdrstr, "UTUL", 4)) { //an update list
			//read the header (number of files)
			strncpy(flname, fname, 128);
			if (fgets(filecntstr, 32, upfpx)) {
				updFileCount = atoi(filecntstr);
				if (updFileCount)
					doNextUpdateFile(client);
			}
		} else {
			msgstr = _("Not a Mkahawa Update File\n");
#ifdef DEBUG_UPD
			printf("updateClient(): %s\n", msgstr);
#endif
		}
	}
#ifdef DEBUG_UPD
	msgstr = "Started Update";
	printf("updateClient(): %s\n", msgstr);
#endif
	return 1;
}

long
CCLWin::updateAllClients()
{
	int retval, fd;
	char fname[256], buf[256];

	retval = getUpdateFileName(fname, &fd);
	if (retval) {
		snprintf(buf, 256, "%s", fname);
		//initiate update

	}
}

long
CCLWin::setAllClientPass()
{
	FXString clipass;
	int      size;
	FXInputDialog *dlg = new FXInputDialog(this,_("Admin Password"),
	                                       _("New Client Admin Password: "),
	                                       NULL,INPUTDIALOG_STRING);
	clipass = FXString((FXchar*)CCL_data_get_blob(CCL_DATA_NONE, 0, "adminpassword",&size));
	if (clipass.length())
		dlg->setText(clipass);
	if (dlg->execute()) {
		clipass = dlg->getText();
		size = clipass.length();
		if (size) {
			int i, client;
			int allow = clsmenu_allowuserlogin_check->getCheck() & ALLOWUSERLOGIN;
			for (int i=0; (-1 != (client = CCL_client_get_nth(i))); i++)
				CCL_client_send_cmd(client,CS_SETADMINPASS, clipass.text(), clipass.length());
			CCL_data_set_blob(CCL_DATA_NONE, 0, "adminpassword", (void *)clipass.text(), size);
			FXMessageBox::information(this,MBOX_OK,_("Admin Password"),
			                          _("Password has been sent!"));
		} else
			FXMessageBox::information(this,MBOX_OK,_("Admin Password"),
			                          _("Password is invalid!"));
	}
	delete dlg;
	return 1;
}

bool
isPrintServerClient(int client)
{
	return (!strncasecmp(CCL_client_name_get(client), "PrintServer", 11));
}

long
CCLWin::onCommand(FXObject*,FXSelector sel,void*)
{
	int current = clientslist->getCurrentItem();
	if (-1 == current)    return 1;
	long client = (long) (clientslist->getItemData(current));
	int status = CCL_client_status_get(client);
	time_t stime;
	time_t etime;
	time_t usedtime;
	FXuint nusedtime;

	if (client == curUpdClient) {
		FXMessageBox::information(getRoot(),MBOX_OK,_("Update in Progress"),
		                          _("Current client is being updated."));
	} else if (client) {
		switch (FXSELID(sel)) {
		case ID_START:
			if (isPrintServerClient(client)) break;
			CCL_client_start(client);
			CCL_client_flags_toggle(client,USERSTOP,FALSE);
			CCL_client_send_cmd(client,CS_START,NULL,0);
			CCL_client_send_cmd(client,CS_UNLOCKSCREEN,NULL,0);
			if (CCL_client_status_get(client)!=CCL_PAUSED)
				job_pages[client % MAX_CLIENTS] = 0;
			break;
		case ID_PAUSE:
			if (isPrintServerClient(client)) break;
			CCL_client_pause(client);
			CCL_client_send_cmd(client,CS_PAUSE,NULL,0);
			CCL_client_send_cmd(client,CS_LOCKSCREEN,NULL,0);
			break;
		case ID_STOP:
			if (isPrintServerClient(client)) break;
			if (status == CCL_ACTIVE) {
				int member = CCL_client_member_get(client);
				CCL_client_stop(client);
				if (member != 0) { //for a member/ticket session
					int credit = CCL_member_credit_get(member);
					int owed = CCL_client_owed_terminal(client);
					CCL_member_credit_set(member, credit - owed);
					CCL_log_session(client,owed,PAID, mainwin->getEmployeeID());
				} else { //a postpaid session (not member / ticket)
					CCL_log_session(client,CCL_client_owed_terminal(client),NOTPAID, getEmployeeID());
					blockClient(client);
					notpaidframe->readNotPaid();
				}
			} else //status is INACTIVE
				CCL_client_flags_toggle(client,USERSTOP,FALSE);
			CCL_client_send_cmd(client,CS_STOP,NULL,0);
			CCL_client_send_cmd(client,CS_LOCKSCREEN,NULL,0);
			break;
		case ID_UNSTOP:
			if (isPrintServerClient(client)) break;
			stime = CCL_client_stime_get(client);
			etime = CCL_client_etime_get(client);
			usedtime = CCL_client_time_used(client);
			nusedtime = CCL_htonl(usedtime);
			if (CCL_INACTIVE == CCL_client_status_get(client)
			    && cashingframe->getSession() == CCL_log_session_find(client,
			            stime,etime))
				cashingframe->setSession(0);
			CCL_client_flags_toggle(client,USERSTOP,FALSE);
			CCL_client_unstop(client);
			CCL_client_unpause(client);
			notpaidframe->readNotPaid();
			CCL_client_send_cmd(client,CS_RESUME,NULL,0);
			CCL_client_send_cmd(client,CS_SETTIME,&nusedtime,sizeof(nusedtime));
			CCL_client_send_cmd(client,CS_UNLOCKSCREEN,NULL,0);
			break;
		case ID_SETMEMBER:
			if (isPrintServerClient(client)) break;
			setClientMember(client);
			break;
		case ID_MONITOROFF:
			CCL_client_send_cmd(client,CS_MONITOROFF,NULL,0);
			break;
		case ID_REBOOT:
			CCL_client_send_cmd(client,CS_REBOOT,NULL,0);
			break;
		case ID_POWEROFF:
			CCL_client_send_cmd(client,CS_SHUTDOWN,NULL,0);
			break;
		case ID_UPDATECLIENT:
			updateClient(client);
#ifdef DEBUG_UPD
			printf("End of Client Update Command\n");
#endif
			break;
		case ID_ALLSETMEMBER:
			//if (confirmAll())
			//  setAllClientMember(client);
			break;
		case ID_ALLMONITOROFF:
			if (confirmAll())
				send_cmd_to_all(CS_MONITOROFF,NULL,0);
			break;
		case ID_ALLREBOOT:
			if (confirmAll())
				send_cmd_to_all(CS_REBOOT,NULL,0);
			break;
		case ID_ALLPOWEROFF:
			if (confirmAll())
				send_cmd_to_all(CS_SHUTDOWN,NULL,0);
			break;
		case ID_ALLUPDATECLIENT:
			updateAllClients();
			break;
		case ID_ALLADMINPASS:
			if (e_inf.empID == 1)
				setAllClientPass();
			break;
		case ID_QUITCLIENT:
			break;
		case ID_60MIN:
		case ID_30MIN:
		case ID_20MIN:
		case ID_10MIN: {
			int     rem_time = CCL_client_timeout_get(client) / 60;
			int     sesstime;
			char    inqstr[100];
			int     was_inactive = 0;

			if (isPrintServerClient(client)) break;
			switch (FXSELID(sel)) {
			case ID_10MIN:
				sesstime = 10;
				break;
			case ID_20MIN:
				sesstime = 20;
				break;
			case ID_30MIN:
				sesstime = 30;
				break;
			case ID_60MIN:
				sesstime = 60;
				break;
			}
			/*confirm*/
			sprintf(inqstr, "Add %d mins to %s?", sesstime, CCL_client_name_get(client));
			if (FXMessageBox::question(this,MBOX_YES_NO,_("Adjust Timeout"),
			                           _(inqstr)) == MBOX_CLICKED_YES) {
				if (status == CCL_INACTIVE) { // start session
					onCommand(NULL, ID_START, NULL);
					was_inactive = 1;
					rem_time = 0;
				}
				int    timeout = (rem_time + sesstime) * 60;
				if (was_inactive) { //subtract elapsed time from client
					timeout -=  (CCL_client_time_used(client) / 60);
					if (timeout < 0) timeout = 0;
				}
				FXuint ntimeout = CCL_htonl(timeout);
				CCL_client_timeout_set(client,timeout);
				CCL_client_send_cmd(client,CS_SETTIMEOUT,&ntimeout,sizeof(ntimeout));
				updateClientIcon(client);
			}
		}
		break;
		case ID_CLOSE:
			//printf("User is exiting now.\n");
			break;
		}
		updateClientIcon(client);
	}
	productsframe->updateClientProducts(client);
	return 1;
}

long
CCLWin::onAbout(FXObject*, FXSelector, void*)
{
	FXMessageBox
	about(this,_("About"),getApp()->getAppName() + " " + "VERSION" + "\n" +
	      _("Modified by Bernard Owuor, Unwire Technologies (owuor@unwiretechnologies.net)"),
	      bigicon,MBOX_OK|DECOR_TITLE|DECOR_BORDER);
	about.execute();
	return 1;
}

long
CCLWin::onSwap(FXObject*,FXSelector,void*)
{
	int current = clientslist->getCurrentItem();
	if (-1 == current)
		return 1;
	if (-1 != toSwap) {
		playbutton->enable();
		pausebutton->enable();
		stopbutton->enable();
		unstopbutton->enable();
		timebutton->enable();
		newbutton->enable();
		delbutton->enable();
		toSwap = -1;
	} else {
		playbutton->disable();
		pausebutton->disable();
		stopbutton->disable();
		unstopbutton->disable();
		timebutton->disable();
		newbutton->disable();
		delbutton->disable();
		toSwap = (long) (clientslist->getItemData(current));
	}
	return 1;
}

long
CCLWin::onTime(FXObject*,FXSelector,void*)
{
	int current = clientslist->getCurrentItem();
	if (-1 == current)
		return 1;
	long client = (long) (clientslist->getItemData(current));
	int time = CCL_client_timeout_get(client) / 60;
	FXInputDialog *dlg = new FXInputDialog(this,_("Time"),_("Minutes:"),
	                                       NULL,INPUTDIALOG_INTEGER);
	dlg->setLimits(1,0);
	dlg->setText(FXStringVal(time));
	if (client && dlg->execute()) {
		time = FXIntVal(dlg->getText());
		int timeout = time * 60;
		FXuint ntimeout = CCL_htonl(timeout);

		CCL_client_timeout_set(client,timeout);
		CCL_client_send_cmd(client,CS_SETTIMEOUT,&ntimeout,sizeof(ntimeout));
		updateClientIcon(client);
	}
	delete dlg;
	return 1;
}

void
CCLWin::toggleClientSetting(int client, int flag)
{
	int settings = CCL_data_get_int(CCL_DATA_CLIENT, client, "client_settings", -1);
#ifdef DEBUG_SETTINGS
	int preset = settings;
#endif
	if (settings != -1)
		settings =  (settings & flag) ? settings &= ~flag: settings |= flag;
	else
		settings = flag;
	CCL_data_set_int(CCL_DATA_CLIENT, client, "client_settings", settings);
#ifdef DEBUG_SETTINGS
	printf("CCLWin::toggleClientSetting() Client[%02d] flag [%08X] Setting: [%08X]->[%08X]\n",
	       client, flag, preset, settings);
#endif
	CCL_client_flags_toggle(client, flag, (settings & flag));
}

long
CCLWin::onAllowUserLogin(FXObject*,FXSelector,void*)
{
	int current = clientslist->getCurrentItem();
	if (-1 == current)
		return 1;
	long client = (long) (clientslist->getItemData(current));
	toggleClientSetting(client, ALLOWUSERLOGIN);
	int nallow = CCL_htonl(CCL_client_flags_get(client) & ALLOWUSERLOGIN);
	CCL_client_send_cmd(client,CS_ALLOWUSERLOGIN,&nallow,sizeof(nallow));

	return 1;
}

long
CCLWin::onAllAllowUserLogin(FXObject*,FXSelector,void*)
{
	int i, client;
	int allow = clsmenu_allowuserlogin_check->getCheck() & ALLOWUSERLOGIN;
	for (int i=0; (-1 != (client = CCL_client_get_nth(i))); i++) {
		CCL_client_flags_toggle(client,ALLOWUSERLOGIN, allow);
		CCL_client_send_cmd(client,CS_ALLOWUSERLOGIN,&allow,sizeof(allow));
	}
	return 1;
}

long
CCLWin::onEnableAssist(FXObject*,FXSelector,void*)
{
	int current = clientslist->getCurrentItem();

	if (-1 == current)
		return 1;
	long client = (long) (clientslist->getItemData(current));

	//  CCL_client_flags_toggle(client,ENABLEASSIST, !(CCL_client_flags_get(client) & ENABLEASSIST));
	toggleClientSetting(client, ENABLEASSIST);
	int assist = CCL_htonl(CCL_client_flags_get(client) & ENABLEASSIST);
	CCL_client_send_cmd(client,CS_ENABLEASSIST ,&assist,sizeof(assist));

	return 1;
}

bool
CCLWin::confirmAll()
{
	bool retval = FALSE;
	FXDialogBox dlg(this, _("Confirm Action"));
	FXVerticalFrame *vframe =
	    new FXVerticalFrame(&dlg,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,0);
	//confirm all first
	FXHorizontalFrame *hframe1 =
	    new FXHorizontalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y);
	FXLabel lbl(hframe1, _("Apply this action to all stations?"));
	FXHorizontalFrame *hframe2 =
	    new FXHorizontalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y);
	new FXButton(hframe2,_("Apply"),dbIcon2,&dlg,FXDialogBox::ID_ACCEPT,
	             BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
	new FXButton(hframe2,_("Cancel"),dbIcon2,&dlg,FXDialogBox::ID_CANCEL,
	             BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
	if (dlg.execute()) {
#ifdef DEBUG
		printf("confirmAll(): Applied.\n");
#endif
		retval = TRUE;
	} else {
#ifdef DEBUG
		printf("confirmAll(): Not Applied.\n");
#endif
		retval = FALSE;
	}
	return retval;
}

long
CCLWin::onEnableAllAssist(FXObject*,FXSelector,void*)
{
	int i, assistval, sndassist;
	long client;

	int current = clientslist->getCurrentItem();
	client = (long) (clientslist->getItemData(current));
	assistval = !(CCL_client_flags_get(client) & ENABLEASSIST);
	sndassist = CCL_htonl(CCL_client_flags_get(client) & ENABLEASSIST);
	if (-1 == client)    return 1;
	if (!confirmAll())    return 1;
	for (i=0; (-1 != (client=CCL_client_get_nth(i)));  i++) {
		CCL_client_flags_toggle(client,ENABLEASSIST,assistval);
		CCL_client_send_cmd(client,CS_ENABLEASSIST ,&sndassist,sizeof(sndassist));
	}

	return 1;
}


long
CCLWin::onAllowMemberLogin(FXObject*,FXSelector,void*)
{
	int current = clientslist->getCurrentItem();

	if (-1 == current)
		return 1;
	long client = (long) (clientslist->getItemData(current));
	toggleClientSetting(client, ALLOWMEMBERLOGIN);
	int allow = CCL_htonl(CCL_client_flags_get(client) & ALLOWMEMBERLOGIN);
	CCL_client_send_cmd(client,CS_ALLOWMEMBERLOGIN,&allow,sizeof(allow));

	return 1;
}

long
CCLWin::onAllAllowMemberLogin(FXObject*,FXSelector,void*)
{
	int i, client;

	int allow = clsmenu_allowmemberlogin_check->getCheck() & ALLOWMEMBERLOGIN;
	for (int i=0; (-1 != (client = CCL_client_get_nth(i))); i++) {
		CCL_client_flags_toggle(client,ALLOWMEMBERLOGIN, allow);
		CCL_client_send_cmd(client,CS_ALLOWMEMBERLOGIN,&allow,sizeof(allow));
	}
	return 1;
}

long
CCLWin::onAllowTicketLogin(FXObject*,FXSelector,void*)
{
	int current = clientslist->getCurrentItem();

	if (-1 == current)
		return 1;
	long client = (long) (clientslist->getItemData(current));

	//  CCL_client_flags_toggle(client,ALLOWTICKETLOGIN, !(CCL_client_flags_get(client) & ALLOWTICKETLOGIN));
	toggleClientSetting(client, ALLOWTICKETLOGIN);
	int allow = CCL_htonl(CCL_client_flags_get(client) & ALLOWTICKETLOGIN);
	CCL_client_send_cmd(client,CS_ALLOWTICKETLOGIN,&allow,sizeof(allow));
#ifdef DEBUG_TICKET
	printf("CCLWin::onAllowTicketLogin(): Sent allow ticket login message\n");
#endif

	return 1;
}


long
CCLWin::onAllAllowTicketLogin(FXObject*,FXSelector,void*)
{
	int i, client;

	int allow = clsmenu_allowticketlogin_check->getCheck() & ALLOWTICKETLOGIN;
	for (int i=0; (-1 != (client = CCL_client_get_nth(i))); i++) {
		CCL_client_flags_toggle(client,ALLOWTICKETLOGIN, allow);
		CCL_client_send_cmd(client,CS_ALLOWTICKETLOGIN,&allow,sizeof(allow));
	}
	return 1;
}

long
CCLWin::onNewClient(FXObject*,FXSelector,void*)
{
	FXString result;

	if (FXInputDialog::getString(result,this,_("Add new Client"),
	                             _("New Client Name:")) && result.length()) {
		char *name = fxstrdup(result.text());
		int id = CCL_client_new(name);

		appendClient(id);
		FXFREE(&name);
	}
	return 1;
}

long
CCLWin::onDelClient(FXObject*,FXSelector,void*)
{
	int current = clientslist->getCurrentItem();
	FXDialogBox dlg(this, _("Delete Client?"));
	FXVerticalFrame *vframe =
	    new FXVerticalFrame(&dlg,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,0);

	if (-1 == current)
		return 1;

	long client = (long) (clientslist->getItemData(current));
	//confirm deletion first
	FXHorizontalFrame *hframe3 =
	    new FXHorizontalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y);
	new FXButton(hframe3,_("Cancel"),dbIcon2,&dlg,FXDialogBox::ID_CANCEL,
	             BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_LEFT);
	new FXButton(hframe3,_("Delete"),dbIcon2,&dlg,FXDialogBox::ID_ACCEPT,
	             BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_RIGHT);
	if (dlg.execute()) {
		//FXFoldingItem *item;

		clientslist->removeItem(current);
		//item = clientslist2->findItemByData((void *)client);
		//clientslist2->removeItem(item);
		if (client) {
			CCL_client_flags_toggle(client, CLIENT_DELETED, TRUE);
			deleteClient(client);
		}
		productsframe->updateClientProducts(client);
	}
	return 1;
}

long
CCLWin::onEmpLogin(FXObject* fxo,FXSelector fxs,void* v)
{
	if (loginstat == 0) {
		employeeLogin(fxo, fxs, v);
	} else {
		employeeLogout(fxo, fxs, v);
	}
	return 1;
}


long
CCLWin::onListShutter(FXObject* fxo,FXSelector fxs,void* ptr)
{
	if (ListType == LV_ICONLIST) {
		ListType = LV_DETAILLIST;
#ifdef DEBUG_GUI
		printf("Toggle ListType = Details\n");
#endif
	} else {
		ListType = LV_ICONLIST;
#ifdef DEBUG_GUI
		printf("Toggle ListType = Icons\n");
#endif
	}
	return 1;
}

long
CCLWin::onClientSelected(FXObject*,FXSelector,void* ptr)
{
	long idx = (long) ptr;
	long client = (long) (clientslist->getItemData(idx));

	if (-1 != toSwap) {
		int oldidx = clientslist->findItem(CCL_client_name_get(toSwap));

		CCL_client_swap(toSwap,client);
		updateClientIcon(client);
		updateClientIcon(toSwap);
		updateClientStatus(client);
		updateClientStatus(toSwap);
		onSwap(NULL,0,NULL);	// Reenable buttons
	}

	idx = clientslist->getCurrentItem();
	client = (long) (clientslist->getItemData(idx));
	updateInfo(client);
	productsframe->updateClientProducts(client);
	showProducts();
	return 1;
}

long
CCLWin::onClientSelected2(FXObject*,FXSelector,void* ptr)
{
	FXFoldingItem *item = (FXFoldingItem *) ptr;
	long client = (long) (clientslist2->getItemData(item));

	if (-1 != toSwap) {
		FXFoldingItem *oldidx = clientslist2->findItem(CCL_client_name_get(toSwap));
		CCL_client_swap(toSwap,client);
		updateClientIcon(client);
		updateClientIcon(toSwap);
		updateClientStatus(client);
		updateClientStatus(toSwap);
		onSwap(NULL,0,NULL);	// Reenable buttons
	}
	item = clientslist2->getCurrentItem();
	client = (long) (clientslist2->getItemData(item));
	updateInfo(client);
	productsframe->updateClientProducts(client);
	showProducts();
	return 1;
}

long
CCLWin::onShowClientMenu(FXObject*,FXSelector,void* ptr)
{
	FXEvent *event = (FXEvent *) ptr;
	int idx = clientslist->getItemAt(event->click_x,event->click_y);
	if (-1 != idx) { /* menu for client x */
		long client = (long) clientslist->getItemData(idx);
		clmenu_caption->setText(CCL_client_name_get(client));
		int settings = CCL_data_get_int(CCL_DATA_CLIENT, client, "client_settings", -1);

		if (settings != -1) {

			if (settings & ALLOWUSERLOGIN)
//      if (CCL_client_flags_get(client) & ALLOWUSERLOGIN)
				clmenu_allowuserlogin_check->setCheck(TRUE);
			else
				clmenu_allowuserlogin_check->setCheck(FALSE);
		}
		if (CCL_client_flags_get(client) & ENABLEASSIST)
			clmenu_enableassist_check->setCheck(TRUE);
		else
			clmenu_enableassist_check->setCheck(FALSE);
		if (CCL_client_flags_get(client) & ALLOWMEMBERLOGIN)
			clmenu_allowmemberlogin_check->setCheck(TRUE);
		else
			clmenu_allowmemberlogin_check->setCheck(FALSE);
		if (CCL_client_flags_get(client) & ALLOWTICKETLOGIN)
			clmenu_allowticketlogin_check->setCheck(TRUE);
		else
			clmenu_allowticketlogin_check->setCheck(FALSE);

		clientslist->setCurrentItem(idx,TRUE);
		clientslist->selectItem(idx,TRUE);
		clmenu->popup(NULL,event->root_x,event->root_y);
	} else { /*Now menu for all clients*/
		clsmenu->popup(NULL,event->root_x,event->root_y);
	}
	return 0;
}

long
CCLWin::onShowClientMenu2(FXObject*,FXSelector,void* ptr)
{
	FXEvent *event = (FXEvent *) ptr;
	FXFoldingItem *item = clientslist2->getItemAt(event->click_x,event->click_y);
	if (item != NULL) { /* menu for client x */
		long client = (long) clientslist2->getItemData(item);
		clmenu_caption->setText(CCL_client_name_get(client));
		if (CCL_client_flags_get(client) & ALLOWUSERLOGIN)
			clmenu_allowuserlogin_check->setCheck(TRUE);
		else
			clmenu_allowuserlogin_check->setCheck(FALSE);
		if (CCL_client_flags_get(client) & ENABLEASSIST)
			clmenu_enableassist_check->setCheck(TRUE);
		else
			clmenu_enableassist_check->setCheck(FALSE);
		if (CCL_client_flags_get(client) & ALLOWMEMBERLOGIN)
			clmenu_allowmemberlogin_check->setCheck(TRUE);
		else
			clmenu_allowmemberlogin_check->setCheck(FALSE);

		if (CCL_client_flags_get(client) & ALLOWTICKETLOGIN)
			clmenu_allowticketlogin_check->setCheck(TRUE);
		else
			clmenu_allowticketlogin_check->setCheck(FALSE);

		clientslist2->setCurrentItem(item,TRUE);
		clientslist2->selectItem(item,TRUE);
		clmenu->popup(NULL,event->root_x,event->root_y);
	} else { /*Now menu for all clients*/
		clsmenu->popup(NULL,event->root_x,event->root_y);
	}

	return 0;
}

long
CCLWin::onTimerTick(FXObject*,FXSelector,void*)
{
	static int counter = 0;

	if (ListType == LV_ICONLIST) {
		if (long num = clientslist->getNumItems()) {
			long client, idx;

			for (idx = 0; idx < num; idx++) {
				client = (long) (clientslist->getItemData(idx));
				updateClientIcon(client);
				//obtain summary at the same time
			}
			if (!(++counter % 5) )
				updateSummaryInfo();
		}
		getApp()->addTimeout(this,ID_TIMERTICK, 1000);
		updateInfo(getCurrentClient());
	} else { /* List View is LV_DETAILLIST */
		/* update every 5 seconds */
#ifdef DEBUG_GUI
		printf("LV_DETAILS\n");
#endif
		if (long num = clientslist2->getNumItems()) {
			long idx, client;
			char buf[128];

			//clientslist2->clearItems();
			for (FXFoldingItem * i = clientslist2->getFirstItem();
			     NULL != i; i = i->getNext()) {
				client = (long) (clientslist2->getItemData(i));
				getClientInfoStr(client, buf, 128);
				//clientslist2->prependItem(NULL, buf, NULL, NULL, (void *)client);
				i->setText(_(buf));
				clientslist2->updateItem(i);
			}
		}
		updateSummaryInfo();
		getApp()->addTimeout(this,ID_TIMERTICK, 5000);
	}

	return 1;
}

void
CCLWin::updateSummaryInfo()
{


}

void
CCLWin::updateInfo(int client)
{
	time_t time = 0;
	if (-2 != CCL_client_time_left(client))
		time = CCL_client_time_left(client);
	else
		time = CCL_client_time_used(client);
	int h = time / 3600,m = (time % 3600) / 60,s = (time % 3600) % 60;
	char buf[32];
	if (time > 0)
		snprintf(buf,32,"%.2d:%.2d:%.2d",h,m,s);
	else
		snprintf(buf,32,"--:--");
	i_time->setText(buf);
	int owedp = CCL_client_owed_products(client);
	snprintf(buf,32,"%.2f",owedp / 100.0);
	i_products->setText(buf);
	int owedt = CCL_client_owed_terminal(client);
	snprintf(buf,32,"%.2f",owedt / 100.0);
	i_terminal->setText(buf);
	int owed = owedt + owedp;
	snprintf(buf,32,"%.2f",owed / 100.0);
	i_owes->setText(buf);
}

void
CCLWin::getClientInfoStr(int client, char *clbuf, int len)
{
	time_t ctime = 0;
	if (-2 != CCL_client_time_left(client))
		ctime = CCL_client_time_left(client);
	else
		ctime = CCL_client_time_used(client);
	int h = ctime / 3600,m = (ctime % 3600) / 60;
	char startt[32], endt[32], clname[128], ststr[32];
	char sesslen[10], sesscost[20], prn[10];
	//char clistr[5][16] = {"FREE", "IN USE"), _("PAUSE"), _("STOPPED")};
	char clistr[5][16] = {"FREE", "IN USE", "PAUSE", "STOPPED"};

	//client name
	snprintf(clname, 64, "%s", CCL_client_name_get(client));
	//status
	if (CCL_client_flags_get(client) & CLIENT_CONNECTED)
		snprintf(ststr, 32, "O: %s", clistr[CCL_client_status_get(client)]);
	else
		snprintf(ststr, 32, "X: %s", clistr[CCL_client_status_get(client)]);
	//start time
	time_t etime = CCL_client_etime_get(client);
	if (!etime) etime = time(NULL);
	strftime(endt,32,"%H:%M",localtime(&etime));
	//end time
	time_t stime = CCL_client_stime_get(client);
	if (!stime) stime = time(NULL);
	strftime(startt,32,"%d/%m  %H:%M",localtime(&stime));
	//time length
	if (time > 0)
		snprintf(sesslen,10,"%.2dmins",h*60+m);
	else
		snprintf(sesslen,10,"-- mins");
	//cost of session
	int owedt = CCL_client_owed_terminal(client);
	snprintf(sesscost,20,"%.2f",owedt / 100.0);
	//products and/or printouts
	int owedp = CCL_client_owed_products(client);
	snprintf(prn,10,"%.2f",owedp / 100.0);
	//put it together
	snprintf(clbuf, 128, "%s\t%s\t%s\t%s\t%s\t%s\t%s",
	         clname, ststr, startt, endt, sesslen, sesscost, prn);
#ifdef DEBUG_GUI
	printf("%s\n", clbuf);
#endif
}

void
CCLWin::updateClientIcon(int client)
{
	int idx = clientslist->findItem(CCL_client_name_get(client));

	if (-1 == idx) return;
	if (CCL_client_time_left(client) == 0
	    && CCL_ACTIVE == CCL_client_status_get(client))
		clientslist->setItemBigIcon(idx,bpcicons[3]);
	else if (CCL_client_flags_get(client) & USERSTOP)
		clientslist->setItemBigIcon(idx,bpcicons[3]);
	else if (client == curUpdClient)
		clientslist->setItemBigIcon(idx,bpcicons[4]);
	else
		clientslist->setItemBigIcon(idx,bpcicons[CCL_client_status_get(client)]);
}

long
CCLWin::onProductAdd(FXObject*,FXSelector,void* ptr)
{
	FXFoldingItem *child = (FXFoldingItem *) ptr;
	FXFoldingItem *prnt = child->getParent();
	long idx = clientslist->getCurrentItem();

	if (!prnt || -1 == idx)
		return 0;
	long client = (long) (clientslist->getItemData(idx));
	int amount = 0;
	if (FXInputDialog::getInteger(amount,this,_("Add Products"),
	                              _("Enter the quantity:")) && amount >= 1) {
		long pid = (long) child->getData();
		CCL_client_product_add(client,pid,amount);
		productsframe->updateClientProducts(client);
	}
	return 1;
}

long
CCLWin::onProductRemove(FXObject*,FXSelector,void* ptr)
{
	FXFoldingItem *item = (FXFoldingItem *) ptr;
	long idx = clientslist->getCurrentItem();

	if (-1 == idx) return 0;
	long client = (long) (clientslist->getItemData(idx));
	int amount = 0;
	if (FXInputDialog::getInteger(amount,this,_("Remove Products"),
	                              _("Quantity:")) && amount >= 1) {
		long pid = (long) item->getData();
		CCL_client_product_sub(client,pid,amount);
		productsframe->updateClientProducts(client);
	}
	return 1;
}

void
CCLWin::setEmployeeID(int id)
{
	curempid = id;
}

int
CCLWin::getEmployeeID()
{
	return e_inf.empID; //curempid;
}


/*
  Log in dialog for employees
  Returns Values:
  Repeats until correctly logged in or cancelled
  True  when the username / password combination is correct
  False  when the user hits cancel
*/
FXbool
CCLWin::employeeLogin(FXObject*,FXSelector,void*)
{
	FXDialogBox dialog(this,_("Staff Login"));
	FXLabel lbl1(&dialog,_("User Name:"));
	FXTextField usrname(&dialog,20,NULL,0,TEXTFIELD_NORMAL |FRAME_SUNKEN);
	FXLabel lbl2(&dialog,_("Password:"));
	FXTextField passwd(&dialog,20,NULL,0,TEXTFIELD_PASSWD|FRAME_SUNKEN);
	FXVerticalFrame *vframe =
	    new FXVerticalFrame(&dialog,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,0);
	FXHorizontalFrame *hframe =
	    new FXHorizontalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y);
	FXButton cancelbtn(hframe,_("  Quit  "),dbIcon1,&dialog,FXDialogBox::ID_CANCEL,
	                   BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,
	                   0,0,100,20);
	FXButton okbtn(hframe,_("   Ok   "),dbIcon1,&dialog,FXDialogBox::ID_ACCEPT,
	               BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_RIGHT,
	               0,0,100,20);
	FXbool retval;
	okbtn.setWidth(100);
	cancelbtn.setWidth(100);

	retval = FALSE;
	//usrname.setFocus();
	do {
		if (dialog.execute(PLACEMENT_OWNER)) {
			FXButton *btn;
			char buf[64];

			if (authemp((FXuchar *)usrname.getText().text(),
			            (FXuchar *)passwd.getText().text())) {
				FXMessageBox::information(getRoot(),MBOX_OK,_("Welcome"),
				                          _("Welcome to Mkahawa Cyber Manager"));
				retval = TRUE;
				loginstat = 1;
				setPerms(e_inf.lvl);
				btn = clientshutter->getButton();
				sprintf(buf, "Client - %s", CCL_employee_name_get(e_inf.empID));
				btn->setText(buf);
			} else {
				FXMessageBox::information(getRoot(),MBOX_OK,_("Login Failure"),

				                          _("Invalid Username or Password"));
				retval = TRUE;
				loginstat = 0;
				setPerms(0);
			}
		} else
			break;
	} while (!retval);
	return retval;
}

void
CCLWin::employeeLogout(FXObject*,FXSelector,void*)
{
	if (FXMessageBox::question(this,MBOX_YES_NO,_("Logout"),
	                           _("Do you want to log out?")) == MBOX_CLICKED_YES) {
		loginstat = 0;
		e_inf.empID = 0;
		e_inf.lvl = 0;
		setPerms(e_inf.lvl);
	}
	return;
}

void
CCLWin::setPerms(unsigned long perm)
{
	productsframe->setPerms(perm);
	cashingframe->setPerms(perm);
	tarifframe->setPerms(perm);
	logframe->setPerms(perm);
	ticketsframe->setPerms(perm);

	if(!(perm & PERMTARIFEDIT)) tarifframe->disable();
	membersframe->setPerms(perm);
	if(!(perm & PERMMBREDIT)) membersframe->disable();
	employeesframe->setPerms(perm);
	if (!(perm & PERMEMPEDIT)) employeesframe->disable();
#ifdef DEBUG
	printf("Perms set to %08X\n", perm);
#endif
}

long
CCLWin::onMsgClient(FXObject*, FXSelector, void*)
{
	char     *cmsg;
	FXString  result;
	long       current = clientslist->getCurrentItem();

	if (-1 == current) return 1;
	long client = (long)(clientslist->getItemData(current));
	if (FXInputDialog::getString(result,this,_("Message to Client"),
	                             _("Message:")) && result.length()) {
		char *cmsg = fxstrdup(result.text());
		CCL_client_send_cmd(client, CS_DISPLAYMESSAGE, cmsg, strlen(cmsg));
		FXFREE(&cmsg);
	}
	return 1;
}

long
CCLWin::onCyberSet(FXObject*, FXSelector, void*)
{
	SettingsBox stb(this);
	if (stb.execute()) {
		applySettings(0);
	}
	return 1;
}


long
CCLWin::onMsgServer(FXObject*, FXSelector, void*)
{
	char     *cmsg;
	FXString  result;
	long      current = clientslist->getCurrentItem();

	if (-1 == current) return 1;
	long client = (long)(clientslist->getItemData(current));

	if (FXInputDialog::getString(result,this,_("Message to Client"),
	                             _("Message:")) && result.length()) {
		char *cmsg = fxstrdup(result.text());
		CCL_client_send_cmd(client, CS_CHATSERVER, cmsg, strlen(cmsg));
		FXFREE(&cmsg);
	}
	return 1;
}

FXbool
CCLWin::clientHelpIsUp(int client)
{
	/*if (clHelpStatus[client] > 0 )
	return TRUE;
	*/
	return FALSE;
}


long
CCLWin::onAlertClient(FXObject*, FXSelector, void*)
{
	char      cmsg[256];
	FXString  result;
	int       current = clientslist->getCurrentItem();

	if (-1 == current) return 1;
	long client = (long)(clientslist->getItemData(current));
	if (FXInputDialog::getString(result,this,_("Message to Client"),
	                             _("Message:")) && result.length()) {
		strncpy(cmsg, result.text(), 256);
		CCL_client_send_cmd(client, CS_ALERTCLIENT, cmsg, strlen(cmsg));
	}
	return 1;
}

long
CCLWin::sendUpdateChunk(int client, long pos)
{
	struct CHUNK chunk;
	int retval, chunksize;
	char *msgstr;

	if (pos >= (upinfo.fsize-sizeof(UpdateInfo))) { //complete update
		CCL_client_send_cmd(client, CS_UPDATEEND, NULL, 0);
#ifdef DEBUG_UPD
		printf("sendUpdateChunk(): Server Ending: Sent [%d bytes] Offset[%d]\n",
		       upinfo.fsize, 0);
#endif
		//prgrs->setProgress(0);
		//prgrs->hide();
		fclose(upfp);
		return 0;
	}
	//  fseek(
	retval = fread(chunk.buf, 1, MAXCHUNKSIZE, upfp);
	if (!retval) {
		msgstr = _("Unable to read update file\n");
#ifdef DEBUG_UPD
		printf("sendUpdateChunk(): [retval = %d] %s\n", retval, msgstr);
#endif
		return 0;
	}
	//prgrs->setProgress(pos);
	chunk.blen = retval;
	chunk.pos = pos;
	chunksize = sizeof(chunk.blen) + sizeof(chunk.pos) + chunk.blen;
	CCL_client_send_cmd(client, CS_UPDATEDATA, (char *)&chunk, chunksize);
#ifdef DEBUG_UPD
	printf("sendUpdateChunk(): Sent [%d bytes] Offset[%d]\n", retval, pos);
#endif
	return 1;
}


long
sendUpdateChunks(int client)
{
	struct CHUNK chunk;
	int retval, chunksize, pos;
	char *msgstr;

	for (pos=0; pos < mainwin->upinfo.fsize; pos += MAXCHUNKSIZE) {
		retval = fread(chunk.buf, 1, MAXCHUNKSIZE, mainwin->upfp);
		if (!retval) {
			msgstr = _("Unable to read update file\n");
#ifdef DEBUG_UPD
			printf("sendUpdateChunks(): [retval = %d] %s\n", retval, msgstr);
#endif
			return 0;
		}
		chunk.blen = retval;
		chunk.pos = pos;
		chunksize = sizeof(chunk.blen) + sizeof(chunk.pos) + chunk.blen;
		CCL_client_send_cmd(client, CS_UPDATEDATA, (char *)&chunk, chunksize);
#ifdef DEBUG_UPD
		printf("sendUpdateChunks(): [%d] Sent [%d bytes] Offset[%d]\n", client, retval, pos);
#endif
	}
	CCL_client_send_cmd(client, CS_UPDATEEND, NULL, 0);
#ifdef DEBUG_UPD
	printf("sendUpdateChunks(): Server Ending: Sent [%d bytes] Offset[%d]\n",
	       mainwin->upinfo.fsize, pos);
#endif
	fclose(mainwin->upfp);
	return 1;
}


int
CCLWin::applySettings(unsigned long setting)
{
	if (cyber_settings)
		cyber_settings->loadSettings();
	else
		cyber_settings = new CyberSettings();

#ifdef DEBUG_SETTINGS
	printf("applySettings(): round_off = %u\n", (unsigned int)cyber_settings->round_off);
#endif
	CCL_set_settings( (void *) cyber_settings->round_off );
	int npoll_itvl = CCL_htonl(cyber_settings->poll_interval);
	send_cmd_to_all(CS_SETPOLLINTERVAL,(char *)&npoll_itvl, sizeof(npoll_itvl));

	return 0;
}
