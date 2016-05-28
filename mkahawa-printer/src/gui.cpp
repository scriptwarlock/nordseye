#ifndef WIN32
#  include <X11/Xlib.h>
#  include <X11/keysymdef.h>
#else
#  include <windows.h>
#  include <winuser.h>
#endif
#include <signal.h>
#include <fox-1.6/fx.h>
#include <fox-1.6/fxkeys.h>
using namespace FX;

#include "cclpfox.h"
#include "gui.h"
#include "lockpix.h"

extern CCLPFox *cclpfox;
extern ClientWin *clientwin;

#ifdef WIN32
static HHOOK kHook = NULL;
static LRESULT CALLBACK
hklpfn(int nCode,WPARAM wp,LPARAM lp)
{
  KBDLLHOOKSTRUCT *kbh = (KBDLLHOOKSTRUCT *) lp;

  if (nCode == HC_ACTION) {
    if (kbh->vkCode == VK_F12 && wp == WM_KEYUP) {
      if (cclpfox->isInfoShown())
	cclpfox->hideInfo();
      else
	cclpfox->showInfo();
      return 1;
    }
  }
  return CallNextHookEx(kHook,nCode,wp,lp);
}
#endif

FXDEFMAP(Grabber) GrabberMap[] =
{
  FXMAPFUNC(SEL_MOTION,0,Grabber::onMotion),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,Grabber::onBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,Grabber::onBtnRelease),
  //FXMAPFUNC(SEL_KEYRELEASE,0,Grabber::onKeyRelease),
  FXMAPFUNC(SEL_PAINT,0,Grabber::onPaint)
};

FXIMPLEMENT(Grabber,FXShell,GrabberMap,ARRAYNUMBER(GrabberMap))

FXDEFMAP(ClientWin) ClientWinMap[] =
{
  FXMAPFUNC(SEL_SIGNAL,ClientWin::ID_SIGNAL,ClientWin::onSignal),
  FXMAPFUNC(SEL_COMMAND,ClientWin::ID_EXITBTN,ClientWin::onExitBtn),
  FXMAPFUNC(SEL_COMMAND,ClientWin::ID_SETPASS,ClientWin::onSetPassword),
  FXMAPFUNC(SEL_COMMAND,ClientWin::ID_HELPBTN,ClientWin::onHelpBtn)
};

FXIMPLEMENT(ClientWin,FXShell,ClientWinMap,ARRAYNUMBER(ClientWinMap))

Grabber::Grabber(FXApp * app)
:FXShell(app,0,0,20,0,0)
{
  enable();
  grabbericon = new FXGIFIcon(getApp(),grabber_gif,0,IMAGE_OPAQUE);
}

Grabber::~Grabber()
{
  delete grabbericon;
}

bool
Grabber::doesOverrideRedirect() const
{
  return TRUE;
}

void
Grabber::create()
{
  FXShell::create();
  grabbericon->create();
  show();
#ifdef WIN32
  SetWindowPos((HWND) id(),HWND_TOPMOST,0,20,0,0,
	       SWP_NOSIZE|SWP_NOREPOSITION);
#endif
  setHotkey();
}

int
Grabber::getDefaultWidth()
{
  return grabbericon->getWidth();
}

int
Grabber::getDefaultHeight()
{
  return grabbericon->getHeight();
}

void
Grabber::setHotkey()
{
#ifndef WIN32
  Display *dsp = (Display*)(getApp()->getDisplay());

  if (!dsp)
    return;

  XGrabKey(dsp,KEY_F12,AnyModifier,id(),FALSE,GrabModeAsync,GrabModeAsync);
#else
  if (!kHook) {
    OSVERSIONINFO osversion;

    osversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osversion);

    if (osversion.dwPlatformId == VER_PLATFORM_WIN32_NT)	// NT/2000/XP
      kHook = SetWindowsHookEx(WH_KEYBOARD_LL,(HOOKPROC) hklpfn,
			       (HINSTANCE) GetModuleHandle(NULL),0);
    else			// 9x/Me
      ; //kHook = SetWindowsHookEx(WH_KEYBOARD,(HOOKPROC) hklpfn,
	//		       (HINSTANCE) GetModuleHandle(NULL),0);
  }
#endif
}

void
Grabber::unsetHotkey()
{
#ifndef WIN32
  Display *dsp = (Display*)(getApp()->getDisplay());

  if (!dsp)
    return;
  XUngrabKey(dsp,KEY_F12,AnyModifier,id());
#else
  if (kHook) {
    UnhookWindowsHookEx(kHook);
    kHook = NULL;
  }
#endif
}

long
Grabber::onKeyRelease(FXObject*,FXSelector,void* ptr)
{
#ifndef WIN32
  FXEvent *event = (FXEvent *) ptr;

  cclpfox->hideInfo();

  if (event->code == KEY_F12) {
    /*    if (cclpfox->isInfoShown())
      cclpfox->hideInfo();
    else
      cclpfox->showInfo();
      return 1;*/
  } else
#endif
    return 0;
}

