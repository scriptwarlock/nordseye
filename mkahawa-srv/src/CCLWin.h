#ifndef CCLWin_H
#define CCLWin_H

#include "update.h"

enum ListTypeEnum {  LV_ICONLIST, LV_DETAILLIST };

class CCLWin : public FXMainWindow
{
FXDECLARE(CCLWin)
friend void onEventCallback(int client,FXuint cmd,void *data,FXuint size);
protected:
  // App Icons
  FXIcon	*bigicon;
  FXIcon	*miniicon;
  FXSplitter	*vsplitter;
  FXShutter	*rshutter;
  // Clients area
  FXToolBar	*ctoolbar;
  FXIcon	*playicon;
  FXIcon	*pauseicon;
  FXIcon	*stopicon;
  FXIcon	*cancelicon;
  FXIcon	*unstopicon;
  FXIcon	*swapicon;
  FXIcon	*timeicon;
  FXIcon        *msgicon;
  FXIcon        *emplogicon;
  FXIcon        *delicon;
  FXIcon        *newicon;
  FXIcon        *min10icon;
  FXIcon        *min20icon;
  FXIcon        *min30icon;
  FXIcon        *min60icon;
  FXIcon        *csicon;

  FXButton	*playbutton;
  FXButton	*stopbutton;
  FXButton	*unstopbutton;
  FXButton	*cancelbutton;
  FXButton	*pausebutton;
  FXButton	*swapbutton;
  FXButton	*timebutton;
  FXButton	*newbutton;
  FXButton	*delbutton;
  FXButton	*passbutton;
  FXButton      *loginbutton;
  FXButton      *msgbutton;
  FXButton      *csbutton;
  FXButton      *min10btn;
  FXButton      *min20btn;
  FXButton      *min30btn;
  FXButton      *min60btn;

