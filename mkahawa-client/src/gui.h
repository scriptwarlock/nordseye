// gui.h
//

#ifndef GUI_H
#define GUI_H

class Grabber : public FXShell
{
  FXDECLARE(Grabber)
  protected:
    FXIcon    *grabbericon;
    struct { int x,y; } grabpt;
  protected:
    Grabber(){}
  public:
    Grabber(FXApp *app);
    ~Grabber();
    virtual bool doesOverrideRedirect() const;
    void create();
    int getDefaultWidth();
    int getDefaultHeight();
    void setHotkey();
    void unsetHotkey();
  public:
    long onKeyRelease(FXObject*,FXSelector,void* ptr);
    long onMotion(FXObject*,FXSelector,void* ptr);
    long onBtnPress(FXObject*,FXSelector,void* ptr);
    long onBtnRelease(FXObject*,FXSelector,void* ptr);
    long onPaint(FXObject*,FXSelector,void* ptr);
  public:
    enum {
      ID_LAST = FXShell::ID_LAST
    };
};

class ClientWin : public FXShell
{
FXDECLARE(ClientWin)
protected:
  FXLabel	    *timelbl;
  FXLabel	    *owedlbl;
  FXLabel	    *lblBtnOwed;
  FXLabel	    *productslbl;
  FXButton	    *setpassbtn;
  FXButton	    *exitbtn;
  FXButton          *helpbtn;
  FXbool	     pressed;
  FXbool	     hidden;
  int                ack_assist;
  int                enableassist;

protected:
  ClientWin(){}
public:
  ClientWin(FXApp *app);
  ~ClientWin();
  virtual bool doesOverrideRedirect() const;
  void create();
  int getDefaultWidth();
  int getDefaultHeight();
  void enableHelpBtn();
  void disableHelpBtn();
  void enableAssist(bool);
public:
  void setPasswordEnabled(int enabled = 1);
  void setOwed(const FXString &text);
  void setOwedLbl(const FXString &text);
  void setProducts(const FXString &text);
  void setTime(const FXString &text);
  void dispMessage(FXString &msgstr, int timeout);
public:
  long onSignal(FXObject*,FXSelector,void*);
  long onExitBtn(FXObject*,FXSelector,void*);
  long onHelpBtn(FXObject*,FXSelector,void*);
  long onSetPassword(FXObject*,FXSelector,void*);
public:
  enum {
    ID_SIGNAL = FXShell::ID_LAST,ID_EXITBTN,ID_SETPASS,ID_HELPBTN,
    ID_LAST
  };
};


class MessageWin : public FXShell
{
FXDECLARE(MessageWin)
protected:
  FXLabel	    *lblSender;
  FXLabel	    *lblMessage;
  FXButton	    *btnOk;
  FXButton	    *btnCancel;
  FXButton          *btnExit;

protected:
  MessageWin(){}
public:
  MessageWin(FXApp *app);
  ~MessageWin();
  virtual bool doesOverrideRedirect() const;
  void create();
  int getDefaultWidth();
  int getDefaultHeight();
public:
  void dispMessage(FXString &msgstr, int timeout);
public:
  long onSignal(FXObject*,FXSelector,void*);
  long onBtnOk(FXObject*,FXSelector,void*);
  long onBtnCancel(FXObject*,FXSelector,void*);
  long onBtnExit(FXObject*,FXSelector,void*);
public:
  enum {
    ID_SIGNAL = FXShell::ID_LAST,ID_OKBTN, ID_EXITBTN,ID_CANCELBTN,
    ID_LAST
  };
};

#endif
