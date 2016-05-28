#ifdef WIN32
# include <windows.h>
# include <winuser.h>
#endif

#ifdef MACOSX
# include <ctype.h>
# include <Carbon/Carbon.h>
# include <ApplicationServices/ApplicationServices.h>
#endif

#include <fox-1.6/fx.h>
#include <fox-1.6/FXRex.h>
#include <fox-1.6/fxkeys.h>
using namespace FX;

#include "cclcfox.h"
#include "locker.h"
#include "img_ctrl.h"

//#define DEBUG_IMAGE
//#define DEBUG_DRAW
//#define DEBUG_DIR
//#define DEBUG_KEY


extern CCLCFox *cclcfox;
extern unsigned char *locker_gif;
extern unsigned char *member_btn_gif;
extern unsigned char *ticket_btn_gif;
extern unsigned char *postpay_btn_gif;

extern unsigned char *passwd_pad_gif;
extern unsigned char *login_btn_gif;
extern unsigned char *blank_inp_gif;
extern unsigned char *logo_gif;

#ifdef MACOSX
extern Locker *locker;
#endif

FXDEFMAP(Locker) LockerMap[] =
{
#ifdef MACOSX
  FXMAPFUNC(SEL_CHORE,Locker::ID_HANDLEMACOSXEVENTS,
	    Locker::onHandleMacOSXEvents),
#endif
  FXMAPFUNC(SEL_PAINT,0,Locker::onPaint),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,Locker::onButtonRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,Locker::onKeyPress)
};

FXIMPLEMENT(Locker,FXShell,LockerMap,ARRAYNUMBER(LockerMap))
#ifdef WIN32
static FXuint ps;
static HHOOK kHook = NULL;
static HHOOK mHook = NULL;

static LRESULT CALLBACK mylpfn(int nCode,WPARAM wp,LPARAM lp)
{
  KBDLLHOOKSTRUCT *kbh = (KBDLLHOOKSTRUCT *) lp;
  BOOL ctrlDown = FALSE;

  if (cclcfox->isLocked()) {
    if (nCode == HC_ACTION) {
      ctrlDown = GetAsyncKeyState(VK_CONTROL) >> ((sizeof(SHORT) * 8) - 1);
      if (kbh->vkCode == VK_TAB && kbh->flags & LLKHF_ALTDOWN)
	return 1; // Alt+Tab
      if (kbh->vkCode == VK_ESCAPE && ctrlDown)
	return 1; // Ctrl+Escape
      if (kbh->vkCode == VK_ESCAPE && kbh->flags & LLKHF_ALTDOWN)
	return 1; // Alt+Escape
      if (kbh->vkCode == VK_LWIN || kbh->vkCode == VK_RWIN)
	return 1; // Win
      // otherwise
      return CallNextHookEx(kHook,nCode,wp,lp);
    } 
  } else
    return CallNextHookEx(kHook,nCode,wp,lp);
}
#endif

#ifdef MACOSX
// I'm going to create the window only once
static WindowRef winref = NULL;

struct _adbx11_keymap { int adb, x11; };
typedef struct _adbx11_keymap adbx11_keymap;