  FXIconList	*clientslist;
  FXFoldingList	*clientslist2;
  FXGIFIcon	*bpcicons[5];
  FXGIFIcon	*disconicon;
  // Client Info
  FXFont	*i_fonthandle;
  FX7Segment	*i_time;
  FX7Segment	*i_terminal;
  FX7Segment	*i_products;
  FX7Segment	*i_owes;
  // Client menu
  FXMenuPane	*clmenu;
  FXMenuPane	*clsmenu;
  FXMenuCaption	*clmenu_caption;
  FXMenuCaption	*clsmenu_caption;
  FXMenuCheck	*clmenu_allowuserlogin_check;
  FXMenuCheck	*clmenu_allowmemberlogin_check;
  FXMenuCheck	*clmenu_allowticketlogin_check;
  FXMenuCheck	*clmenu_enableassist_check;
  FXMenuCheck	*clsmenu_allowuserlogin_check;
  FXMenuCheck	*clsmenu_allowmemberlogin_check;
  FXMenuCheck	*clsmenu_allowticketlogin_check;
  FXMenuCheck	*clsmenu_enableassist_check;
  //progress
  //FXProgressDialog *prgrs;
  FXProgressBar *prgrs;
protected:
  int		 toSwap;    // Client selected to be swaped with another
  int            curempid;
  int            loginstat;
  enum ListTypeEnum  ListType;
public:
  UpdateInfo     upinfo;
  int            curUpdClient;
  int            updFileCount;
  char           flname[128];
  FILE          *upfp;
  FILE          *upfpx;
protected:
  CCLWin(){}
public:
  CCLWin(FXApp *app);
  virtual void create();
  virtual ~CCLWin();
public:
  void loadClients();
  int appendClient(int client);
  void deleteClient(int client);
  void setClientDisconnected(int client,FXbool disconnected = TRUE);
  void setClientMember(int client);
  void setEmployeeID(int empid);
  int getEmployeeID();
  FXbool auth(int id,FXuchar * digest);
  FXbool authemp(FXuchar *,FXuchar * digest);
  void updateInfo(int client);
  void updateClientIcon(int client);
  int getSelectedClient();
  void showCashing();
  void showProducts();
  void drawPasswordBox(FXEvent* event);
  void clearPasswordBox();
  void setPerms(unsigned long perm);
  void send_cmd_to_all(long cmd, char *arg, int arglen);
  void setAllClientMember(int);
  long unBlockClient(int client);
  long blockClient(int client);
  long updateClient(int client);
  long updateAllClients();
  long getUpdateFileName(char *fname, int *buf);
  unsigned int getClientIndex(int client);
  unsigned int getPageCount(char *cupstr, int *client);
  void updateSummaryInfo();
  void getClientInfoStr(int client, char *clbuf, int len);
  int  dispMessage(char * message);
  int  applySettings(unsigned long settings);

public:
  long onCheckEvents(FXObject*,FXSelector,void*);
  long onCommand(FXObject*,FXSelector sel,void*);
  long onSwap(FXObject*,FXSelector,void*);
  long onTime(FXObject*,FXSelector,void*);
  void toggleClientSetting(int client,  int flag);
  long onAllowUserLogin(FXObject*,FXSelector,void*);
  long onAllowMemberLogin(FXObject*,FXSelector,void*);
  long onAllowTicketLogin(FXObject*,FXSelector,void*);
  long onAbout(FXObject*,FXSelector,void*);
  long onNewClient(FXObject*,FXSelector,void*);
  long onDelClient(FXObject*,FXSelector,void*);
  long onNewProduct(FXObject*,FXSelector,void*);
  long onDelProduct(FXObject*,FXSelector,void*);
  long onEditProduct(FXObject*,FXSelector,void*);
  long onSellProduct(FXObject*,FXSelector,void*);
  long onClientSelected(FXObject*,FXSelector,void* ptr);
  long onClientSelected2(FXObject*,FXSelector,void* ptr);
  long onShowClientMenu(FXObject*,FXSelector,void* ptr);
  long onShowClientMenu2(FXObject*,FXSelector,void* ptr);
  long onTimerTick(FXObject*,FXSelector,void*);
  long onProductAdd(FXObject*,FXSelector,void* ptr);
  long onProductRemove(FXObject*,FXSelector,void* ptr);
  long onEmpLogin(FXObject*,FXSelector,void* ptr);
  long onMsgClient(FXObject*,FXSelector, void* ptr);
  long onMsgServer(FXObject*,FXSelector, void* ptr);
  long onAlertClient(FXObject*,FXSelector, void* ptr);
  long onCallAssist(FXObject*,FXSelector,void* ptr);
  long onEnableAssist(FXObject*,FXSelector,void* ptr);
  long onEnableAllAssist(FXObject*,FXSelector,void* ptr);
  long onAllAllowMemberLogin(FXObject*,FXSelector,void* ptr);
  long onAllAllowTicketLogin(FXObject*,FXSelector,void* ptr);
  long onAllAllowUserLogin(FXObject*,FXSelector,void* ptr);
  long onListShutter(FXObject*,FXSelector,void* ptr);
  long onExitPressed(FXObject*,FXSelector,void* ptr);
  long onCyberSet(FXObject*,FXSelector, void* ptr);
  FXbool employeeLogin(FXObject*, FXSelector, void*ptr);
  void employeeLogout(FXObject*, FXSelector, void*ptr);
  FXbool clientHelpIsUp(int client);
  bool confirmAll();
  long sendUpdateChunk(int, long);
  int  startUpdateClient(int client, char *fname);
  int  doNextUpdateFile(int client);
  int  getCurrentClient();
  long setAllClientPass();
  FXbool close(FXbool notify);
public:
  enum {
    ID_START = FXMainWindow::ID_LAST,ID_STOP,ID_UNSTOP,ID_SWAP,
    ID_TIME,ID_MONITOROFF,ID_REBOOT,ID_POWEROFF,ID_PAUSE,ID_ABOUT,
    ID_ALLOWUSERLOGIN,ID_NEWCLIENT,ID_DELCLIENT,ID_SETPASS, ID_SETMEMBER,
    ID_CLIENTSLIST,ID_TIMERTICK,ID_QUITCLIENT,ID_CHECKEVENTS,
    ID_ALLOWMEMBERLOGIN, ID_ALLOWTICKETLOGIN, ID_EMPLOGIN, ID_MSGCLIENT, ID_ENABLEASSIST,
    ID_ALLSETMEMBER, ID_ALLPOWEROFF, ID_ALLREBOOT, ID_ALLMONITOROFF,
    ID_ALLALLOWUSERLOGIN, ID_ALLALLOWTICKETLOGIN, ID_ALLALLOWMEMBERLOGIN, ID_ALLENABLEASSIST,
    ID_UPDATECLIENT, ID_ALLUPDATECLIENT,ID_TOGGLELIST, ID_CLIENTSLIST2,
    ID_SHUTTER, ID_SHUTTER2, ID_ALLADMINPASS, ID_10MIN, ID_20MIN, ID_30MIN,
    ID_60MIN, ID_CYBERSET, ID_LAST
  };
};

void updateClientStatus(int client);
void onEventCallback(int client,FXuint cmd,void *data,FXuint size,
		     void *userdata);
void onConnectCallback(int client,void *userdata);
void onDisconnectCallback(int client,void *userdata);



#endif