long
Grabber::onPaint(FXObject*,FXSelector,void* ptr)
{
  FXEvent *event = (FXEvent *) ptr;
  FXDCWindow dc(this,event);

  dc.setForeground(FXRGB(0,0,0));
  dc.fillRectangle(0,0,getDefaultWidth(),getDefaultHeight());
  dc.drawIcon(grabbericon,0,0);

  return 1;
}

long
Grabber::onMotion(FXObject*,FXSelector,void* ptr)
{
  FXEvent *event = (FXEvent *) ptr;

  if (flags & FLAG_PRESSED) {
    int newx = event->root_x - grabpt.x;
    int newy = event->root_y - grabpt.y;
    int rootw = getRoot()->getDefaultWidth();
    int rooty = getRoot()->getDefaultHeight();

    if (newx > rootw - grabbericon->getWidth())
      newx = rootw - grabbericon->getWidth();
    else if (newx <= 0)
      newx = 0;
    if (newy > rooty - grabbericon->getHeight())
      newy = rooty - grabbericon->getHeight() - getHeight();
    else if (newy <= 0)
      newy = 0;

    move(newx,newy);
    //clientwin->move(newx,newy + getHeight());
    return 1;
  }

  return 0;
}

long
Grabber::onBtnPress(FXObject*,FXSelector,void* ptr)
{
  if (!shown())
    show();
  FXEvent *event = (FXEvent *) ptr;

  grabpt.x = event->win_x;
  grabpt.y = event->win_y;
  flags |= FLAG_PRESSED;
  grab();

  return 1;
}

long
Grabber::onBtnRelease(FXObject*,FXSelector,void* ptr)
{
  FXEvent *event = (FXEvent *) ptr;

  flags &= ~FLAG_PRESSED;
  /*if (!event->moved) {
    if (clientwin->shown()) {
      clientwin->hide();
    } else {
      clientwin->show();
    }
    }*/
  ungrab();

  return 1;
}

ClientWin::ClientWin(FXApp * app)
:FXShell(app,0,0,0,180,50)
{
  FXVerticalFrame *vframe;
  FXHorizontalFrame *hframe1;
  FXHorizontalFrame *hframe2;
  FXVerticalFrame *vframe1;
  FXVerticalFrame *vframe2;
  FXVerticalFrame *vframe3;
  
  enable();
  vframe = new FXVerticalFrame(this,LAYOUT_FILL_Y|LAYOUT_FILL_X|FRAME_LINE,
			       0,0,180,50,0,0,0,0,0,0);
  hframe1 = new FXHorizontalFrame(vframe,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,
				  0,0,5,0,0,0,0,0);
  vframe1 =
    new FXVerticalFrame(hframe1,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0,
			0,0,0,0,0,0);
  FXLabel *label1 = new FXLabel(vframe1,_("Time:"),NULL,LABEL_NORMAL,
				3,0,0,0,0,0,0,0);

  timelbl = new FXLabel(vframe1,"00:00",NULL,
			   LAYOUT_FILL_Y|LAYOUT_FILL_X|FRAME_LINE);
//  timelbl->setCellHeight(12);
//  timelbl->setCellWidth(10);
  
  vframe2 =
    new FXVerticalFrame(hframe1,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0,
			0,0,0,0,0,0);
  FXLabel *label2 = new FXLabel(vframe2,_("Owed:"),NULL,LABEL_NORMAL,
				0,0,0,0,0,0,0,0);

  owedlbl = new FXLabel(vframe2,"00.00",NULL,
			   LAYOUT_FILL_Y|LAYOUT_FILL_X|FRAME_LINE);
//  owedlbl->setCellHeight(12);
//  owedlbl->setCellWidth(10);
  
  vframe3 =
    new FXVerticalFrame(hframe1,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0,
			  0,0,0,0,0,0);
  FXLabel *label3 = new FXLabel(vframe3,_("Other:"),NULL,LABEL_NORMAL,
				0,0,0,0,0,0,0,0);

  productslbl = new FXLabel(vframe3,"00.00",NULL,
			       LAYOUT_FILL_Y|LAYOUT_FILL_X|FRAME_LINE);
//  productslbl->setCellHeight(12);
//  productslbl->setCellWidth(10);
 
  hframe2 = new FXHorizontalFrame(vframe,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,
				 0,0,0,0,0,0,0,0);
  setpassbtn =
    new FXButton(hframe2,_("Password"),NULL,this,ID_SETPASS,
  		 FRAME_LINE|LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0,2,2,2,2);

  helpbtn =
    new FXButton(hframe2,_("Assist"),NULL,this,ID_HELPBTN,
		 FRAME_LINE|LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0,2,2,2,2);
  //default is disabled
  helpbtn->disable();
  exitbtn =
    new FXButton(hframe2,_("End Session"),NULL,this,ID_EXITBTN,
		 FRAME_LINE|LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0,2,2,2,2);

  FXColor backcolor = FXRGB(222,222,215);
  FXColor labelcolor = FXRGB(200,200,200);

  label1->setBackColor(backcolor);
  label2->setBackColor(backcolor);
  label3->setBackColor(backcolor);
  vframe->setBackColor(backcolor);
  hframe1->setBackColor(backcolor);
  hframe2->setBackColor(backcolor);
  vframe1->setBackColor(backcolor);
  vframe2->setBackColor(backcolor);
  vframe3->setBackColor(backcolor);
  timelbl->setBackColor(labelcolor);
  owedlbl->setBackColor(labelcolor);
  productslbl->setBackColor(labelcolor);
  setpassbtn->setBackColor(backcolor);
  setpassbtn->disable();
  helpbtn->setBackColor(backcolor);
  helpbtn->disable();
  //exitbtn->setBackColor(backcolor);
  exitbtn->setBackColor(FXRGB(250,150,150));
  // Signals
#ifndef WIN32
  getApp()->addSignal(SIGTERM,this,ID_SIGNAL);
  getApp()->addSignal(SIGQUIT,this,ID_SIGNAL);
#endif
}