static adbx11_keymap mapArray[] = {
  {0x00,KEY_a},
  {0x01,KEY_s},
  {0x02,KEY_d},
  {0x03,KEY_f},
  {0x04,KEY_h},
  {0x05,KEY_g},
  {0x06,KEY_z},
  {0x07,KEY_x},
  {0x08,KEY_c},
  {0x09,KEY_v},
  {0x0B,KEY_b},
  {0x0C,KEY_q},
  {0x0D,KEY_w},
  {0x0E,KEY_e},
  {0x0F,KEY_r},
  {0x10,KEY_y},
  {0x11,KEY_t},
  {0x1F,KEY_o},
  {0x20,KEY_u},
  {0x22,KEY_i},
  {0x23,KEY_p},
  {0x24,KEY_Return},
  {0x25,KEY_l},
  {0x26,KEY_j},
  {0x28,KEY_k},
  {0x2D,KEY_n},
  {0x2E,KEY_m},
  {0x30,KEY_Tab},
  {0x33,KEY_BackSpace},
  {0x34,KEY_Return},
  {0x35,KEY_Escape},
  {0x47,KEY_Clear},
  {0x4C,KEY_Return},
  {0x60,KEY_F5},
  {0x61,KEY_F6},
  {0x62,KEY_F7},
  {0x63,KEY_F3},
  {0x64,KEY_F8},
  {0x65,KEY_F9},
  {0x67,KEY_F11},
  {0x69,KEY_F13},
  {0x6D,KEY_F10},
  {0x6F,KEY_F12},
  {0x6B,KEY_F14},
  {0x71,KEY_F15},
  {0x72,KEY_Help},
  {0x73,KEY_Home},
  {0x74,KEY_Page_Up},
  {0x75,KEY_Delete},
  {0x76,KEY_F4},
  {0x77,KEY_End},
  {0x78,KEY_F2},
  {0x79,KEY_Page_Down},
  {0x7A,KEY_F1},
  {0x7B,KEY_Left},
  {0x7C,KEY_Right},
  {0x7D,KEY_Down},
  {0x7E,KEY_Up},
  {-1,-1}
};

static int
ADB2X11(int code)
{
  adbx11_keymap *ptr = NULL;

  for (ptr = mapArray; (ptr->adb != -1 && code > ptr->adb); ptr++)
    ;
  
  return (ptr->adb != -1) ? ptr->x11 : code;
}

// This function is going to handle key press, and mouse click events
long
Locker::onHandleMacOSXEvents(FXObject*,FXSelector,void*)
{
  EventRef event;
  int err;

  while (noErr ==
	  (err = ReceiveNextEvent(0,NULL,1.0/1000.0,true,&event))) {
    switch (GetEventClass(event)) {
    case kEventClassKeyboard:
      {
	if (kEventRawKeyDown == GetEventKind(event)) {
	  UInt32 keycode;
	  char chr;
	  FXEvent fxev;

	  GetEventParameter(event,kEventParamKeyCode,typeUInt32,NULL,
			    sizeof(UInt32),NULL,&keycode);
	  GetEventParameter(event,kEventParamKeyMacCharCodes,typeChar,NULL,
			    sizeof(char),NULL,&chr);

	  fxev.code = ADB2X11(keycode);
	  if (isprint(chr))
	    fxev.text.assign(chr);

	  onKeyPress(NULL,0,&fxev);
	}
	break;
      }
    case kEventClassMouse:
      {
	if (kEventMouseUp == GetEventKind(event)) {
	  Point where;
	  EventMouseButton button;
	  FXEvent fxev;

	  GetEventParameter(event,kEventParamMouseButton,typeMouseButton,NULL,
			    sizeof(EventMouseButton),NULL,&button);

	  if (kEventMouseButtonPrimary == button) {
	    GetEventParameter(event,kEventParamMouseLocation,typeQDPoint,NULL,
			      sizeof(Point),NULL,&where);

	    fxev.win_x = where.h;
	    fxev.win_y = where.v;

	    onButtonRelease(NULL,0,&fxev);
	  }
	}
	break;
      }
    default:
      break;
    }

    SendEventToEventTarget(event,GetEventDispatcherTarget());

    ReleaseEvent(event);
  }

  if (!(getApp()->hasChore(this,ID_HANDLEMACOSXEVENTS)))
    getApp()->addChore(this,ID_HANDLEMACOSXEVENTS,NULL);
}
#endif


void 
Locker::loadPixes(const char *imgFname, void *pixdata, FXGIFIcon **icon)
{
  if (FXStat::exists(imgFname)) {
    *icon = new FXGIFIcon(getApp());
    FXFileStream stream;

    stream.open(imgFname,FXStreamLoad);
    (*icon)->loadPixels(stream);
    stream.close();
#ifdef DEBUG_IMAGE
    printf("loadPixes(): Loaded file %s: {%d} [%d,%d]\n", imgFname,
	   (*icon)->id(), (*icon)->getWidth(), (*icon)->getHeight());
#endif    
  } 
  else{ 
    *icon = new FXGIFIcon(getApp(),pixdata);
#ifdef DEBUG_IMAGE
    printf("Locker(): Retrieved {%s} from pstpix: {%d} [%d,%d]\n", imgFname, 
	   (*icon)->id(), (*icon)->getWidth(), (*icon)->getHeight());
#endif    
  }
}

