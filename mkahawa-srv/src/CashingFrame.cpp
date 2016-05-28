#include <ccls.h>
#include <fox-1.6/fx.h>
using namespace FX;

#include "cclfox.h"
#include "CashingFrame.h"
#include "ProductsFrame.h"
#include "NotpaidFrame.h"
#include "verifiers.h"
#include "CCLWin.h"
#include "EmployeesFrame.h"

extern FXGIFIcon *dbIcon1;
extern FXGIFIcon *dbIcon2;
extern FXGIFIcon *dbIcon3;

static FXuint
minusTax(FXuint price,double tax)
{
  if (tax > 0.0)
    return price - FXuint(price * (tax / (100.0 + tax)));
  else
    return price;
}

static double
minusTax(double price,double tax)
{
  if (tax > 0.0)
    return price - (price * (tax / (100.0 + tax)));
  else
    return price;
}

static FXuint
applyDiscount(FXuint price,double discount)
{
  if (discount > 0.001)
    return price - FXuint((price * discount) / 100.0);
  else
    return price;
}

static double
applyDiscount(double price,double discount)
{
  if (discount > 0.001)
    return price - (price * discount) / 100.0;
  else
    return price;
}

FXDEFMAP(CashingFrame) CashingFrameMap[] =
{
  FXMAPFUNC(SEL_COMMAND,CashingFrame::ID_CASH,CashingFrame::onCash),
  FXMAPFUNC(SEL_COMMAND,CashingFrame::ID_CANCEL,CashingFrame::onCancel),
  FXMAPFUNC(SEL_COMMAND,CashingFrame::ID_CANCELSALE,CashingFrame::onCancelSale),
  FXMAPFUNC(SEL_COMMAND,CashingFrame::ID_EDITMEMBER,CashingFrame::onEditMember),
  FXMAPFUNC(SEL_COMMAND,CashingFrame::ID_EDITPRICE,CashingFrame::onEditPrice),
  //FXMAPFUNC(SEL_COMMAND,CashingFrame::ID_PRINTTICKETCHECK,
  //	    CashingFrame::onPrintTicketCheck),
  FXMAPFUNC(SEL_COMMAND,CashingFrame::ID_SENDCODE, CashingFrame::onSendCode),
  FXMAPFUNC(SEL_COMMAND,CashingFrame::ID_DISCOUNT,CashingFrame::onDiscount),
  //FXMAPFUNC(SEL_COMMAND,CashingFrame::ID_TICKETEDIT,
  //	    CashingFrame::onTicketEdit),
  FXMAPFUNC(SEL_COMMAND,CashingFrame::ID_CALCCHANGE,CashingFrame::onCalcChange)
};

FXIMPLEMENT(CashingFrame,FXVerticalFrame,CashingFrameMap,
	    ARRAYNUMBER(CashingFrameMap))

extern CCLWin *mainwin;
extern ProductsFrame *productsframe;
extern NotpaidFrame *notpaidframe;

void
CashingFrame::noPermInfo()
{
    FXMessageBox::error(this,MBOX_OK,_("Permission"),
			_("Unable to access this feature.\n Contact the Administrator"));
    return;
}

void 
CashingFrame::setPerms(long perm)
{
  cancelsalebutton->enable();
  if (!isPermitted(PERMCASHCANCEL)){
    cancelsalebutton->disable();
  }
  okbutton->enable();
  if (!isPermitted(PERMCASHRECEIVE)){
    okbutton->disable();
  }
  discountsp->enable();
  if (!isPermitted(PERMCASHDISCOUNT)){
    discountsp->disable();
  }
  editprice->enable();
  if (!isPermitted(PERMCASHEDIT)){
    editprice->disable();
  }
  editmember->enable();
  if (!isPermitted(PERMMBREDIT)){
    editmember->disable();
  }
}

