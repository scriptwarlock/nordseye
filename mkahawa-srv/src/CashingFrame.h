#ifndef CASHINGFRAME_H
#define CASHINGFRAME_H

class CashingFrame : public FXVerticalFrame
{
FXDECLARE(CashingFrame)
protected:
  FX7Segment	*timeprice;
  FXButton	*editmember;
  FXButton	*editprice;
  FX7Segment	*productsprice;
  FXLabel	*clientname;
  FXLabel	*membername;
  FXFoldingList	*clprodlist;
  FXFoldingList	*timeintervals;
  FX7Segment	*totaltime;
  FX7Segment	*totalprice;
  FXRealSpinner	*discountsp;
  FXButton	*okbutton;
  FXButton	*cancelbutton;
  FXButton	*cancelsalebutton;
  FXCheckButton	*printticketcb;
  FXButton	*sendcodebutton;
  FXButton	*editticket;
  FXButton	*calcchangebutton;
protected:
  int		 csession;
  int		 cclient;
  int		 cmember;
  FXuint	 cprice;
  FXuint	 cpprice;
  double	 discount;
  struct sale_item {
    int id;
    unsigned int amount;
    unsigned int price;
  }		*products;
protected:
  CashingFrame(){}
public:
  CashingFrame(FXComposite *parent);
  ~CashingFrame();
  void create();
public:
  int getSession();
  void setProductsSale(void* sale_products);
  void setSession(int session,FXbool forcashing = TRUE);
  void noPermInfo();
  void setPerms(long perm);
  void clear();

private:
  FXString generateSessionTicket(double taxpercent,double &owed_products,
				 double &owed_terminal);
  FXString generateSaleTicket(double taxpercent,double &owed_products,
			      double &owed_terminal);
  void printTicket();
public:
  long onDiscount(FXObject*,FXSelector,void*);
  long onCash(FXObject*,FXSelector,void*);
  long onCancel(FXObject*,FXSelector,void*);
  long onCancelSale(FXObject*,FXSelector,void*);
  long onEditPrice(FXObject*,FXSelector,void*);
  long onEditMember(FXObject*,FXSelector,void*);
  long onPrintTicketCheck(FXObject*,FXSelector,void*);
  long onSendCode(FXObject*,FXSelector,void*);
  long onTicketEdit(FXObject*,FXSelector,void*);
  long onCalcChange(FXObject*,FXSelector,void*);
public:
  enum {
    ID_CASH = FXVerticalFrame::ID_LAST,ID_CANCEL,ID_EDITPRICE,ID_EDITMEMBER,
    ID_PRINTTICKETCHECK,ID_SENDCODE,ID_TICKETEDIT,ID_CANCELSALE,ID_DISCOUNT,
    ID_CALCCHANGE,
    ID_LAST
  };
};
#endif