Locker::Locker(FXApp * app):FXShell(app,0,0,0,0,0)
{
  void *imgpix = (void *)locker_gif;
  void *mbrpix = (void *)member_btn_gif;
  void *tktpix = (void *)ticket_btn_gif;
  void *pstpix = (void *)postpay_btn_gif;
  
  void *ppwdpix = (void *)passwd_pad_gif;
  void *loginpix = (void *)login_btn_gif;
  void *binppix = (void *)blank_inp_gif;
  void *logopix = (void *)logo_gif;

  enable();
  ctext = fxstrdup(_("Click here to start"));
  mid = -1;
  minput.clear();
  mlogin.clear();
  font = new FXFont(getApp(),"arial",10,FXFont::Bold);

  box.h = 80;
  box.w = 200;
  box.x = getRoot()->getDefaultWidth()/2 - box.w/2;
  box.y = getRoot()->getDefaultHeight()/2 - box.h/2;


#ifdef DEBUG_DIR
  printf("Current Directory: %s\n", FXSystem::getCurrentDirectory().text());
#endif
  // If "lockpix.gif" exists, lets show it when the screen is locked
  if (FXStat::exists("lockpix.gif")) {
    lockpix = new FXGIFImage(getApp(),NULL,IMAGE_OPAQUE);
    FXFileStream stream;

    stream.open("lockpix.gif",FXStreamLoad);
    lockpix->loadPixels(stream);
    stream.close();
#ifdef DEBUG_IMAGE
    printf("Locker::Locker(): Loaded lockpix from file: {%d} [%d,%d]\n", 
	   lockpix->id(), lockpix->getWidth(), lockpix->getHeight());
#endif    
  } 
  else{ 
    lockpix = new FXGIFImage(getApp(),imgpix,IMAGE_OPAQUE);
#ifdef DEBUG_IMAGE
    printf("Locker(): Retrieved lockpix from imgpix: {%d} [%d,%d]\n", 
	   lockpix->id(), lockpix->getWidth(), lockpix->getHeight());
#endif    
  }
  loadPixes("postpay_btn.gif", pstpix, &imgPostpay);
  pst_box.h = imgPostpay->getHeight();
  pst_box.w = imgPostpay->getWidth();
  loadPixes("ticket_btn.gif", tktpix, &imgTicket);
  tkt_box.h = imgTicket->getHeight();
  tkt_box.w = imgTicket->getWidth();
  loadPixes("member_btn.gif", mbrpix, &imgMember);
  mbr_box.h = imgMember->getHeight();
  mbr_box.w = imgMember->getWidth();
  loadPixes("passwd_pad.gif", ppwdpix, &imgPasswdPad);
  loadPixes("login_btn.gif", loginpix, &imgLogin);
  rcTktLogin.h = imgLogin->getHeight();
  rcTktLogin.w = imgLogin->getWidth();
  rcMbrLogin.h = imgLogin->getHeight();
  rcMbrLogin.w = imgLogin->getWidth();
  loadPixes("blank_input.gif", binppix, &imgBlankInp);
  inp_box.h = imgBlankInp->getHeight();
  inp_box.w = imgBlankInp->getWidth();
  imgLogo = new FXGIFIcon(getApp(), logopix);
  dltxt.isSet = FALSE;
  allowmemberlogin = FALSE;
  allowticketlogin = FALSE;
  allowuserlogin = FALSE;
}

Locker::~Locker()
{
  delete imgPasswdPad;
  delete imgBlankInp;
  delete imgTicket;
  delete imgPostpay;
  delete imgMember;
  delete imgLogo;
  delete lockpix;
  delete font;

  FXFREE(&ctext);

#ifdef MACOSX
  if (getApp()->hasChore(this,ID_HANDLEMACOSXEVENTS))
    getApp()->removeChore(this,ID_HANDLEMACOSXEVENTS);
#endif
}

