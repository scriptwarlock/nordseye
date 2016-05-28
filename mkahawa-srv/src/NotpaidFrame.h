#ifndef NOTPAIDFRAME_H
#define NOTPAIDFRAME_H

class NotpaidFrame : public FXVerticalFrame
{
FXDECLARE(NotpaidFrame)
protected:
  FXFoldingList *notpaidlist;
protected:
  NotpaidFrame(){}
public:
  NotpaidFrame(FXComposite *parent);
  ~NotpaidFrame();
  void create();
public:
  void readNotPaid();
  bool isUnpaid(int client, int session);
public:
  long onSelected(FXObject*,FXSelector,void* ptr);
public:
  enum {
    ID_NOTPAIDLIST = FXVerticalFrame::ID_LAST,
    ID_LAST
  };
};
#endif
