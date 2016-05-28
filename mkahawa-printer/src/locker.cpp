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

#include "cclpfox.h"
#include "lockpix.h"
#include "locker.h"

extern CCLPFox *cclpfox;
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

  if (cclpfox->isLocked()) {
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

Locker::Locker(FXApp * app)
:FXShell(app,0,0,0,0,0)
{
  enable();
  ctext = fxstrdup(_("Click here to start"));
  mid = -1;
  input.clear();
  mlogin.clear();
  font = new FXFont(getApp(),"arial",20,FXFont::Bold);

  box.h = 80;
  box.w = 400;
  box.x = getRoot()->getDefaultWidth()/2 - box.w/2;
  box.y = getRoot()->getDefaultHeight()/2 - box.h/2;

  // If "lockpix.gif" exists, lets show it when the screen is locked
  if (FXStat::exists("lockpix.gif")) {
    lockpix = new FXGIFImage(getApp(),NULL,IMAGE_OPAQUE);
    FXFileStream stream;

    stream.open("lockpix.gif",FXStreamLoad);
    lockpix->loadPixels(stream);
    stream.close();
  } else // If not, lets show the CCL logo
    lockpix = new FXGIFImage(getApp(),lockscreen_gif,IMAGE_OPAQUE);

  allowmemberlogin = FALSE;
  allowuserlogin = TRUE;
}

Locker::~Locker()
{
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
Locker::allowMemberLogin(bool allow)
{
  allowmemberlogin = allow;
  if (!allowmemberlogin) {
    mid = -1;
    input.clear();
    mlogin.clear();
    clearPasswordBox();
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
  show();
  grabKeyboard();
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
}

void
Locker::drawPasswordBox(FXEvent* event)
{
  const char *title = (-1 == mid) ? _("Member ID:") : _("Password:");
  FXString txt = input + "|";
  if (-1 != mid)
    txt.replace(0,txt.length()-1,'*',txt.length()-1);
  int theight = font->getTextHeight(title);
  int twidth = font->getTextWidth(title);
  int iwidth = font->getTextWidth(txt.text());
  FXDCWindow dc(this,event);

  box.x = getRoot()->getDefaultWidth()/2 - box.w/2;
  box.y = getRoot()->getDefaultHeight()/2 - box.h/2;

  dc.begin(this);
  dc.setClipRectangle(box.x,box.y,box.w+1,box.h+1);
  // Main box
  dc.setForeground(FXRGB(0,0,0));
  dc.fillRectangle(box.x,box.y,box.w,box.h);
  dc.setForeground(FXRGB(100,100,140));
  dc.fillRectangle(box.x,box.y,box.w,theight);
  dc.setForeground(FXRGB(255,255,255));
  dc.drawRectangle(box.x,box.y,box.w,box.h);
  // Password/ID
  dc.setFont(font);
  dc.setForeground(FXRGB(0,0,0));
  dc.drawText(box.x + box.w/2 - twidth/2 + 2,box.y + 10 + theight/2 + 2,
	      title,strlen(title));
  dc.setForeground(FXRGB(255,255,255));
  dc.drawText(box.x + box.w/2 - twidth/2,box.y + 10 + theight/2,
	      title,strlen(title));
  // User input
  dc.drawText(box.x + box.w/2 - iwidth/2,box.y + 30 + theight,
	      txt.text(),txt.length());
  dc.end();
}

void
Locker::clearPasswordBox()
{
  update(box.x,box.y,box.w+1,box.h+1);
}

long
Locker::onPaint(FXObject*,FXSelector,void* ptr)
{
  int textheight = font->getTextHeight(ctext,strlen(ctext));
  int textwidth = font->getTextWidth(ctext,strlen(ctext));
  int width = getRoot()->getDefaultWidth();
  int height = getRoot()->getDefaultHeight();
  FXEvent *event = (FXEvent *) ptr;
  FXDCWindow dc(this,event);

  dc.begin(this);
  dc.setForeground(FXRGB(0,0,0));
  dc.fillRectangle(0,0,width,height);
  dc.drawImage(lockpix,(width - lockpix->getWidth()) / 2,
	       (height - lockpix->getHeight()) / 2);

  if (!allowuserlogin) return 1;

  // If the user clicks on this section,I will send
  // a "start session" request to the server
  dc.setForeground(FXRGB(50,50,50));
  dc.fillRectangle(0,0,width,textheight);
  dc.drawRectangle(0,0,width,textheight);
  dc.setForeground(FXRGB(0,0,0));
  dc.setFont(font);
  dc.drawText((width - textwidth) / 2,textheight,ctext,strlen(ctext));
  dc.setForeground(FXRGB(255,255,255));
  dc.drawText((width - textwidth) / 2 - 3,textheight - 3,ctext,strlen(ctext));
  dc.end();
  
  return 1;
}

long
Locker::onButtonRelease(FXObject*,FXSelector,void* ptr)
{
  FXEvent *event = (FXEvent *) ptr;

  if (allowuserlogin && font->getTextHeight(ctext) >= event->win_y) {
    mid = -1;
    input.clear();
    mlogin.clear();
    cclpfox->userStart();
  }

  return 1;
}

long
Locker::onKeyPress(FXObject*,FXSelector,void* ptr)
{
  FXEvent *event = (FXEvent *) ptr;

  if (!allowmemberlogin) return 1;

  if ((event->code >= 0x0020 && event->code <= 0x00FF)
      || (event->code >= 0xFFB0 && event->code <= 0xFFB9)) {
//      FXRex numeric("^\\d*$",REX_NORMAL);
      
//      if (mid != -1 || numeric.match(event->text)) {
	input.append(event->text);
	drawPasswordBox(event);
//      }     
  } else if (event->code == KEY_Escape) {
    input.clear();
    mlogin.clear();
    mid = -1;
    clearPasswordBox();
  } else if (event->code == KEY_BackSpace || event->code == KEY_Delete) {
    input.trunc(input.length() - 1);
    drawPasswordBox(event);
  } else if (event->code == KEY_KP_Enter || event->code == KEY_Return) {
    if (mid == -1) {
      if (input.length()) {
	mid = FXIntVal(input);
	mlogin = input;
	input.clear();
      }
      drawPasswordBox(event);
    } else {
      if (mid)
	cclpfox->unlockWithPass(mid,input);
      else
	cclpfox->unlockWithPass(mlogin,input);
      input.clear();
      mlogin.clear();
      mid = -1;
      clearPasswordBox();
    }
  }
  
  return 1;
}