bool
Locker::doesOverrideRedirect() const
{
  return TRUE;
}

void
Locker::create()
{
  FXShell::create();

  lockpix->create();
  imgMember->create();
  imgPostpay->create();
  imgTicket->create();
  imgPasswdPad->create();
  imgBlankInp->create();
  imgLogo->create();

  font->create();
}

void
Locker::allowUserLogin(bool allow)
{
  allowuserlogin = allow;
  repaint();
  update();
}

void
Locker::allowTicketLogin(bool allow)
{
  allowticketlogin = allow;
  if (!allowticketlogin) {
    mid = -1;
    minput.clear();
    mlogin.clear();
    clearInputBox();
  }
  repaint();
  update();
}

void
Locker::allowMemberLogin(bool allow)
{
  allowmemberlogin = allow;
  if (!allowmemberlogin) {
    mid = -1;
    minput.clear();
    mlogin.clear();
    clearInputBox();
  }
  repaint();
  update();
}

void
Locker::lock()
{
  int width = getRoot()->getDefaultWidth();
  int height = getRoot()->getDefaultHeight();

  position(0,0,width,height);
  cclcfox->setLoginMode(0); 
  show();
  grabKeyboard();
#ifdef DEBUG_IMAGE
  printf("Locker::lock():  Lock Effected\n");
#endif  

#ifdef MACOSX
# if 0
  raise();
  // Create the window if it wasn't created before
  if (winref == NULL) { 
    Rect rect;

    SetRect(&rect,0,0,width,height);
  // kUtilityWindowClass
    OSStatus err = CreateNewWindow(kDocumentWindowClass,		      // Plain Window
		    kWindowNoTitleBarAttribute	      // No titlebar
		    |kWindowStandardHandlerAttribute, // Standard event handler
		    //|kWindowIgnoreClicksAttribute,    // Pass clicks
		    &rect,
		    &winref);
    printf("CreateNewWindow: err = %d\n",err); // DEL
    err = SetWindowAlpha(winref,0.3);
    printf("SetWindowAlpha: err = %d\n",err); // DEL
  }
 
  // Bring it to front, and make it active 
# if 0
  BringToFront(winref);
  ActivateWindow(winref,true);
  // Make it cover the entire screen
  ConstrainWindowToScreen(winref,kWindowContentRgn,
			  kWindowConstraintMayResize
			  |kWindowConstraintMoveRegardlessOfFit,
			  NULL,
			  NULL);
# endif

  // Alternative method
# if 1
  MoveWindow(winref,0,0,false);
  SizeWindow(winref,width,height,true);
  SelectWindow(winref);
  ShowWindow(winref);
# endif
   // Hide the menubar, the dock, and disable some shortcuts 
  SetSystemUIMode(kUIModeAllHidden,
		  kUIOptionDisableAppleMenu
		  |kUIOptionDisableProcessSwitch
		  |kUIOptionDisableForceQuit
		  |kUIOptionDisableSessionTerminate);
 
  ProcessSerialNumber myProcess;
  GetCurrentProcess(&myProcess);
  SetFrontProcess(&myProcess);
# endif 
  // Add a callback to handle Carbon Events
  if (!(getApp()->hasChore(this,ID_HANDLEMACOSXEVENTS)))
    getApp()->addChore(this,ID_HANDLEMACOSXEVENTS,NULL);
# if 1 // Method 3
  // Capture the display
  CGDirectDisplayID dpy = CGMainDisplayID();
  CGDisplayCapture(dpy);
  winref = (WindowRef)CGShieldingWindowID(dpy);
# endif
#endif
#ifdef WIN32
  SetForegroundWindow((HWND) id());
  OSVERSIONINFO osversion;

  osversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&osversion);
  
  SetWindowPos((HWND) id(),HWND_TOPMOST,0,0,width,height,
		SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOOWNERZORDER);

  if (osversion.dwPlatformId == VER_PLATFORM_WIN32_NT) {	// NT/2000/XP
    HWND hwnd = FindWindow("Shell_traywnd",NULL);

    EnableWindow(hwnd,FALSE);
    // Disable the task manager
    HKEY hk;
    DWORD val = 1;
    const char *key =
      "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System";
    
    if (RegOpenKey(HKEY_CURRENT_USER,key,&hk) != ERROR_SUCCESS)
      RegCreateKey(HKEY_CURRENT_USER,key,&hk);
    RegSetValueEx(hk,"DisableTaskMgr",0,REG_DWORD,(BYTE *) &val,sizeof(val));
    // Add Hooks
    if (!kHook)
      kHook = SetWindowsHookEx(WH_KEYBOARD_LL,(HOOKPROC) mylpfn,
			       (HINSTANCE) GetModuleHandle(NULL),0);
    /*  if (!mHook)
     *   mHook = SetWindowsHookEx (WH_MOUSE,(HOOKPROC)mylpfn,
     *    (HINSTANCE)GetModuleHandle(NULL),0);
     */
  } else // All the above was only for NT/2000/XP, this is for Win 9x/Me
    SystemParametersInfo(SPI_SETSCREENSAVERRUNNING,TRUE,&ps,0);
