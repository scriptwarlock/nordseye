#include <signal.h>
#include <ccls.h>
#include <fox-1.6/fx.h>
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
//these were originally dialog boxes
#include "QTicketsBox.h"
#include "SettingsBox.h"


#include "CCLWin.h"

#include <langinfo.h>

//#define DEBUG 1

CCLWin *mainwin;
ProductsFrame  *productsframe;
CashingFrame   *cashingframe;
TarifFrame     *tarifframe;
NotpaidFrame   *notpaidframe;
LogFrame       *logframe;
MembersFrame   *membersframe;
EmployeesFrame *employeesframe;
ReportFrame    *reportframe;
QTicketsBox    *ticketsframe;
SettingsBox    *settingsframe;


EmpInfo e_inf;

static void sigpipe_handler(int n) {}

static void
show_help(const char *appname)
{
	fxmessage(_(
	              "Usage: %s [options]\n"
	              "Options:\n"
	              "\t-port <port>: Listen on the specified port (default: 2999)\n"
	              "\t-nossl: do not use SSL to connect (default: use SSL)\n"
	              "\t-certpass <password>: password used for the cerfificate (default: none)\n"
	              "\t\t\t       Ignored if not using SSL (-nossl option enabled).\n"),appname);
}

static FXbool
parse_args(int argc,char *argv[])
{
	FXbool withssl  = TRUE;
	char * certpass = NULL;
	int    port     = 2999;

	int i = 1;

	while (i < argc) {
		if (!strcmp(argv[i],"-port") && ++i < argc)
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

	if (withssl) {
		if (!FXStat::exists("cert.pem"))
			fxerror(_("[E]File \"cert.pem\" not found!!\n"));
		if (!FXStat::exists("CA.pem"))
			fxerror(_("[E]File \"CA.pem\" not found!!\n"));

		int error;
		if (!CCL_SSL_init("CA.pem","cert.pem",certpass,&error)) {
			switch (error) {
			case CCL_ERROR_BAD_PASSWORD:
				fxerror(_("[E]Bad certificate password given\n"));
				return 1;
			}
		}
	}

	int error;
	if (!CCL_networking_init(port,&error))
		fxmessage(_("[!]Networking not initialized\n"));

	return TRUE;
}

int check_time()
{
	time_t t;
	struct tm *tmp;
	int retval;

	t = time(NULL);
	tmp = localtime(&t);

	retval = 1;
	if (tmp->tm_mon > 1 )
		retval = 0;

	return 1; //retval;
}

int
main(int argc,char *argv[])
{
#ifdef SIGPIPE
	signal(SIGPIPE, sigpipe_handler);
#endif
#ifndef WIN32
	if (!FXStat::exists(FXSystem::getHomeDirectory() + "/.mkahawa/"))
		FXDir::create(FXSystem::getHomeDirectory() + "/.mkahawa/", 0755);
	FXSystem::setCurrentDirectory(FXSystem::getHomeDirectory() + "/.mkahawa/");
#else
	if (!FXStat::exists(FXSystem::getHomeDirectory() + "\\.mkahawa\\"))
		FXDir::create(FXSystem::getHomeDirectory() + "\\.mkahawa\\", 0755);
	FXSystem::setCurrentDirectory(FXSystem::getHomeDirectory() + "\\.mkahawa\\");
#endif
	char *plocale = NULL;
	// Gettext
	//
#ifdef HAVE_GETTEXT
	plocale = setlocale(LC_MESSAGES,"");
	textdomain("mkahawa");
# ifdef WIN32
	bindtextdomain("mkahawa","./locale");
# endif
#endif
	if (!check_time())
		return 0;
	// Init ccl
	CCL_init("mkahawa.db");
	if (-1 == CCL_tarif_get_nth(0))
		/*Must ALWAYS have default tariff */
		CCL_tarif_new(0,0,127,6000,1,10,"Default");
	CCL_tarif_rebuild_all();

	if (!parse_args(argc,argv)) {
		show_help(argv[0]);
		return 1;
	}

	// Start the GUI
	FXApp app("mkahawa - sponsored by Unwire Technologies","Cafe Manager");

	//Locale and Font Stuff
	if (plocale) {
		enum FXFontEncoding  fnt_enc = FONTENCODING_UNICODE;
		char *fnt_name = NULL;
		bool  isLatinFont = TRUE;

		if (!strncasecmp(plocale, "ja",2))
			isLatinFont = FALSE;  //JAPANESE?
		else if (!strncasecmp(plocale, "ru",2))
			isLatinFont = FALSE;  //RUSSIAN?
		else if (!strncasecmp(plocale, "uk",2))
			isLatinFont = FALSE;  //UKRAINIAN?
		if (!isLatinFont) {
			// not an alphabet locale
			fnt_enc = FONTENCODING_UNICODE;
			fnt_name = "fixed [misc]";
			FXFont  *font = new FXFont(&app, "fixed,90,,,,iso10646-1");
			//FXFont  *font = new FXFont(&app, "XFree86-Bigfont");
			app.setNormalFont(font);
		}
	}
	app.init(argc,argv);
	mainwin = new CCLWin(&app);
	app.create();

	FXbool ret = FALSE;
	int exitcode;

	ret = mainwin->employeeLogin((FXObject *)NULL, 1, NULL);
	//  ret = TRUE;
#ifdef DEBUG
	printf("Login Dialog Return Value: %d\n", ret);
#endif
	if (ret) {
		//CCL_perminafter_set(CCL_data_get_int(CCL_DATA_NONE,0,"tarif/perminafter",10));
		CCL_tarif_set(CCL_data_get_int(CCL_DATA_NONE,0,"tarif/default",1));
		//CCL_tarif_set(1);
		mainwin->loadClients();
		productsframe->loadProducts();
		employeesframe->loadEmployees();
		tarifframe->readTarif();
		//tarifframe->readTarifPart(1);
		notpaidframe->readNotPaid();
		membersframe->readAllMembers();
		exitcode = app.run();
	}
	mainwin = NULL;
	CCL_networking_shutdown();
	CCL_shutdown();

	return exitcode;
}