ClientWin::~ClientWin()
{
}

bool
ClientWin::doesOverrideRedirect() const
{
  return TRUE;
}

void
ClientWin::create()
{
  FXShell::create();
  show();
#ifdef WIN32
  SetWindowPos((HWND) id(),HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOREPOSITION);
#endif
}

void
ClientWin::setPasswordEnabled(int enabled)
{
  if (enabled) setpassbtn->enable();
  else setpassbtn->disable();
}

void
ClientWin::setOwed(const FXString & text)
{
  owedlbl->setText(text);
}

void
ClientWin::enableAssist(bool assist)
{  
  enableassist = assist;

  if (assist)
    helpbtn->enable();
  else
    helpbtn->disable();
}

void
ClientWin::setProducts(const FXString & text)
{
  productslbl->setText(text);
}

void
ClientWin::setTime(const FXString & text)
{
  timelbl->setText(text);
}

int
ClientWin::getDefaultWidth()
{
  return 120;
}

int
ClientWin::getDefaultHeight()
{
  return 100;
}

long
ClientWin::onSignal(FXObject*,FXSelector,void*)
{
  return 1;
}

long
ClientWin::onExitBtn(FXObject*,FXSelector,void*)
{
  hide();
  cclpfox->userExit();

  return 1;
}

long 
ClientWin::onHelpBtn(FXObject *, FXSelector, void*)
{
  cclpfox->askForHelp();
  /*helpbtn->disable();*/
  return 0L;
}

void 
ClientWin::enableHelpBtn()
{
  helpbtn->enable();
}

void 
ClientWin::disableHelpBtn()
{
  helpbtn->disable();
}

long
ClientWin::onSetPassword(FXObject*,FXSelector,void*)
{
  setpassbtn->disable();
  exitbtn->disable();
  cclpfox->hideInfo();
  
  FXDialogBox dialog(getRoot(),_("Change password"));
  FXLabel lbl1(&dialog,_("Old password:"));
  FXTextField oldpass(&dialog,20,NULL,0,TEXTFIELD_PASSWD|FRAME_SUNKEN);
  FXLabel lbl2(&dialog,_("New password:"));
  FXTextField newpass1(&dialog,20,NULL,0,TEXTFIELD_PASSWD|FRAME_SUNKEN);
  FXLabel lbl3(&dialog,_("New password (again):"));
  FXTextField newpass2(&dialog,20,NULL,0,TEXTFIELD_PASSWD|FRAME_SUNKEN);
  FXHorizontalFrame hframe(&dialog);
  FXButton okbtn(&hframe,_("Ok"),NULL,&dialog,FXDialogBox::ID_ACCEPT);
  FXButton cancelbtn(&hframe,_("Cancel"),NULL,&dialog,FXDialogBox::ID_CANCEL);

  if (dialog.execute()) {
    if (newpass1.getText() == newpass2.getText()) {
      FXuchar digests[2*CCLC_MD5_DIGEST_LENGTH];

      CCLC_MD5((FXuchar*)oldpass.getText().text(),oldpass.getText().length(),
	       digests);
      CCLC_MD5((FXuchar*)newpass1.getText().text(),newpass1.getText().length(),
	       digests+CCLC_MD5_DIGEST_LENGTH);
      cclpfox->setPassword(digests);
    }
    else
      FXMessageBox::error(getRoot(),MBOX_OK,_("Error"),
			  _("The two given passwords were different"));

  }

  setpassbtn->enable();
  exitbtn->enable();
  cclpfox->showInfo();
 
  return 1;
}