#endif
}

void
Locker::unlock()
{
  if (img_ctrl)  /*Stop playing images*/
    img_ctrl->haltPlay((void *)this);
  hide();
  ungrabKeyboard();
#ifdef MACOSX
# if 0 // Method 1 and 2
  // Reenable everithing again
  SetSystemUIMode(kUIModeNormal,0);
  // Hide the window
  HideWindow(winref);
# endif
# if 1 // Method 3
  // Release the display
  CGDirectDisplayID dpy = CGMainDisplayID();
  CGDisplayRelease(dpy);
# endif
#endif
#ifdef WIN32
  SetWindowPos((HWND) id(),HWND_BOTTOM,0,0,0,0,
	       SWP_HIDEWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|
	       SWP_NOOWNERZORDER);

  OSVERSIONINFO osversion;

  osversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&osversion);

  if (osversion.dwPlatformId == VER_PLATFORM_WIN32_NT) {	// NT/2000/XP
    // Show the taskbar
    HWND hwnd = FindWindow("Shell_traywnd",NULL);

    EnableWindow(hwnd,TRUE);
    // Reenable the task manager
    HKEY hk;
    const char *key =
      "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System";
    if (RegOpenKey(HKEY_CURRENT_USER,key,&hk) != ERROR_SUCCESS)
      RegCreateKey(HKEY_CURRENT_USER,key,&hk);
    RegDeleteValue(hk,"DisableTaskMgr");
    // Remove Hooks
    if (kHook) {
      UnhookWindowsHookEx(kHook);
      kHook = NULL;
    }
    if (mHook) {
      UnhookWindowsHookEx(mHook);
      mHook = NULL;
    }
  } else // 9x/Me
    SystemParametersInfo(SPI_SETSCREENSAVERRUNNING,FALSE,&ps,0);
#endif
  if (img_ctrl)   /*Start playing images*/
    img_ctrl->playImages((void *)this);
}

#define TXT_BOX_W    (130)
#define TXT_BOX_H    (20)
void
Locker::drawInputBox(FXEvent* event)
{
  FXString txt;
  if (cclcfox->getLoginMode() == OPMODE_TICKET){
    txt = tinput + "|";
  }
  else if (cclcfox->getMemberLoginState() == MEMBER_LOGIN_PWD){
    txt = minput + "|";
    txt.replace(0,txt.length()-1,'*',txt.length()-1);
  }
  else {
    txt = minput + "|";
  }
  int iwidth = font->getTextWidth(txt.text());
  FXDCWindow dc(this,event);
  dc.begin(this);
  struct {int x, y, w, h;} txt_box = {inp_box.x,inp_box.y,TXT_BOX_W,TXT_BOX_H};
  dc.setClipRectangle(txt_box.x,txt_box.y,txt_box.w,txt_box.h);
  dc.setForeground(FXRGB(255,255,255));
  dc.fillRectangle(txt_box.x,txt_box.y,txt_box.w,txt_box.h);
  dc.setForeground(FXRGB(0,0,0));
  dc.drawRectangle(txt_box.x,txt_box.y,txt_box.w,txt_box.h);
  dc.setFont(font);
  dc.drawText(txt_box.x + txt_box.w/2 - iwidth/2,txt_box.y+16,txt.text(),txt.length());
  dc.end();
}

