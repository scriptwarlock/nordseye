#include <ccls.h>
#include <fox-1.6/fx.h>
#include <fox-1.6/FXRex.h>
using namespace FX;
using namespace std;

#include "cclfox.h"
#include "ReportFrame.h"
#include "CashingFrame.h"
#include "CCLWin.h"
#include "verifiers.h"

//#define DEBUG 
extern FXGIFIcon *dbIcon01;
extern FXGIFIcon *dbIcon0;
extern FXGIFIcon *dbIcon1;
extern FXGIFIcon *dbIcon2;
extern FXGIFIcon *dbIcon3;

extern char *cybername;
static void printTicket(const char *description, unsigned int amount);
static void openCashRegister();

FXDEFMAP(ReportFrame) ReportFrameMap[] =
{
  FXMAPFUNC(SEL_COMMAND,ReportFrame::ID_REFRESH,ReportFrame::onRefresh),
  FXMAPFUNC(SEL_COMMAND,ReportFrame::ID_CLEAR,ReportFrame::onClear),
  FXMAPFUNC(SEL_COMMAND,ReportFrame::ID_CHECKVALID,ReportFrame::onCheckValid),
  FXMAPFUNC(SEL_COMMAND,ReportFrame::ID_RESET,ReportFrame::onReset),
  FXMAPFUNC(SEL_COMMAND,ReportFrame::ID_SESSIONS,ReportFrame::onSwitchToSessions),
  FXMAPFUNC(SEL_COMMAND,ReportFrame::ID_PRODUCTS,ReportFrame::onSwitchToProducts),
  FXMAPFUNC(SEL_COMMAND,ReportFrame::ID_EXPENSES,ReportFrame::onSwitchToExpenses),
  FXMAPFUNC(SEL_COMMAND,ReportFrame::ID_LOGEXPENSE,ReportFrame::onLogExpense),
  FXMAPFUNC(SEL_COMMAND,ReportFrame::ID_SAVEREPORT,ReportFrame::onSaveReport),
  FXMAPFUNC(SEL_VERIFY,ReportFrame::ID_CHECKVALID,ReportFrame::onVerify),
  FXMAPFUNC(SEL_SELECTED,ReportFrame::ID_SESSIONLIST,ReportFrame::onSessionSelect),
  FXMAPFUNC(SEL_COMMAND,ReportFrame::ID_STARTCASH,ReportFrame::onStartingCashChange),
  FXMAPFUNC(SEL_COMMAND,ReportFrame::ID_SORTLOG,ReportFrame::onSortLogs)
};

FXIMPLEMENT(ReportFrame,FXVerticalFrame,ReportFrameMap,ARRAYNUMBER(ReportFrameMap))

extern CCLWin *mainwin;
extern CashingFrame *cashingframe;
  
static int
litemSortFunc(const FXFoldingItem * l,const FXFoldingItem * r)
{
  return compare(l->getText(),r->getText());
}

