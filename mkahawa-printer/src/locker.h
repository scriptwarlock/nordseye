// locker.h
//

#ifndef LOCKER_H
#define LOCKER_H

class Locker : public FXShell
{
FXDECLARE(Locker)
protected:
  FXGIFImage	*lockpix;
  FXFont	*font;
  char		*ctext;
  FXString	 input;
  FXString	 mlogin;
  int		 mid;
  struct { int x,y,w,h; } box;
  bool		 allowuserlogin;
  bool		 allowmemberlogin;
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
  void lock();
  void unlock();
  void drawPasswordBox(FXEvent* event);
  void clearPasswordBox();
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