void
Locker::initLogoText(FXEvent* event)
{
  FXDCWindow dc(this,event);
  FXColor  bg;

  dltxt.isSet = TRUE;
  dltxt.font = new FXFont(getApp(), "verdana",12,FXFont::Bold | FXFont::Italic);
  dltxt.font->create();
  dltxt.txt = "mkahawa cyber software - from - www.unwiretechnologies.net";
  dltxt.w = dltxt.font->getTextWidth(dltxt.txt);
  dltxt.h = dltxt.font->getTextHeight(dltxt.txt);
  dltxt.x = (width - (dltxt.w))/2;
  dltxt.y = height - (dltxt.h+3);
  bg = dc.readPixel(dltxt.x,dltxt.y);
  dltxt.fgcol = FXRGB(255-FXREDVAL(bg), 255-FXGREENVAL(bg), 255-FXBLUEVAL(bg));
#ifdef DEBUG_DRAW
  printf("intLogoText(): (%d, %d), %0x08 -> %0x08\n", dltxt.x, dltxt.y, bg, dltxt.fgcol);
#endif
}

void
Locker::writeLogoText(FXEvent* event)
{
  FXDCWindow dc(this,event);
  dc.begin(this);
  dc.setForeground(dltxt.fgcol);
  dc.setFont(dltxt.font);
  dc.drawText(dltxt.x, dltxt.y+16, dltxt.txt);
  dc.end();
}


void
Locker::clearInputBox()
{
  struct {int x, y, w, h;} txt_box = {inp_box.x+25,inp_box.y+27,TXT_BOX_W,TXT_BOX_H};
  update(inp_box.x,inp_box.y,TXT_BOX_W+1,TXT_BOX_H+1);
}

void
Locker::drawPostpayItems(FXEvent *ev)
{
  FXDCWindow dc(this,ev);
  dc.begin(this);
  pst_box.x = (width - imgPostpay->getWidth()) / 2;
  pst_box.y = (height - (imgPostpay->getHeight() * 2 )) + 25;
#ifdef DEBUG_IMAGE
  printf("Locker::drawPostpayItems(): userlogin image: (%d,%d) -> [%d,%d]\n", 
	 pst_box.x, pst_box.y, pst_box.w, pst_box.h);
#endif    
  dc.drawIcon(imgPostpay, pst_box.x, pst_box.y);  
  dc.end();
}

void
Locker::drawTicketItems(FXEvent *ev)
{
  FXDCWindow dc(this,ev);
  dc.begin(this);
  tkt_box.x = (width - imgTicket->getWidth()) / 2  + (imgTicket->getWidth() * 1.5);
  tkt_box.y = (height - (imgTicket->getHeight() * 1.5)) + 25;
#ifdef DEBUG_IMAGE
  printf("Locker::drawTicketItems(): ticket image: (%d,%d) -> [%d,%d]\n", 
	 tkt_box.x, tkt_box.y, tkt_box.w, tkt_box.h);
#endif    
  dc.drawIcon(imgTicket, tkt_box.x, tkt_box.y);
  if (cclcfox->getLoginMode() == OPMODE_TICKET){ //draw pad
    rcTktLogin.x = tkt_box.x + 79;
    rcTktLogin.y = tkt_box.y + 106;
    inp_box.x = tkt_box.x + 14;
    inp_box.y = tkt_box.y + 69;
  }
  dc.end();
}

void
Locker::setMemberInputBox()
{
  if (cclcfox->getLoginMode() == OPMODE_MEMBER){ //draw pad
    inp_box.x = mbr_box.x + 12;
    if (cclcfox->getMemberLoginState() == MEMBER_LOGIN_NAME){  // accept name
      inp_box.y = mbr_box.y + 40;
    }
    else if (cclcfox->getMemberLoginState() == MEMBER_LOGIN_PWD){  // accept password
      inp_box.y = mbr_box.y + 78;
    }
  }
}