ReportFrame::ReportFrame(FXComposite * parent)
:FXVerticalFrame(parent,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN,
		 0,0,0,0,0,0,0,0,0,0)
{
  FXHorizontalFrame *hframe0 = new FXHorizontalFrame(this,LAYOUT_FILL_X);
  FXHorizontalFrame *hframe1 = new FXHorizontalFrame(this,LAYOUT_FILL_X);

  new FXLabel(hframe0,_("Time:"));
  ttotallbl = new FXLabel(hframe0,"0 Day(s) 00 Hr(s) 00 Min(s)");
  new FXLabel(hframe0,_("     Clients: "));
  ctotallbl = new FXLabel(hframe0,"0");
  new FXLabel(hframe1,_("Sessions: "));
  stotallbl = new FXLabel(hframe1,"0.00");
  new FXLabel(hframe1,_("Products: "));
  ptotallbl = new FXLabel(hframe1,"0.00");
  new FXLabel(hframe1,_("Expenses: "));
  etotallbl = new FXLabel(hframe1,"0.00");
  new FXLabel(hframe1,_("Total: " ));
  totallbl = new FXLabel(hframe1,"0.00");

  FXVerticalFrame *loglistframe =
    new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN,
			0,0,0,0,0,0,0,0,0,0);

  listswitcher = new FXSwitcher(loglistframe,LAYOUT_FILL_X|LAYOUT_FILL_Y,
				0,0,0,0,0,0,0,0);
  // Sessions
  sessionslist = new FXFoldingList(listswitcher,this,ID_SESSIONLIST,
				   FOLDINGLIST_SINGLESELECT|LAYOUT_FILL_X|
				   LAYOUT_FILL_Y);
  sessionslist->appendHeader(_("Station"),NULL,100);
  sessionslist->appendHeader(_("Start"),NULL,100);
  sessionslist->appendHeader(_("End"),NULL,100);
  sessionslist->appendHeader(_("Time"),NULL,55);
  sessionslist->appendHeader(_("Price"),NULL,55);
  sessionslist->appendHeader(_("Discount"),NULL,55);
  sessionslist->appendHeader(_("Staff"),NULL,80);
  sessionslist->setSortFunc(litemSortFunc);
  // Clients
  clientslist = new FXFoldingList(listswitcher,NULL,0,
				   FOLDINGLIST_SINGLESELECT|LAYOUT_FILL_X|
				   LAYOUT_FILL_Y);
  clientslist->appendHeader(_("Station"),NULL,100);
  clientslist->appendHeader(_("Sessions"),NULL,100);
  clientslist->appendHeader(_("Time"),NULL,100);
  clientslist->appendHeader(_("Amount"),NULL,70);
  clientslist->setSortFunc(litemSortFunc);

  // Products
  productslist = new FXFoldingList(listswitcher,NULL,0,
				   FOLDINGLIST_SINGLESELECT|LAYOUT_FILL_X|
				   LAYOUT_FILL_Y);
  productslist->appendHeader(_("Product"),NULL,140);
  productslist->appendHeader(_("Station"),NULL,115);
  productslist->appendHeader(_("Date"),NULL,120);
  productslist->appendHeader(_("Amount"),NULL,60);
  productslist->appendHeader(_("Price"),NULL,55);
  productslist->appendHeader(_("Discount"),NULL,55);
  productslist->appendHeader(_("Staff"),NULL,60);
  productslist->setSortFunc(litemSortFunc);

  // Products Summary
  sumprodlist = new FXFoldingList(listswitcher,NULL,0,
				   FOLDINGLIST_SINGLESELECT|LAYOUT_FILL_X|
				   LAYOUT_FILL_Y);
  sumprodlist->appendHeader(_("Product"),NULL,140);
  sumprodlist->appendHeader(_("Units"),NULL,60);
  sumprodlist->appendHeader(_("Amount"),NULL,55);
  sumprodlist->setSortFunc(litemSortFunc);

  // Expenses
  expenseslist = new FXFoldingList(listswitcher,NULL,0,
				   FOLDINGLIST_SINGLESELECT|LAYOUT_FILL_X|
				   LAYOUT_FILL_Y);
  expenseslist->appendHeader(_("Description"),NULL,260);
  expenseslist->appendHeader(_("Date"),NULL,120);
  expenseslist->appendHeader(_("Spent"),NULL,60);
  expenseslist->appendHeader(_("Staff"),NULL,70);
  expenseslist->setSortFunc(litemSortFunc);

  FXHorizontalFrame *hframe10 =
    new FXHorizontalFrame(this,LAYOUT_FILL_X,0,0,0,0,0,0,0,0,0,0);
  clearbtn = new FXButton(hframe10,_("Clear"),dbIcon3,this,ID_CLEAR,
		            FRAME_RAISED|LAYOUT_CENTER_X|LAYOUT_FILL_X);
  //			  BUTTON_NORMAL|LAYOUT_CENTER_X|LAYOUT_FILL_X);
  refreshbtn = new FXButton(hframe10,_("Refresh"),dbIcon3,this,ID_REFRESH,
		            FRAME_RAISED|LAYOUT_CENTER_X|LAYOUT_FILL_X);
  //			    BUTTON_NORMAL|LAYOUT_CENTER_X|LAYOUT_FILL_X);
  //  logexpensebtn = new FXButton(hframe10,_("Log Expense"),NULL,this,
  //			       ID_LOGEXPENSE,BUTTON_NORMAL|LAYOUT_CENTER_X|
  //			       LAYOUT_FILL_X);

  sesssumbtn = new FXButton(hframe10,_("Sessions"),dbIcon3,this,ID_SESSIONS,
		            FRAME_RAISED|LAYOUT_CENTER_X|LAYOUT_FILL_X);
  //			    BUTTON_NORMAL|LAYOUT_CENTER_X| LAYOUT_FILL_X);

  prodsumbtn = new FXButton(hframe10,_("Products"),dbIcon3,this,ID_PRODUCTS,
			    FRAME_RAISED|LAYOUT_CENTER_X|LAYOUT_FILL_X);
  //			    BUTTON_NORMAL|LAYOUT_CENTER_X| LAYOUT_FILL_X);

  //  expensebtn = new FXButton(hframe10,_("Expenses"),NULL,this,
  //			       ID_EXPENSES,BUTTON_NORMAL|LAYOUT_CENTER_X|
  //			       LAYOUT_FILL_X);
  saverptbtn = new FXButton(hframe10,_("Save Report"),dbIcon3,this, ID_SAVEREPORT,
			    FRAME_RAISED|LAYOUT_CENTER_X|LAYOUT_FILL_X);
  //			    BUTTON_NORMAL|LAYOUT_CENTER_X| LAYOUT_FILL_X);
  FXVerticalFrame *vframe1 = new FXVerticalFrame(this,LAYOUT_FILL_X);
  FXHorizontalFrame *hframe2 = new FXHorizontalFrame(vframe1,LAYOUT_FILL_X);
  new FXLabel(hframe2,_("From:"));
  stimetf = new FXTextField(hframe2,5,this,ID_CHECKVALID,
			    TEXTFIELD_NORMAL|TEXTFIELD_LIMITED);
  new FXLabel(hframe2,_("of"));

  sdatetf = new FXTextField(hframe2,8,this,ID_CHECKVALID,
			    TEXTFIELD_NORMAL|TEXTFIELD_LIMITED);
  membertf = new FXTextField(hframe2,6,NULL,0,
			     LAYOUT_RIGHT|TEXTFIELD_NORMAL|TEXTFIELD_INTEGER);
  new FXLabel(hframe2,_("Member:"),NULL,LABEL_NORMAL|LAYOUT_RIGHT);
  stafftf = new FXTextField(hframe2,10,NULL,0,
			     LAYOUT_RIGHT|TEXTFIELD_NORMAL);
  new FXLabel(hframe2,_("Staff:"),NULL,LABEL_NORMAL|LAYOUT_RIGHT);
  FXHorizontalFrame *hframe3 = new FXHorizontalFrame(vframe1,LAYOUT_FILL_X);
  new FXLabel(hframe3,_("    To:"));
  etimetf = new FXTextField(hframe3,5,this,ID_CHECKVALID,
			    TEXTFIELD_NORMAL|TEXTFIELD_LIMITED);
  new FXLabel(hframe3,_("of"));
  edatetf = new FXTextField(hframe3,8,this,ID_CHECKVALID,
			    TEXTFIELD_NORMAL|TEXTFIELD_LIMITED);
  canceledcheck = new FXCheckButton(hframe3,_("Canceled"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_RIGHT);
  summarycheck = new FXCheckButton(hframe3,_("Summary"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_RIGHT);
  detailcheck = new FXCheckButton(hframe3,_("Details"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_RIGHT);
  FXHorizontalFrame *hframe4 = new FXHorizontalFrame(vframe1,LAYOUT_FILL_X);
  new FXLabel(hframe4,_("Days:"));
  daybtn[0] = new FXToggleButton(hframe4,_("Sun"),_("Sun"),NULL,NULL,this,
				 ID_CHECKVALID, TOGGLEBUTTON_NORMAL|
				 TOGGLEBUTTON_KEEPSTATE);
  daybtn[1] =
    new FXToggleButton(hframe4,_("Mon"),_("Mon"),NULL,NULL,this,ID_CHECKVALID,
		       TOGGLEBUTTON_NORMAL|TOGGLEBUTTON_KEEPSTATE);
  daybtn[2] =
    new FXToggleButton(hframe4,_("Tue"),_("Tue"),NULL,NULL,this,ID_CHECKVALID,
		       TOGGLEBUTTON_NORMAL|TOGGLEBUTTON_KEEPSTATE);
  daybtn[3] =
    new FXToggleButton(hframe4,_("Wed"),_("Wed"),NULL,NULL,this,ID_CHECKVALID,
		       TOGGLEBUTTON_NORMAL|TOGGLEBUTTON_KEEPSTATE);
  daybtn[4] =
    new FXToggleButton(hframe4,_("Thu"),_("Thu"),NULL,NULL,this,ID_CHECKVALID,
		       TOGGLEBUTTON_NORMAL|TOGGLEBUTTON_KEEPSTATE);
  daybtn[5] =
    new FXToggleButton(hframe4,_("Fri"),_("Fri"),NULL,NULL,this,ID_CHECKVALID,
		       TOGGLEBUTTON_NORMAL|TOGGLEBUTTON_KEEPSTATE);
  daybtn[6] =
    new FXToggleButton(hframe4,_("Sat"),_("Sat"),NULL,NULL,this,ID_CHECKVALID,
		       TOGGLEBUTTON_NORMAL|TOGGLEBUTTON_KEEPSTATE);
  FXHorizontalFrame *hframe5 = new FXHorizontalFrame(vframe1,LAYOUT_FILL_X);
  new FXLabel(hframe5,_("Between:"));
  strangetf = new FXTextField(hframe5,5,this,ID_CHECKVALID,
			      TEXTFIELD_NORMAL|TEXTFIELD_LIMITED);
  new FXLabel(hframe5,_("and"));
  etrangetf = new FXTextField(hframe5,5,this,ID_CHECKVALID,
			      TEXTFIELD_NORMAL|TEXTFIELD_LIMITED);
  // canceledcheck = new FXCheckButton(hframe5,_("Canceled"),NULL,0,
  //			    CHECKBUTTON_NORMAL|LAYOUT_RIGHT);
  resetbtn = new FXButton(hframe5,_("  Reset  "),dbIcon2,this,ID_RESET,
		          FRAME_RAISED|LAYOUT_TOP|LAYOUT_RIGHT);
			  //BUTTON_NORMAL|LAYOUT_RIGHT);
  //savebtn = new FXButton(hframe5,_("  Save  "),NULL,this,ID_SAVE,
  //			  BUTTON_NORMAL|LAYOUT_RIGHT);
  //new FXLabel(hframe12,_("Start:"));
  //startcashsp = new FXRealSpinner(hframe12,6,this,ID_STARTCASH,
  //				  FRAME_SUNKEN|FRAME_THICK);
  //startcashsp->setRange(0,9999999);
  //startcashsp->setValue(CCL_data_get_int(CCL_DATA_NONE,0,
  //					 "report/starting_cash",0) / 100.0);
  reset();
}

ReportFrame::~ReportFrame()
{
}

void
ReportFrame::create()
{
  FXVerticalFrame::create();
}

void 
ReportFrame::setPerms(long perm)
{
  clear();
  if (!isPermitted(PERMLOGSUMVIEW)){
    summarycheck->setCheck(FALSE);
    summarycheck->disable();
  }
  else{
    summarycheck->enable();
  }

  if (!isPermitted(PERMLOGSUMSAVE)){
    saverptbtn->disable();
  }
  else{
    saverptbtn->enable();
  }
}

void
ReportFrame::reset()
{
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  char buf[64]; 

  stimetf->setText("00:00");
  etimetf->setText("23:59");
  strangetf->setText("00:00");
  etrangetf->setText("23:59");
  strftime(buf,64,"%d/%m/%y",tm);
  //sdatetf->setText("01/01/07");
  sdatetf->setText(buf);
  strftime(buf,64,"%d/%m/%y",tm);
  //edatetf->setText("01/01/17");
  edatetf->setText(buf);
  membertf->setText("");
  stafftf->setText("");
  for (int i = 0; i < 7; i++)
    daybtn[i]->setState(TRUE);
  refreshbtn->enable();
}

void
ReportFrame::clear()
{
  stotal = ptotal = etotal = 0.0;
  sessionslist->clearItems();
  productslist->clearItems();
  expenseslist->clearItems();
  clientslist->clearItems();
  sumprodlist->clearItems();
  totallbl->setText("0.00");
  ptotallbl->setText("0.00");
  stotallbl->setText("0.00");
  etotallbl->setText("0.00");
  ttotallbl->setText("00:00");
}

long
ReportFrame::onSortLogs(FXObject*,FXSelector,void* ptr)
{
  
}

int
ReportFrame::getLogQuery(CCL_log_search_rules *sr)
{
  FXuint strangehr;
  FXuint strangemin;
  FXuint etrangehr;
  FXuint etrangemin;
  FXuint days;
  int num = 0;
  struct tm st;
  struct tm et;
  char empname[64];
  int member = 0, empid = 0;


  memset(sr,0,sizeof(CCL_log_search_rules));
  memset(&st,0,sizeof(struct tm));
  memset(&et,0,sizeof(struct tm));
  memset(empname,0, 64);
  sscanf(stimetf->getText().text(),"%d:%d",&st.tm_hour,&st.tm_min);
  sscanf(sdatetf->getText().text(),"%d/%d/%d",&st.tm_mday,&st.tm_mon,
	 &st.tm_year);
  sscanf(strangetf->getText().text(),"%d:%d",&strangehr,&strangemin);
  st.tm_year += 100;
  st.tm_mon -= 1;
  sscanf(etrangetf->getText().text(),"%d:%d",&etrangehr,&etrangemin);
  sscanf(etimetf->getText().text(),"%d:%d",&et.tm_hour,&et.tm_min);
  sscanf(edatetf->getText().text(),"%d/%d/%d",&et.tm_mday,&et.tm_mon,
	 &et.tm_year);
  et.tm_mon -= 1;
  et.tm_year += 100;
  sscanf(membertf->getText().text(),"%d",&member);
  if (member < 1)
    member = 0;
  sscanf(stafftf->getText().text(),"%s", empname);

  for (int i = 0; i < 7; i++)
    if (daybtn[i]->getState())
      sr->days |= (1 << i);

  sr->etime_min = mktime(&st);
  sr->etime_max = mktime(&et);
  sr->time_min = mktime(&st);
  sr->time_max = mktime(&et);
  if (-1 == sr->etime_min) {
    FXMessageBox::error(this,MBOX_OK,_("Error"),_("Invalid start date"));
    return -1;
  } else if (-1 == sr->etime_max) {
    FXMessageBox::error(this,MBOX_OK,_("Error"),_("Invalid end date"));
    return -1;
  } else if (sr->etime_min > sr->etime_max) {
    FXMessageBox::error(this,MBOX_OK,_("Error"),
			_("Start date greater than end date"));
    return -1;
  }

  sr->daytime_min = strangehr * 60 * 60 + strangemin * 60;
  sr->daytime_max = etrangehr * 60 * 60 + etrangemin * 60 + 59;
  if (canceledcheck->getCheck())
    sr->flags = CANCELED;
  else
    sr->flags = PAID;
  sr->rulemask = CCL_SR_ETIMEMIN|CCL_SR_ETIMEMAX|CCL_SR_DAYTIME_RANGE
		|CCL_SR_DAYS|CCL_SR_FLAGS;
  sr->member = member;
  if (member > 0)
    sr->rulemask |= CCL_SR_MEMBER;
  //employee
  sr->employee = 0;
  if (strlen(empname)>0){
    empid = atoi(empname);
    if (empid<=0) //the employee id is not a number
      empid = CCL_data_find_by_key_sval(CCL_DATA_EMPLOYEE, 
				    "username", (char *)empname);
    sr->rulemask |= CCL_SR_EMPLOYEE;
    sr->employee = empid;
#ifdef DEBUG
    printf("getLogQuery(): Employee: ID = %d: Name: %s\n", empid, empname);
#endif
  }

  return 0;
}

void 
ReportFrame::readSessionLogs()
{
  CCL_log_search_rules sr;
  CCL_log_session_entry *se = NULL;
  int num;

  //read input and create rules
  if (getLogQuery(&sr)<0)
    return;
  num = CCL_log_sessions_get(&sr,&se);
#ifdef DEBUG
  printf("readSessionLogs(): [num=%d] Rules[%08X]\n", num, sr.rulemask);
#endif
  for (int i = 0; i < num; i++) {
    char entry[512];
    char ststr[64];
    char etstr[64];
    unsigned int discount = 0;
    const char *cname = CCL_client_name_get(se[i].client);
    const char *mname = CCL_member_exists(se[i].member)
      ? CCL_member_name_get(se[i].member) : "";
    const char *ename;
    if (se[i].employee>0)
      ename = CCL_employee_usrname_get(se[i].employee);
    else
      ename = "";
    
    if (!mname) mname = "";
    if (se[i].flags & WITH_DISCOUNT)
      discount = CCL_data_get_int(CCL_DATA_LOGSESSION, se[i].id,
				  "discount", 0);
    
    strftime(ststr,64,"%d/%m/%y  %H:%M",localtime(&(se[i].stime)));
    strftime(etstr,64,"%d/%m/%y  %H:%M",localtime(&(se[i].etime)));
    snprintf(entry,512,"%s:%s\t%s\t%s\t%.2d:%.2d\t%6.2f\t%.2f\t%s",
	     cname,mname,ststr,etstr,
	     se[i].time / 3600,(se[i].time % 3600) / 60,se[i].price / 100.0,
	     discount / 100.0, ename);
    sessionslist->prependItem(NULL,entry,NULL,NULL,
			      (void *)(se[i].id));
  }
  CCL_free(se);
}

void 
ReportFrame::readSessionSummary()
{
  CCL_log_search_rules sr;
  CCL_log_session_entry *se = NULL;
  int num, client;
  const char *cname;

  ctotal = 0;
  ttotal = 0;
  stotal = 0;
  if (getLogQuery(&sr)<0)
    return;
  sr.rulemask |= CCL_SR_CLIENT;
  for (FXuint j = 0; -1 != (client = CCL_client_get_nth(j)); j++){
    sr.client = client;
    se = NULL;
    num = CCL_log_sessions_get(&sr,&se);
#ifdef DEBUG
    printf("readSessionSummary(): Logs for Client [%d] -> %d\n", client, num);
#endif
    ctotal += num;
    if (num > 0){  // Do the summary
      long   tottime=0;
      double totprice=0., cltotprice;
      char entry[512];
      cname = CCL_client_name_get(se[0].client);

      if (detailcheck->getCheck()){
	snprintf(entry,512,"Client: %s\t \t \t ", cname);
	//clientslist->prependItem(NULL,entry,NULL,NULL, (void *)(se[0].id));
	clientslist->appendItem(NULL,entry,NULL,NULL, (void *)(se[0].id));
	cltotprice = 0.;
      }
      for (int i = 0; i < num; i++) {
	tottime += se[i].time;
	totprice += se[i].price;

	if (detailcheck->getCheck()){
	  char ststr[64], etstr[64];
	  strftime(ststr,64,"%d/%m/%y %H:%M",localtime(&(se[i].stime)));
	  strftime(etstr,64,"%d/%m/%y %H:%M",localtime(&(se[i].etime)));
	  //snprintf(entry,512," \t%u\t%.2dh %.2dm\t%6.2f",
	  //   i+1, (se[i].time)/3600, (se[i].time % 3600)/60, se[i].price/100.);
	  snprintf(entry,512," \t%s - %s\t%.2dh %.2dm\t%6.2f",
		   ststr, etstr, (se[i].time)/3600, (se[i].time % 3600)/60, se[i].price/100.);
	  //clientslist->prependItem(NULL,entry,NULL,NULL, (void *)(se[0].id));
	  clientslist->appendItem(NULL,entry,NULL,NULL, (void *)(se[0].id));
	}
      }
      snprintf(entry,512,"Total: %s\t%u\t%.2dh %.2dm\t%6.2f",
	       cname, num, tottime/3600, (tottime % 3600)/60, totprice/100);
      //clientslist->prependItem(NULL,entry,NULL,NULL, (void *)(se[0].id));
      clientslist->appendItem(NULL,entry,NULL,NULL, (void *)(se[0].id));
	
      stotal += totprice / 100.0;
      ttotal += tottime / 60;
    }
    CCL_free(se);
  }
  //clientslist->sortItems();
}

void 
ReportFrame::readProductLogs()
{
  CCL_log_search_rules sr;
  CCL_log_product_entry *pe = NULL;
  int num;

  //read input and create rules
  if (getLogQuery(&sr)<0)
    return;
  sr.rulemask |= CCL_SR_TIMEMIN;
  sr.rulemask |= CCL_SR_TIMEMAX;
  num = CCL_log_products_get(&sr,&pe);
#ifdef DEBUG
    printf("readProductLogs():  [num = %d]\n", num);
#endif
  for (int i = 0; i < num; i++) {
    char entry[512];
    char tstr[64];
    unsigned int discount = 0;
    char *pname = NULL;

    const char *cname = CCL_client_exists(pe[i].client)
      ? CCL_client_name_get(pe[i].client) : "";
    const char *mname = CCL_member_exists(pe[i].member)
      ? CCL_member_name_get(pe[i].member) : "";
    const char *ename;
    if (pe[i].employee>0)
      ename = CCL_employee_usrname_get(pe[i].employee);
    else
      ename = "";

    CCL_product_info_get(pe[i].product,NULL,&pname,NULL);
    if (!cname)  cname = "-";
    if (pe[i].flags & WITH_DISCOUNT)
      discount = CCL_data_get_int(CCL_DATA_LOGPRODUCT, pe[i].id,
				  "discount", 0);
    strftime(tstr,64,"%d/%m/%y %H:%M",localtime(&(pe[i].time)));
    snprintf(entry,512,"%d-%s\t%s:%s\t%s\t%u\t%.2f\t%6.2f\t%s",pe[i].product,
	     pname,cname,mname,tstr,pe[i].amount,pe[i].price / 100.0,
	     discount / 100.0, ename);
    productslist->prependItem(NULL,entry,NULL,NULL,NULL);
    CCL_free(pname);
  }    
  CCL_free(pe);
}

void 
ReportFrame::readProductSummary()
{
  CCL_log_search_rules sr;
  CCL_log_product_entry *pe = NULL;
  int num, prod;
  char *pname = NULL;

  ptotal = 0;
  //read input and create rules
  if (getLogQuery(&sr)<0)
    return;
  sr.rulemask |= CCL_SR_TIMEMIN;
  sr.rulemask |= CCL_SR_TIMEMAX;
  sr.rulemask |= CCL_SR_PRODUCT;
  for (FXuint j = 0; -1 != (prod = CCL_product_get_nth(j)); j++){
    pe = NULL;
    sr.product = prod;
    num = CCL_log_products_get(&sr, &pe);
#ifdef DEBUG
    printf("readProductSummary(): Logs for Product [%d] -> %d\n", prod, num);
#endif
    if (num > 0){
      double totprice=0., totdisc=0.;
      char entry[512];
      unsigned int nr=0;
      
      CCL_product_info_get(pe[0].product,NULL,&pname,NULL);
      for (int i=0; i<num; i++){
	totprice += pe[i].price;
	nr += pe[i].amount;
      }
      snprintf(entry, 512, "%s\t%u\t%6.2f", pname, nr, totprice/100);
      sumprodlist->prependItem(NULL,entry,NULL,NULL, (void *)(pe[0].id));
      ptotal += totprice / 100.0;
    }
    CCL_free(pe);
  }
  sumprodlist->sortItems();
}

void 
ReportFrame::readExpenseLogs()
{
  CCL_log_search_rules sr;
  CCL_log_expense_entry *ee = NULL;
  int num, expense;
  char *pname = NULL;

  etotal = 0;
  //read input and create rules
  if (getLogQuery(&sr)<0)
    return;
  num = CCL_log_expenses_get(&sr,&ee);
  for (int i = 0; i < num; i++) {
    char entry[512];
    char tstr[64];

    strftime(tstr,64,"%H:%M  %d/%m/%Y",localtime(&(ee[i].time)));
    snprintf(entry,512,"%s\t%s\t%.2f",
	     ee[i].description,tstr,ee[i].cash / 100.0);
    expenseslist->prependItem(NULL,entry,NULL,NULL,NULL);

    etotal += ee[i].cash / 100.0;
  }
  CCL_free(ee);
}

void
ReportFrame::readLog()
{
  FXuint ttotal = 0;


}

void
ReportFrame::displaySummary()
{
  char buf[64];

  snprintf(buf,64,"%.2f",stotal);
  stotallbl->setText(buf);
  snprintf(buf,64,"%.2f",ptotal);
  ptotallbl->setText(buf);
  snprintf(buf,64,"%.2f",etotal);
  etotallbl->setText(buf);
  snprintf(buf,64,"%.2u Hr(s) %.2u Min(s) ",ttotal / 60, ttotal % 60);
  ttotallbl->setText(buf);
  snprintf(buf,64,"%d", ctotal);
  ctotallbl->setText(buf);
  //snprintf(buf,64,"%.2f",startcashsp->getValue() + stotal + ptotal - etotal);
  snprintf(buf,64,"%.2f", stotal + ptotal - etotal);
  totallbl->setText(buf);
}

void
ReportFrame::clearSummary()
{
  char buf[64];

  ttotal = 0;
  stotallbl->setText("");
  ptotallbl->setText("");
  etotallbl->setText("");
  snprintf(buf,64,"%u Day(s) %.2u Hr(s) %.2u Min(s)",ttotal / 1440,(ttotal % 1440) / 60,
	   ttotal % 60);
  ttotallbl->setText(buf);
  ctotallbl->setText("");
  totallbl->setText("");
}

bool
ReportFrame::saveReport(const char *filename)
{
  FXString output = "";
  FXFoldingList *lists[] = { clientslist, 
			     sumprodlist, 
			     NULL };

  FXString lhdr[] = {
    "<table border=1 class=TTableTitle><th>Sessions Summary<th>Sessions<th>Time<th>Amount",
    "<table border=1 class=TTableTitle><th>Products Summary<th>Quantity<th>Amount"
  };
  int j=0, retval;
  char buf[64];

  clientslist->clearItems();
  readSessionSummary();
  sumprodlist->clearItems();
  readProductSummary();
  displaySummary();
  memset(buf, 0, 64); 
  output = "<html>\n <head><title>Unwire Technologies Cyber Manager Report</title>";
  output += "<style>\n.TTableLargeTitle {\n";
  output += "FONT-SIZE: 16px; FONT-WEIGHT: bold; COLOR: #081640; FONT-FAMILY: Georgia, Verdana, Arial, sans-serif\n}\n";
  output += ".TTableTitle {\n";
  output += "FONT-SIZE: 12px; COLOR: #081640; FONT-FAMILY: Georgia, Verdana, Arial, sans-serif\n}\n";
  output += ".TTableTitleBold {\n";
  output += "FONT-SIZE: 12px; FONT-WEIGHT: bold; COLOR: #081640; FONT-FAMILY: Georgia, Verdana, Arial, sans-serif\n}\n";
  output += "</style>\n</head>\n<body>\n";
  output += "<center>\n";
  output += "<a class=TTableLargeTitle>" + FXString(cybername) + "</a>\n";
  if (detailcheck->getCheck())
    //output += "<h6><br></h6><h3>Detailed Report</h3>\n";
    output += "<h3>Detailed Report</h3>\n";
  else
    output += "<br><h3>Summary Report</h3><br>\n";
  //snprintf(buf,64,"%.2f",startcashsp->getValue());
  output += "<table border=1 class=TTableTitle>\n <tr><td>From: <td><b>" 
          + sdatetf->getText() + " " + stimetf->getText() + "</b></td>"
	  + "<td>To: <td><b>" + edatetf->getText() + " " + etimetf->getText() 
          + "</b></td></tr></table><br>"
          + "<table border=1 class=TTableTitleBold>\n" 
	  + "\n <tr><td>Total Time: <td>" + ttotallbl->getText() + "</td></tr>"
	  + "\n <tr><td>No. of Clients: <td>" + ctotallbl->getText() + "</td></tr>" 
	  + "\n <tr><td>Sessions [Cash]: <td>" + stotallbl->getText() + "</td></tr>"
	  + "\n <tr><td>Products [Cash]: <td>" + ptotallbl->getText() + "</td></tr>"
          + "\n <tr><td>Total: <td>" + totallbl->getText() + "</td>\n </tr>\n</table>"
    	  + buf + "<br><br>\n";
  
  for (FXFoldingList **list = lists; NULL != *list; list++) {
    FXString lnstr;
    output += lhdr[j] + "\n <tr>";
    for (FXFoldingItem *i = (*list)->getFirstItem(); NULL != i;
	 i = i->getNext()) {
      lnstr = "\n <tr><td>" + i->getText();
      lnstr.substitute("\t","<td>",TRUE);
      if (detailcheck->getCheck()){
	if (lnstr.find("Total", 0) >= 0)//bold the Total
	  lnstr.substitute("Total", "<b>Total", FALSE);
	if (lnstr.find("Client", 0) >= 0)//bold the Total
	  lnstr.substitute("Client", "<b>Client", FALSE);
      }
      output += lnstr;
      output += "</td></tr>";
    }
    output += "\n</table>";
    j++;
  }
  
  FILE *out = fopen(filename,"w");

  if (out) {
    char *buf = "\n\nPowered by Mkahawa.Net";

    fwrite(output.text(),sizeof(char),output.length(),out);
    fwrite(buf,sizeof(char),strlen(buf),out);
    fclose(out);

    retval = TRUE;
  } else
    retval = FALSE;

  //clear the lists
  clientslist->clearItems();
  sumprodlist->clearItems();
  clearSummary();

  return retval;
}

void 
ReportFrame::refreshList(enum CURLIST clist)
{

  switch(clist){
  case SESSLIST:
    clearSummary();
    sessionslist->clearItems();
    readSessionLogs();
#ifdef DEBUG
    printf("refreshList(): [curlist=%d] [readSessionLogs]\n", clist);
#endif
    break;
  case SESSSUMLIST:
    clientslist->clearItems();
    readSessionSummary();
    displaySummary();
#ifdef DEBUG
    printf("refreshList(): [curlist=%d] [readSessionSummary]\n", clist);
#endif
    break;
  case PRODLIST:
    clearSummary();
    productslist->clearItems();
    readProductLogs();
#ifdef DEBUG
    printf("refreshList(): [curlist=%d] [readProductLogs]\n", clist);
#endif
    break;
  case PRODSUMLIST:
    sumprodlist->clearItems();
    readProductSummary();
    displaySummary();
#ifdef DEBUG
    printf("refreshList(): [curlist=%d] [readProductSummary]\n", clist);
#endif
    break;
  case EXPENSELIST:
    expenseslist->clearItems();
    readExpenseLogs();
#ifdef DEBUG
    printf("refreshList(): [curlist=%d] [readExpenseLogs]\n", clist);
#endif
    break;
  }
}

long
ReportFrame::onRefresh(FXObject*,FXSelector,void*)
{
  clear();
  refreshList(curlist);
  listswitcher->setCurrent(curlist);
  
  return 1;
}

long
ReportFrame::onClear(FXObject*,FXSelector,void*)
{
  clear();

  return 1;
}

long
ReportFrame::onCheckValid(FXObject*,FXSelector,void*)
{
  FXRex time("\\A[0-2]?[0-9]:[0-5][0-9]\\Z",REX_NORMAL);
  FXRex date("\\A([1-3][0-9]|0?[1-9])/[0-1]?[0-9]/([0-2][0-9]|3[0-7])\\Z",
	     REX_NORMAL);
  FXbool dayset = FALSE;

  for (int i = 0; i < 7; i++)
    if (daybtn[i]->getState())
      dayset = TRUE;

  if (time.match(stimetf->getText()) && date.match(sdatetf->getText())
      && time.match(etimetf->getText()) && date.match(edatetf->getText())
      && time.match(strangetf->getText()) && time.match(etrangetf->getText())
      && dayset)
    refreshbtn->enable();
  else
    refreshbtn->disable();

  return 1;
}

long
ReportFrame::onReset(FXObject*,FXSelector,void*)
{
  reset();

  return 1;
}

long
ReportFrame::onVerify(FXObject* sender,FXSelector,void* ptr)
{
  if (sender == stimetf || sender == etimetf || sender == strangetf
      || sender == etrangetf)
    return !isTime((char *)ptr);
  else if (sender == sdatetf || sender == edatetf)
    return !isDate((char *)ptr);

  return 0;
}

long
ReportFrame::onSessionSelect(FXObject*,FXSelector,void*)
{
  FXFoldingItem *current = sessionslist->getCurrentItem();
  long session = (long) current->getData();

  cashingframe->setSession(session,FALSE);
  mainwin->showCashing();

  return 1;
}

long
ReportFrame::onSwitchToSessions(FXObject*,FXSelector,void*)
{
  enum CURLIST prevlist;

  prevlist = curlist;
  if (summarycheck->getCheck())
    curlist = SESSSUMLIST;
  else
    curlist = SESSLIST;
  if (curlist == prevlist)
    refreshList(curlist);
  else
    listswitcher->setCurrent(curlist);

  return 1;
}

long
ReportFrame::onSwitchToProducts(FXObject*,FXSelector,void*)
{
  enum CURLIST prevlist;

  prevlist = curlist;
  if (summarycheck->getCheck())
    curlist = PRODSUMLIST;
  else
    curlist = PRODLIST;
  if (curlist == prevlist)
    refreshList(curlist);
  else
    listswitcher->setCurrent(curlist);

  return 1;
}

long
ReportFrame::onSwitchToExpenses(FXObject*,FXSelector,void*)
{
  enum CURLIST prevlist;

  prevlist = curlist;
  curlist = EXPENSELIST;
  listswitcher->setCurrent(curlist);
  if (curlist != prevlist)
    listswitcher->setCurrent(curlist);
  else
    refreshList(curlist);

  return 1;
}

long
ReportFrame::onLogExpense(FXObject*,FXSelector,void*)
{
  FXString description = "";
  double cash = 0.0;

  if (FXInputDialog::getString(description,this,_("Description"),
			       _("Description:"))
      && description.length()
      && FXInputDialog::getReal(cash,this,_("Amount"),_("Amount:"))
      && cash > 0.0) {
    CCL_log_expense(description.text(),(FXuint) (cash * 100),0);
    //    printTicket(description.text(),(FXuint) (cash * 100));
    //    openCashRegister();
  }

  return 1;
}

long
ReportFrame::onSaveReport(FXObject*,FXSelector,void*)
{
  FXString cwd = FXSystem::getHomeDirectory() + "/";
  char *logpath = CCL_data_get_string(CCL_DATA_NONE,0,"report/path",cwd.text());
  FXString input = logpath;//cwd.text();
  FXFileDialog *fd;

  CCL_free(logpath);

  fd = new FXFileDialog(this, input);
  {
    FXString filename;
    filename = fd->getSaveFilename(this, _("Save Report File"),
				   input, _("*.rpt.html"), 0);
    FXString directory = fd->getDirectory();
    filename.trim();
    if (!filename.find(".rpt.html"))   
      filename += ".rpt.html";
    if (!FXStat::exists(filename)
	|| FXMessageBox::question(this,MBOX_YES_NO,_("Filename Exists"),
				  _("File '%s' already exists. Overwrite?"),
				  filename.text()) == MBOX_CLICKED_YES) {
      if (saveReport(filename.text())) {
	FXMessageBox::information(this,MBOX_OK,_("Save Report"),
				  _("Report was saved"));
	CCL_data_set_string(CCL_DATA_NONE,0,"report/path",filename.text());
      }
      else
	FXMessageBox::error(this,MBOX_OK,_("Report was not saved"),
			    _("Report not saved"));
    }
  }    
  delete fd;
  return 1;
}

long
ReportFrame::onStartingCashChange(FXObject*,FXSelector,void *ptr)
{
  double value = *(double*)ptr;
  char buf[64];

  CCL_data_set_int(CCL_DATA_NONE,0,"report/starting_cash",(int)(value*100));

  snprintf(buf,64,"%.2f",value + stotal + ptotal - etotal);
  totallbl->setText(buf);

  return 1;
}

static void
printTicket(const char *description, unsigned int amount)
{
  char buf[256];
  int ticketnum = 1 + CCL_data_get_int(CCL_DATA_NONE,0,
				       "ticket/number",0);
//  int last_day = CCL_data_get_int(CCL_DATA_NONE,0,
//				  "ticket/last_day",-1);
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);

//  if (-1 == last_day || tm->tm_yday != last_day) {
//    ticketnum = 1;
//    CCL_data_set_int(CCL_DATA_NONE,0,"ticket/last_day",tm->tm_yday);
//  }

  FXString tickettext = "";

  snprintf(buf,sizeof(buf)/sizeof(char),"%05d",ticketnum);

  tickettext += _("Ticket number:");
  tickettext += " ";
  tickettext += buf;
  tickettext += "\n";

  strftime(buf,sizeof(buf)/sizeof(char),"%T %d/%m/%Y",tm);

  tickettext += _("Date:");
  tickettext += " ";
  tickettext += buf;
  tickettext += "\n\n";

  tickettext += _("Description:");
  tickettext += " ";
  tickettext += description;
  tickettext += "\n\n";

  snprintf(buf,64,"$ %.2f",amount/100.0);

  tickettext += _("Cost:");
  tickettext += " ";
  tickettext += buf;
  tickettext += "\n\n\n\n\n";

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

static void
openCashRegister()
{
#ifdef WIN32
  FILE *p = fopen("PRN","w");
  char code = 0x07;

  fwrite(&code,sizeof(code),1,p);
  fclose(p);
#endif
}
