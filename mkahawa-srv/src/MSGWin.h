
#ifndef _MSG_WIN_H
#define _MSG_WIN_H

class MSGWin : public FXShell
{
FXDECLARE(MSGWin)
  protected:
  FXLabel	    *timelbl;
  FXLabel	    *msglbl;
  FXLabel	    *msgsrc;
  FXbool	     pressed;
  FXbool	     hidden;

protected:
  MSGWin(){}
public:
  MSGWin(FXApp *app);
  ~MSGWin();
  void create();
  int getDefaultWidth();
  int getDefaultHeight();
  void enableOkBtn();
  void disableCancelBtn();
public:
  void setMsg(const FXString &text);
  void setMsgSource(const FXString &text);
  void setTime(const FXString &text);
public:
  long onSignal(FXObject*,FXSelector,void*);
  long onCancelBtn(FXObject*,FXSelector,void*);
  long onOkBtn(FXObject*,FXSelector,void*);
  long onCommand(FXObject*,FXSelector sel,void*);
  long onCheckEvents(FXObject*,FXSelector,void*);
public:
  enum {
    ID_SIGNAL = FXShell::ID_LAST,ID_CANCELBTN, ID_OKBTN, ID_CHECKEVENTS,
    ID_LAST
  };
};


#endif// _MSG_WIN_H