void
Locker::drawMemberItems(FXEvent *ev)
{
  FXDCWindow dc(this,ev);
  dc.begin(this);
  mbr_box.x = (width - imgMember->getWidth()) / 2  - (imgMember->getWidth() * 1.5);
  mbr_box.y = (height - (imgMember->getHeight() * 1.5)) + 25;
#ifdef DEBUG_IMAGE
  printf("Locker::drawMemberItems():  member image: (%d,%d) -> [%d,%d]\n", 
	 mbr_box.x, mbr_box.y, mbr_box.w, mbr_box.h);
#endif    
  dc.drawIcon(imgMember, mbr_box.x, mbr_box.y);
  rcMbrLogin.x = mbr_box.x + 79;
  rcMbrLogin.y = mbr_box.y + 106;
  setMemberInputBox();

  dc.end();
}

long
Locker::onPaint(FXObject*,FXSelector,void* ptr)
{
  FXEvent *event = (FXEvent *) ptr;
  FXDCWindow dc(this, event);

  dc.begin(this);
  dc.setForeground(FXRGB(0,0,0));
  dc.fillRectangle(0, 0, getRoot()->getDefaultWidth(), getRoot()->getDefaultHeight());
  dc.drawImage(lockpix, (width-lockpix->getWidth())/2, (height-lockpix->getHeight())/2);
  if (allowmemberlogin){
    drawMemberItems(event);
  }
  if (allowticketlogin){
    drawTicketItems(event);
  }
  if (allowuserlogin){
    drawPostpayItems(event);
  }
  //draw logo
  dc.drawIcon(imgLogo, 50, height-130);
  dc.end();
  if (!dltxt.isSet)
    initLogoText(event);
  writeLogoText(event);

  return 1;
}

bool isInBox(int px, int py, int x, int y, int w, int h)
{
  bool retval;

  retval = ( px >= x && px <= (x+w)  && py>=y && py <= (y+h)) ;
#ifdef DEBUG_IMAGE
  printf ("isInBox: [%d, %d] [%d, %d] [%d, %d] -> retval = %d\n", px, py, x, y, w, h,
	  (int) retval);
#endif
  return retval;
}


long
Locker::onButtonRelease(FXObject*,FXSelector,void* ptr)
{
  FXEvent *event = (FXEvent *) ptr;
  if (allowuserlogin && isInBox(event->win_x, event->win_y,
				pst_box.x, pst_box.y,
				pst_box.w, pst_box.h)) {
    cclcfox->setLoginMode(OPMODE_POSTPAY);
    cclcfox->userStart();
  }
  //ticket login
  else if (allowticketlogin && isInBox(event->win_x, event->win_y,
				       rcTktLogin.x, rcTktLogin.y,
				       rcTktLogin.w, rcTktLogin.h)){
    cclcfox->unlockWithPass(tinput);
    cclcfox->setLoginMode(OPMODE_TICKET);
    tinput.clear();
    drawTicketItems(event);
  }
  // Ticket ID
  else if (allowticketlogin && isInBox(event->win_x, event->win_y,
				       tkt_box.x, tkt_box.y,
				       tkt_box.w, tkt_box.h)){
    
    if (cclcfox->getLoginMode() != OPMODE_TICKET)
      tinput.clear();
    cclcfox->setLoginMode(OPMODE_TICKET);
    drawTicketItems(event);
  }
  //member login
  else if (allowmemberlogin && isInBox(event->win_x, event->win_y,
				       rcMbrLogin.x, rcMbrLogin.y,
				       rcMbrLogin.w, rcMbrLogin.h)) {
    //Try password
    cclcfox->unlockWithPass(mlogin, minput);
    //init vars - just in case login failed
    minput.clear();
    mlogin.clear();
    cclcfox->setLoginMode(OPMODE_MEMBER);
    cclcfox->setMemberLoginState(MEMBER_LOGIN_NAME);
    drawMemberItems(event);
  }
  //member pad clicked 
  else if (allowmemberlogin && isInBox(event->win_x, event->win_y,
				       mbr_box.x, mbr_box.y,
				       mbr_box.w, mbr_box.h)) {
    //clear if coming from another window
    if (cclcfox->getLoginMode() != OPMODE_MEMBER){
      mlogin.clear();
      minput.clear();
      drawMemberItems(event);
    }
      //member password clicked
    if (isInBox(event->win_x, event->win_y, mbr_box.x, mbr_box.y, mbr_box.w, mbr_box.h)) {
      cclcfox->setLoginMode(OPMODE_MEMBER);
      cclcfox->setMemberLoginState(MEMBER_LOGIN_NAME);
    }
    //member name clicked
    else if (isInBox(event->win_x, event->win_y, pwd_box.x, pwd_box.y, pwd_box.w, pwd_box.h)){
	cclcfox->setLoginMode(OPMODE_MEMBER);
	cclcfox->setMemberLoginState(MEMBER_LOGIN_PWD);
    }
    //draw the cursor or original content
    setMemberInputBox();
    drawInputBox(event);
  }
  //
  return 1;
}