CashingFrame::CashingFrame(FXComposite * parent)
:FXVerticalFrame(parent,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN,
		 0,0,0,0,0,0,0,0,0,0)
{
  FXHorizontalFrame *hframe1 =
    new FXHorizontalFrame(this,LAYOUT_FILL_X,0,0,0,0,4,4,0,0,5,0);
  new FXLabel(hframe1,_("Client:"));
  clientname = new FXLabel(hframe1,"");
  editmember = new FXButton(hframe1,_("Edit"),dbIcon1,this,ID_EDITMEMBER,
			    BUTTON_TOOLBAR|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK);

  FXHorizontalFrame *hframe2 =
    new FXHorizontalFrame(this,LAYOUT_FILL_X,0,0,0,0,4,4,0,0,5,0);
  new FXLabel(hframe2,_("Browse Time:"));
  timeprice = new FX7Segment(hframe2,"-.--",FRAME_SUNKEN);
  timeprice->setCellHeight(14);
  timeprice->setCellWidth(10);
  timeprice->setTextColor(FXRGB(255,50,50));
  timeprice->setBackColor(FXRGB(0,0,0));
  editprice = new FXButton(hframe2,_("Edit"),dbIcon1,this,ID_EDITPRICE,
			   BUTTON_TOOLBAR|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK);
  
  FXHorizontalFrame *hframe3 =
    new FXHorizontalFrame(this,LAYOUT_FILL_X,0,0,0,0,4,4,0,0,5,0);
  new FXLabel(hframe3,_("Products:"));
  productsprice = new FX7Segment(hframe3,"-.--",FRAME_SUNKEN);
  productsprice->setCellHeight(14);
  productsprice->setCellWidth(10);
  productsprice->setTextColor(FXRGB(255,50,50));
  productsprice->setBackColor(FXRGB(0,0,0));

  FXVerticalFrame *listframe =
    new FXVerticalFrame(this,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
			0,0,0,0,0,0,0,0);
  clprodlist = new FXFoldingList(listframe,NULL,0,
				 FOLDINGLIST_SINGLESELECT|LAYOUT_FILL_X|
				 LAYOUT_FILL_Y);
  clprodlist->appendHeader(_("Name"),NULL,160);
  clprodlist->appendHeader(_("Amount"),NULL,60);
  clprodlist->appendHeader(_("Price"),NULL,60);
  new FXLabel(this,_("Time Interval:"));

  FXVerticalFrame *intervalsframe =
    new FXVerticalFrame(this,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
			0,0,0,0,0,0,0,0);
  timeintervals = new FXFoldingList(intervalsframe,NULL,0,
				    FOLDINGLIST_SINGLESELECT|LAYOUT_FILL_X|
				    LAYOUT_FILL_Y);
  timeintervals->appendHeader(_("Start"),NULL,210);
  timeintervals->appendHeader(_("Duration"),NULL,60);

  FXHorizontalFrame *hframe4 =
    new FXHorizontalFrame(this,LAYOUT_FILL_X,0,0,0,0,4,4,0,0,5,0);
  new FXLabel(hframe4,_("Total Duration:"));
  totaltime = new FX7Segment(hframe4,"--:--",FRAME_SUNKEN);
  totaltime->setCellHeight(14);
  totaltime->setCellWidth(10);
  totaltime->setTextColor(FXRGB(230,230,230));
  totaltime->setBackColor(FXRGB(0,0,0));

  FXHorizontalFrame *hframe5 =
    new FXHorizontalFrame(this,LAYOUT_FILL_X,0,0,0,0,4,4,0,0,5,0);
  new FXLabel(hframe5,_("Total Amount:"));
  totalprice = new FX7Segment(hframe5,"-.--",FRAME_SUNKEN);
  totalprice->setCellHeight(14);
  totalprice->setCellWidth(10);
  totalprice->setTextColor(FXRGB(50,255,50));
  totalprice->setBackColor(FXRGB(0,0,0));
  discountsp = new FXRealSpinner(hframe5,5,this,ID_DISCOUNT,
				 LAYOUT_RIGHT|FRAME_SUNKEN|FRAME_THICK);
  discountsp->setRange(0.0,100.0);
  discountsp->setValue(0.0);
  new FXLabel(hframe5,_("Discount:"),NULL,LAYOUT_RIGHT);
  new FXHorizontalSeparator(this);

  FXHorizontalFrame *hframe6 = new FXHorizontalFrame(this,LAYOUT_FILL_X);
  okbutton = new FXButton(hframe6,_("Cash"),dbIcon1,this,ID_CASH,
			  BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
  cancelbutton = new FXButton(hframe6,_("Log Cancelled"),dbIcon3,this,ID_CANCEL,
			      BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
  cancelsalebutton = new FXButton(hframe6,_("Cancel Sale"),dbIcon3,this,ID_CANCELSALE,
				  BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT);
  calcchangebutton = new FXButton(hframe6,_("Calc Change"),dbIcon3,this, ID_CALCCHANGE,
				  BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT);

  //FXHorizontalFrame *hframe7 = new FXHorizontalFrame(this,LAYOUT_FILL_X);
  //printticketcb = new FXCheckButton(hframe7,_("Print ticket"),this,
  //  				    ID_PRINTTICKETCHECK);
  //sendcodebutton = new FXButton(hframe7,_("Open cash register"),NULL,this,
  //  				ID_SENDCODE);
  if (getApp()->reg().readIntEntry("CASHING","print",0))
    printticketcb->setCheck(TRUE);

  /*  new FXHorizontalSeparator(this);

  FXHorizontalFrame *hframe8 = new FXHorizontalFrame(this,LAYOUT_FILL_X);
  //editticket = new FXButton(hframe8,_("Edit ticket format"),NULL,this,
  //			    ID_TICKETEDIT,FRAME_RAISED|FRAME_THICK);
  */
  products = NULL;
  clear();
}

CashingFrame::~CashingFrame()
{
}

void
CashingFrame::create()
{
  FXVerticalFrame::create();
}

int CashingFrame::getSession()
{
  return csession;
}

void
CashingFrame::setProductsSale(void *sale_products)
{
  double owed_products = 0.0;
  char buf[256];

  clear();
  
  products = (sale_item*)sale_products;

  if (0 >= products[0].id) return;

  csession = 0;

  for (int i = 0; 0 < products[i].id; i++) {
    int id = products[i].id;
    unsigned int amount = products[i].amount;
    char *category;
    char *name;
    unsigned int price = products[i].price;

    if (0 < amount && CCL_product_info_get(id,&category,&name,NULL)) {
      owed_products += price / 100.0;
      snprintf(buf,256,"%s::%s\t%u\t%.2f",category,name,
	       amount,price / 100.0);
      clprodlist->prependItem(NULL,buf,NULL,NULL,NULL);
      CCL_free(category);
      CCL_free(name);
    }
  }

  cpprice = (FXuint) (owed_products * 100);

  snprintf(buf,64,"%.2f",owed_products);
  productsprice->setText(buf);
  totalprice->setText(buf);

  okbutton->enable();
  cancelsalebutton->enable();
  editmember->enable();
  discountsp->enable();
  calcchangebutton->enable();
}

void
CashingFrame::setSession(int session,FXbool forcashing)
{
  CCL_log_search_rules sr;
  CCL_log_session_entry *se = NULL;
  int num = 0;

  sr.id = csession = session;
  sr.rulemask = CCL_SR_ID;
  num = CCL_log_sessions_get(&sr,&se);
  clear();
  if (num < 1)
    return;

  csession = session;
  cprice = se[0].price;
  cclient = se[0].client;
  cmember = se[0].member;

  char buf[256];
  int time = se[0].time;
  double owed_terminal = cprice / 100.0;
  double owed_products = 0.0;
  const char *cname = CCL_client_name_get(cclient);
  const char *mname = CCL_member_exists(se[0].member)
			? CCL_member_name_get(se[0].member) : NULL;

  clientname->setText(cname);
  if (mname)
    clientname->setText(clientname->getText() + ":" + mname);
  snprintf(buf,64,"%.2d:%.2d:%.2d",
	   time / 3600,(time % 3600) / 60,(time % 3600) % 60);
  totaltime->setText(buf);

  // Fill Products List
  char *category;
  char *name;
  int id;
  unsigned int amount;
  CCL_log_product_entry *pe = NULL;

  sr.rulemask = CCL_SR_SESSION;
  sr.session = session;
  num = CCL_log_products_get(&sr,&pe);

  for (int i = 0; i < num; i++) {
    owed_products += pe[i].price / 100.0;
    if (CCL_product_info_get(pe[i].product,&category,&name,NULL)) {
      snprintf(buf,256,"%s::%s\t%u\t%.2f",category,name,
	       pe[i].amount,pe[i].price / 100.0);
      clprodlist->prependItem(NULL,buf,NULL,NULL,NULL);
      CCL_free(category);
      CCL_free(name);
    }
  }
  CCL_free(pe);
  cpprice = (FXuint) (owed_products * 100);

  snprintf(buf,64,"%.2f",owed_terminal);
  timeprice->setText(buf);
  snprintf(buf,64,"%.2f",owed_products);
  productsprice->setText(buf);
  snprintf(buf,64,"%.2f",owed_terminal + owed_products);
  totalprice->setText(buf);

  // Fill Intervals List
  time_t *intervals = NULL;
  int inum = CCL_log_session_intervals_get(csession,&intervals);

  for (int i = 0; i < inum; i++) {
    time_t stime = intervals[i * 2];
    time_t etime = intervals[i * 2 + 1];
    char ststr[64];

    strftime(ststr,64,"%d/%m/%Y  %H:%M:%S",localtime(&stime));
    snprintf(buf,256,"%s\t%.2d:%.2d:%.2d",ststr,(etime - stime) / 3600,
	     ((etime - stime) % 3600) / 60,((etime - stime) % 3600) % 60);
    timeintervals->prependItem(NULL,buf,NULL,NULL,NULL);
  }
  CCL_free(intervals);
  // Enable buttons
  if (forcashing) {
    okbutton->enable();
    cancelbutton->enable();
    editmember->enable();
    editprice->enable();
    discountsp->enable();
    calcchangebutton->enable();
  }
  CCL_free(se);
}

void
CashingFrame::clear()
{
  csession = -1;
  cprice = 0;
  cpprice = 0;
  discount = 0.0;
  clientname->setText("-----");
  timeprice->setText("-.--");
  productsprice->setText("-.--");
  totalprice->setText("-.--");
  clprodlist->clearItems();
  timeintervals->clearItems();
  totaltime->setText("--:--");
  okbutton->disable();
  cancelbutton->disable();
  cancelsalebutton->disable();
  editmember->disable();
  editprice->disable();
  discountsp->disable();
  discountsp->setValue(0.0);
  calcchangebutton->disable();

  products = NULL;
}

FXString
CashingFrame::generateSessionTicket(double taxpercent,double &owed_products,
				    double &owed_terminal)
{
  FXString tickettext = "";
  CCL_log_search_rules sr;
  CCL_log_session_entry *se = NULL;
  int num = 0;

  sr.id = csession;
  sr.rulemask = CCL_SR_ID;
  num = CCL_log_sessions_get(&sr,&se);
  if (num < 1)
    return "";

  cprice = se[0].price;
  cclient = se[0].client;
  cmember = se[0].member;

  char buf[256];
  int time = se[0].time;
  const char *cname = CCL_client_name_get(cclient);

  owed_terminal = (cprice * (100.0 / (100.0 - discount))) / 100.0;

  tickettext += _("Client:");
  tickettext += " ";
  tickettext += cname;
  tickettext += "\n";
  snprintf(buf,64,"%.2d:%.2d:%.2d",
	   time / 3600,(time % 3600) / 60,(time % 3600) % 60);

  tickettext += _("Total time:");
  tickettext += " ";
  tickettext += buf;
  tickettext += "\n\n";

  // Fill Products List
  char *category;
  char *name;
  int id;
  unsigned int amount;
  unsigned int realprice;
  CCL_log_product_entry *pe = NULL;

  sr.rulemask = CCL_SR_SESSION;
  sr.session = csession;
  num = CCL_log_products_get(&sr,&pe);

  for (int i = 0; i < num; i++) {
    realprice = pe[i].price + CCL_data_get_int(CCL_DATA_LOGPRODUCT,pe[i].id,
					       "discount",0);
    owed_products += realprice / 100.0;
    if (CCL_product_info_get(pe[i].product,&category,&name,NULL)) {
      tickettext += category;
      tickettext += "\n";
      snprintf(buf,256,"%-*s",20,name);
      tickettext += buf;
      snprintf(buf,256," %3u   $%6.2f",pe[i].amount,
	       minusTax(realprice,taxpercent) / 100.0);
      tickettext += buf;
      tickettext += "\n";
      CCL_free(category);
      CCL_free(name);
    }
  }
  CCL_free(pe);
  cpprice = (FXuint) (owed_products * 100);

  tickettext += "----------\n";
  snprintf(buf,64,"$ %.2f",minusTax(owed_products,taxpercent));
  tickettext += _("Products:");
  tickettext += " ";
  tickettext += buf;
  tickettext += "\n\n";
  
  snprintf(buf,64,"$ %.2f",minusTax(owed_terminal,taxpercent));
  tickettext += _("Workstation time:");
  tickettext += " ";
  tickettext += buf;
  tickettext += "\n----------\n";

  CCL_free(se);

  return tickettext;
}

FXString
CashingFrame::generateSaleTicket(double taxpercent,double &owed_products,
				 double &owed_terminal)
{
  FXString tickettext = "";

  char buf[256];
  
  // Fill Products List
  char *category;
  char *name;
  unsigned int price;
  int id;
  unsigned int amount;

  owed_terminal = 0.0;
  
  for (int i = 0; 0 < products[i].id; i++) {
    id = products[i].id;
    amount = products[i].amount;
    price = products[i].price;
    if (CCL_product_info_get(id,&category,&name,NULL)) {
      owed_products += price / 100.0;

      tickettext += category;
      tickettext += "\n";
      snprintf(buf,256,"%-*s",20,name);
      tickettext += buf;
      snprintf(buf,256," %3u   $%6.2f",amount,
	       minusTax(price,taxpercent) / 100.0);
      tickettext += buf;
      tickettext += "\n";
      CCL_free(category);
      CCL_free(name);
    }
  }
  
  tickettext += "----------\n";

  return tickettext;
}

void
CashingFrame::printTicket()
{
  if (-1 != csession) {
    char buf[256];
    char *header = CCL_data_get_string(CCL_DATA_NONE,0,"ticket/header",NULL);
    char *footer = CCL_data_get_string(CCL_DATA_NONE,0,"ticket/footer",NULL);
    char *taxname = CCL_data_get_string(CCL_DATA_NONE,0,"ticket/taxname",
					_("Tax"));
    double taxpercent = CCL_data_get_int(CCL_DATA_NONE,0,
					 "ticket/taxpercent",-1) / 100.0;
    int ticketnum = 1 + CCL_data_get_int(CCL_DATA_NONE,0,"ticket/number",0);
//    int last_day = CCL_data_get_int(CCL_DATA_NONE,0,"ticket/last_day",-1);
    double owed_terminal = 0.0;
    double owed_products = 0.0;
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

//    if (-1 == last_day || tm->tm_yday != last_day) {
//      ticketnum = 1;
//      CCL_data_set_int(CCL_DATA_NONE,0,"ticket/last_day",tm->tm_yday);
//    }

    if (ticketnum > 999999)
      ticketnum = 1;

    FXString tickettext = header;
    if (header) {
      tickettext += "\n";
      CCL_free(header);
    }

    snprintf(buf,sizeof(buf)/sizeof(char),"%05d",ticketnum);

    tickettext += _("Ticket number:");
    tickettext += " ";
    tickettext += buf;
    tickettext += "\n";

    strftime(buf,sizeof(buf)/sizeof(char),"%H:%M:%S %d/%m/%Y",tm);

    tickettext += _("Date:");
    tickettext += " ";
    tickettext += buf;
    tickettext += "\n\n";

    const char *mname = CCL_member_exists(cmember)
			? CCL_member_name_get(cmember) : NULL;

    if (mname) {
      tickettext += _("Member:");
      tickettext += " ";
      tickettext += mname;
      tickettext += "\n";
    }

    tickettext += (0 != csession) 
		  ? generateSessionTicket(taxpercent,owed_products,
					  owed_terminal)
		  : generateSaleTicket(taxpercent,owed_products,
				       owed_terminal);

    if (taxpercent > 0.0) {
      snprintf(buf,64,"$ %.2f",minusTax(owed_terminal + owed_products,
					taxpercent));
      tickettext += _("Subtotal:");
      tickettext += " ";
      tickettext += buf;
      tickettext += "\n";
      
      if (discount > 0.001) {
	snprintf(buf,64,"%%%.2f",discount);
	tickettext += _("Discount:");
	tickettext += " ";
	tickettext += buf;
	tickettext += "\n";
      }

      snprintf(buf,64,"$ %.2f",
	       applyDiscount(owed_terminal + owed_products
			     - minusTax(owed_terminal + owed_products,
					taxpercent),
			     discount));
      tickettext += taxname;
      tickettext += ": ";
      tickettext += buf;
      tickettext += "\n";
    }

    snprintf(buf,64,"$ %.2f",
	     applyDiscount(owed_terminal + owed_products,discount));

    tickettext += _("Total price:");
    tickettext += " ";
    tickettext += buf;
    tickettext += "\n";

    CCL_free(taxname);

    if (footer) {
      tickettext += footer;
      CCL_free(footer);
    }
    
    tickettext += "\n";

#ifdef WIN32
    FILE *p = fopen("PRN","w");
    fwrite(tickettext.text(),sizeof(char),tickettext.length(),p);
    fclose(p);
#else
    FXString command = "lpr ";
    FXString filename = FXPath::unique("__ticket.txt");
    FILE *p = fopen(filename.text(),"w");
    
    command += filename;
    fwrite(tickettext.text(),sizeof(char),tickettext.length(),p);
    fclose(p);
    system(command.text());
//    FXFile::remove(filename);
#endif
    
    CCL_data_set_int(CCL_DATA_NONE,0,"ticket/number",ticketnum);
  }
}

long
CashingFrame::onCash(FXObject*,FXSelector,void*)
{
  if (!isPermitted(PERMCASHRECEIVE)){
    noPermInfo();
    return 0;
  }

  if (-1 != csession) {
    if (0 == csession) { //cashing non-client purchase
      for (int i = 0; 0 < products[i].id; i++) {
	int id;
	
	id = CCL_product_sell(products[i].id,products[i].amount,
			      applyDiscount(products[i].price,discount),
			      (discount > 0.001) ? PAID|WITH_DISCOUNT : PAID, e_inf.empID);
	if (discount > 0.001)
	  CCL_data_set_int(CCL_DATA_LOGPRODUCT,id,"discount",
			   products[i].price - applyDiscount(products[i].price,
							     discount));

	productsframe->delProduct(products[i].id);
	productsframe->addProduct(products[i].id);
      }
    } else {  // cashing client purchases and terminal fees
      CCL_client_flags_toggle(cclient,USERSTOP,FALSE);
      mainwin->updateClientIcon(cclient);
      mainwin->unBlockClient(cclient);
      CCL_log_session_set_price(csession,applyDiscount(cprice,discount));
      CCL_log_session_set_flags(csession,
				(discount > 0.001) ? PAID|WITH_DISCOUNT
						   : PAID);
      if (discount > 0.001)
	CCL_data_set_int(CCL_DATA_LOGSESSION,csession,"discount",
			 cprice - applyDiscount(cprice,discount));
      notpaidframe->readNotPaid();

      // Apply discount to the products
      CCL_log_product_entry *pe = NULL;
      CCL_log_search_rules sr;
      int num;

      sr.rulemask = CCL_SR_SESSION;
      sr.session = csession;
      num = CCL_log_products_get(&sr,&pe);

      for (int i = 0; i < num; i++) {
	if (discount > 0.001) {
	  CCL_log_product_set_price(pe[i].id,applyDiscount(pe[i].price,
							   discount));
	  CCL_log_product_set_flags(pe[i].id,PAID|WITH_DISCOUNT);
	  CCL_data_set_int(CCL_DATA_LOGPRODUCT,pe[i].id,"discount",
			   pe[i].price - applyDiscount(pe[i].price,discount));
	} else
	  CCL_log_product_set_flags(pe[i].id,PAID);
      }
      CCL_free(pe);
    }

    //if (printticketcb->getCheck()) printTicket();
    //onSendCode(NULL,0,NULL);
  }

  clear();

  return 1;
}

long
CashingFrame::onCancel(FXObject*,FXSelector,void*)
{
  if (!isPermitted(PERMCASHCANCEL)){
    noPermInfo();
    return 0;
  }

  if (csession != -1) {
    CCL_client_flags_toggle(cclient,USERSTOP,FALSE);
    mainwin->updateClientIcon(cclient);
    mainwin->unBlockClient(cclient);
    CCL_log_session_set_price(csession,cprice);
    CCL_log_session_set_flags(csession,CANCELED);
    notpaidframe->readNotPaid();
  }

  clear();

  return 1;
}

long
CashingFrame::onCancelSale(FXObject*,FXSelector,void*)
{
  if (!isPermitted(PERMCASHRECEIVE)){
    noPermInfo();
    return 0;
  }

  clear();

  return 1;
}

long
CashingFrame::onEditMember(FXObject*,FXSelector,void*)
{
  if (!isPermitted(PERMCASHEDIT)){
    noPermInfo();
    return 0;
  }
  if (csession != -1) {
    int newmember = cmember;

    if (FXInputDialog::getInteger(newmember,this,_("Change member"),
				  _("Enter the member ID (0 for none):"),
				  NULL,0,999999)
	&& (CCL_member_exists(newmember) || newmember == 0)) {
      char buf[256];

      cmember = newmember;
      if (newmember == 0)
	snprintf(buf,256,"%s",CCL_client_name_get(cclient));
      else
	snprintf(buf,256,"%s:%s",CCL_client_name_get(cclient),
		 CCL_member_name_get(newmember));
      clientname->setText(buf);

      // Recalculate the price
      int oldtarif = -1;
      int perminafter = CCL_perminafter_get();
      int permin;
      time_t totaltime = 0;
      time_t *intervals = NULL;
      int inum = CCL_log_session_intervals_get(csession,&intervals);
      FXuint newprice = 0;

      if (newmember != 0 && 0 != CCL_member_tarif_get(newmember)) {
	oldtarif = CCL_tarif_get();
	CCL_tarif_set(CCL_member_tarif_get(newmember));
	perminafter = CCL_perminafter_get();
      }
     
      // Sum the time
      for (int i = 0; i < inum; i++)
	totaltime = intervals[i * 2 + 1] - intervals[i * 2];
      
      permin = ((totaltime / 60) >= perminafter && perminafter != -1);
      
      // Calculate it
      for (int i = 0; i < inum; i++) {
	int stime = intervals[i * 2];
	int etime = intervals[i * 2 + 1];
	double frac;

	if (!permin)
	  frac = (etime - stime) / (double) totaltime;
	else
	  frac = 1.0;
	newprice += (int) (CCL_tarif_calc(stime, etime, permin) * frac);
      }
      
      CCL_free(intervals);

      // Restore the old tarif
      if (1 <= oldtarif)
	CCL_tarif_set(oldtarif);

      // Save it
      CCL_log_session_set_price(csession,newprice);
      CCL_log_session_set_flags(csession,NOTPAID);
      CCL_log_session_set_member(csession,newmember);
      notpaidframe->readNotPaid();
      setSession(csession);
    }
  }

  return 1;
}

long
CashingFrame::onEditPrice(FXObject*,FXSelector,void*)
{
  if (!isPermitted(PERMCASHEDIT)){
    noPermInfo();
    return 0;
  }

  if (csession != -1) {
    double newprice = cprice / 100.0;

    if (FXInputDialog::getReal(newprice,this,_("New price"),
			       _("Enter the new price:"),NULL,0,999999)) {
      char buf[64];
      double newtotalprice = newprice + cpprice / 100.0;

      cprice = (FXuint) (newprice * 100);
      snprintf(buf,64,"%.2f",newprice);
      timeprice->setText(buf);
      snprintf(buf,64,"%.2f",newtotalprice);
      totalprice->setText(buf);
    }
  }

  return 1;
}

long
CashingFrame::onPrintTicketCheck(FXObject*,FXSelector,void* ptr)
{
  FXint print = (0 == (FXlong)ptr) ? 0 : 1;

  getApp()->reg().writeIntEntry("CASHING","print",print);
  getApp()->reg().write();

  return 1;
}

long
CashingFrame::onSendCode(FXObject*,FXSelector,void* ptr)
{
#ifdef WIN32
  FILE *p = fopen("PRN","w");
  char code = 0x07;

  fwrite(&code,sizeof(code),1,p);
  fclose(p);
#endif

  return 1;
}

long
CashingFrame::onDiscount(FXObject*,FXSelector,void *ptr)
{
  char buf[64];
  double newtotalprice;

  if (!isPermitted(PERMCASHDISCOUNT)){
    noPermInfo();
    return 0;
  }

  discount = *(double*)ptr;
  newtotalprice = applyDiscount(cprice + cpprice,discount) / 100.0;

  snprintf(buf,64,"%.2f",newtotalprice);
  totalprice->setText(buf);
 
  return 1;
}

long
CashingFrame::onTicketEdit(FXObject*,FXSelector,void*)
{
  FXDialogBox dlg(this,_("Edit ticket format"));
  FXVerticalFrame *vframe =
    new FXVerticalFrame(&dlg,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,0,0,0,0,0,0);
  new FXLabel(vframe,_("Header:"));
  FXVerticalFrame *htframe =
    new FXVerticalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN,
			0,0,0,0,0,0,0,0);
  FXText *headertext = new FXText(htframe,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(vframe,_("Footer:"));
  FXVerticalFrame *ftframe =
    new FXVerticalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN,
			0,0,0,0,0,0,0,0);
  FXText *footertext = new FXText(ftframe,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXHorizontalFrame *hframe1 =
    new FXHorizontalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(hframe1,_("Tax name:"));
  FXTextField *taxnametf = new FXTextField(hframe1,30);
  FXHorizontalFrame *hframe2 =
    new FXHorizontalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(hframe2,_("Tax %:"));
  FXRealSpinner *taxpercentsb =
    new FXRealSpinner(hframe2,4,NULL,0,REALSPIN_NORMAL|FRAME_SUNKEN);
  taxpercentsb->setRange(0.0,99.9);
  FXHorizontalFrame *hframe3 =
    new FXHorizontalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXButton(hframe3,_("Accept"),dbIcon1,&dlg,FXDialogBox::ID_ACCEPT,
	       FRAME_RAISED|FRAME_THICK);
  new FXButton(hframe3,_("Cancel"),dbIcon1,&dlg,FXDialogBox::ID_CANCEL,
	       FRAME_RAISED|FRAME_THICK);

  char *header = CCL_data_get_string(CCL_DATA_NONE,0,"ticket/header",NULL);
  char *footer = CCL_data_get_string(CCL_DATA_NONE,0,"ticket/footer",NULL);
  char *taxname = CCL_data_get_string(CCL_DATA_NONE,0,"ticket/taxname",NULL);
  double taxpercent = CCL_data_get_int(CCL_DATA_NONE,0,
				       "ticket/taxpercent",0) / 100;

  if (header) {
    headertext->setText(header);
    CCL_free(header);
  }
  if (footer) {
    footertext->setText(footer);
    CCL_free(footer);
  }
  if (taxname) {
    taxnametf->setText(taxname);
    CCL_free(taxname);
  }
  taxpercentsb->setValue(taxpercent);

  if (dlg.execute()) {
    FXString ht = headertext->getText();
    FXString ft = footertext->getText();
    FXString tn = taxnametf->getText();
    int tp = int(taxpercentsb->getValue() * 100);

    if (ht.length())
      CCL_data_set_string(CCL_DATA_NONE,0,"ticket/header",ht.text());
    else
      CCL_data_key_delete(CCL_DATA_NONE,0,"ticket/header");
    if (ft.length())
      CCL_data_set_string(CCL_DATA_NONE,0,"ticket/footer",ft.text());
    else
      CCL_data_key_delete(CCL_DATA_NONE,0,"ticket/footer");
    if (tn.length())
      CCL_data_set_string(CCL_DATA_NONE,0,"ticket/taxname",tn.text());
    else
      CCL_data_key_delete(CCL_DATA_NONE,0,"ticket/taxname");
    if (tp > 2)
      CCL_data_set_int(CCL_DATA_NONE,0,"ticket/taxpercent",tp);
    else
      CCL_data_key_delete(CCL_DATA_NONE,0,"ticket/taxpercent");
  }
  
  return 1;
}

long
CashingFrame::onCalcChange(FXObject*,FXSelector,void*)
{
  if (csession != -1) {
    double paid = 0.0;

    if (FXInputDialog::getReal(paid,this,_("Calculate change"),
			       _("The client paid with:"),NULL,0,999999)) {
      char buf[64];
      double topay = applyDiscount((cprice + cpprice) / 100.0,discount);
      double change = paid - topay;
      FXString msg = _("Change:");

      snprintf(buf,64,"%.2f",change);
      msg += " ";
      msg += buf;

      FXMessageBox::information(this,MBOX_OK,_("Change:"),msg.text());
    }
  }

  return 1;
}
