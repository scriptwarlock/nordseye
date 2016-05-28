// locker.h
//

#ifndef LOCKER_H
#define LOCKER_H
#include <fox-1.6/FXPNGIcon.h>

typedef struct{
  FXFont   *font;
  int       x,y,w,h,len;
  FXString  txt;
  bool      isSet;
  FXColor   fgcol;
} draw_logo_t;

class Locker : public FXShell
{
FXDECLARE(Locker)
protected:
  FXGIFImage	*lockpix;
  FXGIFIcon	*imgMember;
  FXGIFIcon	*imgTicket;
  FXGIFIcon	*imgPostpay;

  FXGIFIcon	*imgTicketPad;
  FXGIFIcon	*imgMemberPad;
  FXGIFIcon	*imgPasswdPad;
  FXGIFIcon     *imgBlankInp;
  FXGIFIcon     *imgLogin;
  //  FXPNGIcon	*imgLogo;
  FXGIFIcon	*imgLogo;

  FXFont	*font;
  char		*ctext;
  FXString	 minput;
  FXString	 mlogin;

  FXString	 pinput;
  FXString	 plogin;


  FXString	 tinput;
  FXString	 tlogin;


  int		 mid;
  struct { int x,y,w,h; } box;
  FXRectangle    tkt_box;
  FXRectangle    mbr_box;
  FXRectangle    pst_box;
  FXRectangle    pwd_box;
  FXRectangle    inp_box;

  FXRectangle    rcMbrLogin;
  FXRectangle    rcTktLogin;
  FXRectangle    rcUsrLogin;

  bool		 allowuserlogin;
  bool		 allowmemberlogin;
  bool		 allowticketlogin;
  draw_logo_t    dltxt;
protected:
  Locker(){}
public:
  Locker(FXApp *app);
  ~Locker();
  bool doesOverrideRedirect() const;
  virtual void create();
public:
  void allowUserLogin(bool allow);
  void allowMemberLogin(bool allow);
  void allowTicketLogin(bool allow);
  void lock();
  void unlock();
  void drawInputBox(FXEvent* event);
  void drawPostpayItems(FXEvent* ev);
  void drawMemberItems(FXEvent* ev);
  void drawTicketItems(FXEvent* ev);
  void initLogoText(FXEvent * ev);
  void writeLogoText(FXEvent * ev);
  void clearInputBox();
  void clearPasswordBox();
  void setMemberInputBox();
  void loadPixes(const char *fname, void *pixdata, FXGIFIcon **);  

  void clearTktCode();
  void clearMbrId();
  void clearMbrPwd();
  void clearMsgBox();

  int  memberLogin();
public:
  long onPaint(FXObject*,FXSelector,void* ptr);
  long onButtonRelease(FXObject*,FXSelector,void* ptr);
  long onKeyPress(FXObject*,FXSelector,void* ptr);
#ifdef MACOSX
  long onHandleMacOSXEvents(FXObject*,FXSelector,void*);
#endif
public:
  enum {
    ID_HANDLEMACOSXEVENTS = FXShell::ID_LAST, // Not used unless on OSX
    ID_LAST
  };
};
#endif