long
Locker::onKeyPress(FXObject*,FXSelector,void* ptr)
{
  FXEvent *event = (FXEvent *) ptr;

#ifdef DEBUG_KEY
  printf("Locker::onKeyPress() memberlogin(%d), ticketlogin(%d), LoginMode(%d)\n", 
	 allowmemberlogin, allowticketlogin, cclcfox->getLoginMode());
#endif
  if (!cclcfox->getLoginMode() || cclcfox->getLoginMode()==OPMODE_POSTPAY)  // no mode / user start
    return 1;  

  //start processing keys 
  //Normal character key
  if ((event->code >= 0x0020 && event->code <= 0x00FF) 
      || (event->code >= 0xFFB0 && event->code <= 0xFFB9)) {
    if( cclcfox->getLoginMode() == OPMODE_TICKET )
      tinput.append(event->text);
    else
      minput.append(event->text);
    drawInputBox(event);
#ifdef DEBUG_KEY
    printf("Locker::onKeyPress() minput,tinput(%s,%s) \n", minput.text(), tinput.text());
#endif
  } 
  //Escape key - clear the text
  else if (event->code == KEY_Escape) {
    if( cclcfox->getLoginMode() == OPMODE_TICKET )
      tinput.clear();
    else{
      minput.clear();
      mlogin.clear();
    }
    if (allowmemberlogin || allowticketlogin){
      cclcfox->setLoginMode(0);
      cclcfox->setMemberLoginState(0);
    }
    update();
  } 
  //Backspace key
  else if (event->code == KEY_BackSpace || event->code == KEY_Delete) {
    if( cclcfox->getLoginMode() == OPMODE_TICKET )
      tinput.trunc(minput.length() - 1);
    else
      minput.trunc(minput.length() - 1);
    drawInputBox(event);
  } 
  //Enter / Return Key
  else if (event->code == KEY_KP_Enter || event->code == KEY_Return) {
    if (cclcfox->getLoginMode() == OPMODE_MEMBER){ //member login
#ifdef DEBUG_KEY
      printf("Locker::onKeyPress() MemberLoginState (%d)\n", cclcfox->getMemberLoginState());
#endif
      if (cclcfox->getMemberLoginState() == MEMBER_LOGIN_NAME) { //username
	if (minput.length()) {
	  mlogin = minput;
	  cclcfox->setMemberLoginState(MEMBER_LOGIN_PWD);
	  //drawMemberItems(event);
	  minput.clear();
	  setMemberInputBox();
	  drawInputBox(event);
	}
      } 
      else {  // entered the password
	cclcfox->unlockWithPass(mlogin,minput);
	cclcfox->setMemberLoginState(MEMBER_LOGIN_NAME);
	drawMemberItems(event);
	minput.clear();
	mlogin.clear();
      }
    }
    else{  //ticket login
      cclcfox->unlockWithPass(tinput);
      tinput.clear();
      drawInputBox(event);
      clearInputBox();
    }
  }
  return 1;
}
