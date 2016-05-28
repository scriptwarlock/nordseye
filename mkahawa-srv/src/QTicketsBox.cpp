#include <ccls.h>
#include <fox-1.6/fx.h>
#include <fox-1.6/FXRex.h>
#include <sys/time.h>

using namespace FX;

#include "cclfox.h"
#include "verifiers.h"
#include "MembersFrame.h" 
#include "EmployeesFrame.h"
#include "TicketsBox.h"
#include "QTicketsBox.h"

extern FXGIFIcon *dbIcon01;
extern FXGIFIcon *dbIcon0;
extern FXGIFIcon *dbIcon1;
extern FXGIFIcon *dbIcon2;
extern FXGIFIcon *dbIcon3;


extern int      formatTicketStr(char *retStr, char *tktStr, int bsize);
extern void     formatTicket(FXString &tickettext, char *tktStr, CCL_ticket_entry &te);
extern FXString outputToHTML(FXString &tickettext);

//#define DEBUG

FXDEFMAP(QTicketsBox) QTicketsBoxMap[] =
{
  FXMAPFUNC(SEL_COMMAND,QTicketsBox::ID_QUERY,QTicketsBox::onQuery),
  FXMAPFUNC(SEL_COMMAND,QTicketsBox::ID_DELETE,QTicketsBox::onDelete),
  FXMAPFUNC(SEL_COMMAND,QTicketsBox::ID_CLEAR,QTicketsBox::onClear),
  FXMAPFUNC(SEL_COMMAND,QTicketsBox::ID_PRINT,QTicketsBox::onPrint),
  //  FXMAPFUNC(SEL_COMMAND,QTicketsBox::ID_EXIT,QTicketsBox::onExit),
  FXMAPFUNC(SEL_COMMAND,QTicketsBox::ID_GENERATE,QTicketsBox::onGenerate),
  FXMAPFUNCS(SEL_COMMAND, QTicketsBox::ID_RADIO1,QTicketsBox::ID_RADIO3,     QTicketsBox::onCmdRadio),
  FXMAPFUNCS(SEL_UPDATE,  QTicketsBox::ID_RADIO1,QTicketsBox::ID_RADIO3,     QTicketsBox::onUpdRadio),
  FXMAPFUNC(SEL_SELECTED,QTicketsBox::ID_TICKETSLIST,QTicketsBox::onTicketsList)
};

FXIMPLEMENT(QTicketsBox,FXVerticalFrame,QTicketsBoxMap,
	    ARRAYNUMBER(QTicketsBoxMap))

QTicketsBox::~QTicketsBox()
{
  
}

void
QTicketsBox::create()
{
  //#ifdef DEBUG
  //printf("QTicketsBox::create(): create QTicketsBox\n");
  //#endif
  FXVerticalFrame::create();
}

QTicketsBox::QTicketsBox(FXComposite * parent)
  //  :FXVerticalFrame(parent,_("Query and Purge Expired Tickets"))
 :FXVerticalFrame(parent,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN,
		0,0,0,0,0,0,0,0,0,0)
  // :FXVerticalFrame(parent,_("Query and Purge Expired Tickets"))
{
  FXVerticalFrame *vf0 =
    new FXVerticalFrame(this,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
	0,0,0,0,0,0,0,0);
  //First horizontal frame
  //FXHorizontalFrame *hf1 = new FXHorizontalFrame(vf0,LAYOUT_FILL_X);


  //options vertical frame
  FXVerticalFrame *vf1 =
    new FXVerticalFrame(vf0,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
			0,0,0,0,0,0,0,0);

  //Tickets List Vertical Field
  FXVerticalFrame *vf2 =
    new FXVerticalFrame(vf1,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
			0,0,380,200,0,0,0,0);
  new FXLabel(vf2,_("Tickets List"));
  new FXHorizontalSeparator(vf2);
  lstTickets = new FXFoldingList(vf2,this,ID_TICKETSLIST,
				 FOLDINGLIST_SINGLESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y,
				 0,0,370,200);
  lstTickets->appendHeader(_("##"),NULL,30);
  lstTickets->appendHeader(_("Ticket"),NULL,100);
  lstTickets->appendHeader(_("Value"),NULL,50);
  lstTickets->appendHeader(_("Current"),NULL,50);
  lstTickets->appendHeader(_("Printed"),NULL,60);
  lstTickets->appendHeader(_("Expiry"),NULL,60);
  //End of Horizontal Frame 2 & GUI
  new FXHorizontalSeparator(this);

  //Labels & TextFields  field
  //Radio Buttons
  tktType = 0;
  rtgt.connect(tktType);
  //rtgt.setTarget((FXObject *)&tktType);
  FXHorizontalFrame *hfvf10 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  new FXLabel(hfvf10,_("Type of Ticket: "));
  rbtnAll = new FXRadioButton(hfvf10,_("All"), &rtgt, FXDataTarget::ID_OPTION+1);
  rbtnValid = new FXRadioButton(hfvf10,_("Valid"), &rtgt, FXDataTarget::ID_OPTION+2);
  rbtnExpired = new FXRadioButton(hfvf10,_("Expired"), &rtgt, FXDataTarget::ID_OPTION+3);
  //End of vertical Frame 1
  new FXHorizontalSeparator(vf1);

  //Face Value
  FXHorizontalFrame *hfvf11 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  chkFaceVal = new FXCheckButton(hfvf11,_("Face Value: "),NULL,0, CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  new FXLabel(hfvf11,_("Between: "));
  tfFaceVal1 = new FXTextField(hfvf11,8,NULL,0,TEXTFIELD_NORMAL);
  new FXLabel(hfvf11,_(" and "));
  tfFaceVal2 = new FXTextField(hfvf11,8,NULL,0,TEXTFIELD_NORMAL);

  //Current Value
  FXHorizontalFrame *hfvf12 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  chkCurVal = new FXCheckButton(hfvf12,_("Current Value: "),NULL,0, CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  new FXLabel(hfvf12,_("Between: "));
  tfCurVal1 = new FXTextField(hfvf12,8,NULL,0,TEXTFIELD_NORMAL);
  new FXLabel(hfvf12,_(" and "));
  tfCurVal2 = new FXTextField(hfvf12,8,NULL,0,TEXTFIELD_NORMAL);

  //Expiry Date
  FXHorizontalFrame *hfvf13 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  chkExpDate = new FXCheckButton(hfvf13,_("Expiry Date: "),NULL,0, CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  new FXLabel(hfvf13,_("Between"));
  tfExpDate1 = new FXTextField(hfvf13,10,NULL,0,TEXTFIELD_NORMAL);
  new FXLabel(hfvf13,_(" and "));
  tfExpDate2 = new FXTextField(hfvf13,10,NULL,0,TEXTFIELD_NORMAL);

  //Print out Date
  FXHorizontalFrame *hfvf14 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  chkPrnDate = new FXCheckButton(hfvf14,_("Printing Date: "),NULL,0, CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  new FXLabel(hfvf14,_("Between: "));
  tfPrnDate1 = new FXTextField(hfvf14,10,NULL,0,TEXTFIELD_NORMAL);
  new FXLabel(hfvf14,_(" and "));
  tfPrnDate2 = new FXTextField(hfvf14,10,NULL,0,TEXTFIELD_NORMAL);


  //End of vertical Frame 1
  new FXHorizontalSeparator(this);
  //Start of Horizontal Frame 2 with buttons
  FXHorizontalFrame *hf2 = new FXHorizontalFrame(vf0,LAYOUT_FILL_X);
  btnQuery = new FXButton(hf2,_("Refresh"),dbIcon1,this,ID_QUERY,
			  BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
  btnClear = new FXButton(hf2,_("Clear"),dbIcon1,this,ID_CLEAR,
			   BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
  btnDelete = new FXButton(hf2,_("Delete"),dbIcon1,this,ID_DELETE,
			   BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT);
  btnPrint = new FXButton(hf2,_("Print"),dbIcon1,this,ID_PRINT,
			  BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT);
  btnGenerate = new FXButton(hf2,_("Generate"),dbIcon2,this,ID_GENERATE,
			  BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT);
  //End of vertical Frame 1
  new FXHorizontalSeparator(this);

  setPerms(0);
  //setPrevious();
  setDefaults();
  //update the credits
  char *sqlstr = "update tickets set credit = (select credit from members where id = tickets.id)";
  CCL_ticket_entry *te = NULL;
  int nr = CCL_member_tickets_get(sqlstr, &te);
  CCL_free(te);
}

// Set choice
long QTicketsBox::onCmdRadio(FXObject*,FXSelector sel,void*){
  tktType=FXSELID(sel);
  return 1;
}

// Update menu
long QTicketsBox::onUpdRadio(FXObject* sender,FXSelector sel,void*){
  sender->handle(this,(FXSELID(sel)==tktType)?FXSEL(SEL_COMMAND,ID_CHECK):FXSEL(SEL_COMMAND,ID_UNCHECK),(void*)&tktType);
  return 1;
}

void 
QTicketsBox::setPerms(long perm)
{
  if (!isPermitted(PERMTKTQRY)){
    btnQuery->disable();
    btnClear->disable();
  }
  else{
    btnQuery->enable();
    btnClear->enable();
  }

  if (!isPermitted(PERMTKTGEN)){
    btnGenerate->disable();
    btnDelete->disable();
  }
  else{
    btnGenerate->enable();    
    btnDelete->enable();
  }

  if (!isPermitted(PERMTKTPRN)){
    btnPrint->disable();
  }
  else{
    btnPrint->enable();
  }
}

void 
QTicketsBox::setDefaults()
{
  time_t     t = time(NULL) - 36000;
  struct tm *tm = localtime(&t);
  time_t     t2 = t + 24*3600*14;
  struct tm *tm2 = localtime(&t2);
  char       buf[64]; 

  strftime(buf,64,"%d/%m/%y",tm);
  tfPrnDate1->setText(buf);
  strftime(buf,64,"%d/%m/%y",tm);
  tfPrnDate2->setText(buf);
  strftime(buf,64,"%d/%m/%y",tm2);
  tfExpDate1->setText(buf);
  strftime(buf,64,"%d/%m/%y",tm2);
  tfExpDate2->setText(buf);

  tfFaceVal1->setText("10");
  tfFaceVal2->setText("200");
  tfCurVal1->setText("10");
  tfCurVal2->setText("300");

  chkFaceVal->setCheck();

  //rtgt.setSelector(FXDataTarget::ID_OPTION+1);
  
  rbtnAll->setCheck(TRUE, TRUE);
  rbtnExpired->setCheck(FALSE);
  rbtnValid->setCheck(FALSE);
}

void 
QTicketsBox::setPrevious()
{


}

void
QTicketsBox::clearList()
{
  

}


void
QTicketsBox::clearFields()
{


}

#ifdef WIN32
//#include "strptime.h"
char * strptime(const char *buf, const char *fmt, struct tm *tm);
#endif


inline time_t 
date2time_t(char *datestr)
{
  struct tm res;

#ifdef DEBUG

#endif  
  memset(&res, 0, sizeof(struct tm));
  strptime(datestr, "%d/%m/%y", &res);
#ifdef DEBUG
  printf("date2time_t(): %s ->  %d/%d/%d \n", datestr, res.tm_year, res.tm_mon, res.tm_mday );
#endif  
  return mktime(&res);
}

inline void
time_t2date(time_t t, const char *datestr)
{
  

  return;
}
int  
QTicketsBox::getInputVals(qticket_input_t *qtkt)
{
  struct tm res;
  
  //  memset(&res, 0, sizeof(struct tm));
  //  strptime((char *)tfPrnDate1->getText().text(), "%d/%m/%y", &res);
  qtkt->prnDate1 = date2time_t((char *)tfPrnDate1->getText().text()); //mktime(&res);
  //memset(&res, 0, sizeof(struct tm));
  //strptime((char *)tfPrnDate2->getText().text(), "%d/%m/%y", &res);
  qtkt->prnDate2 = date2time_t((char *)tfPrnDate2->getText().text()); //mktime(&res);
#ifdef DEBUG
  printf("Print Date: %ld - %ld = %ld\n", qtkt->prnDate2, qtkt->prnDate1, qtkt->prnDate2 - qtkt->prnDate1);
#endif
  memset(&res, 0, sizeof(struct tm));
  strptime(tfExpDate1->getText().text(), "%d/%m/%y", &res);
  qtkt->expDate1 = mktime(&res);
  memset(&res, 0, sizeof(struct tm));
  strptime(tfExpDate2->getText().text(), "%d/%m/%y", &res);
  qtkt->expDate2 = mktime(&res);
#ifdef DEBUG
  printf("Expiry Date: %ld - %ld = %ld\n", qtkt->expDate2, qtkt->expDate1, qtkt->prnDate2 - qtkt->prnDate1);
#endif
  qtkt->faceVal1 = (int)(atof(tfFaceVal1->getText().text()) * 100);
  qtkt->curVal1 = (int)(atof(tfCurVal1->getText().text()) * 100); 
  qtkt->faceVal2 = (int)(atof(tfFaceVal2->getText().text()) * 100);
  qtkt->curVal2 = (int)(atof(tfCurVal2->getText().text()) * 100);
  //get ticket type
  if (rbtnAll->getCheck())
    qtkt->tktType = QTKT_ALL;
  else if (rbtnExpired->getCheck())
    qtkt->tktType = QTKT_EXPIRED;
  else
    qtkt->tktType = QTKT_VALID;
  //get query type
  if (chkFaceVal->getCheck())
    qtkt->qryType |= QFACEVAL;
  if (chkExpDate->getCheck())
    qtkt->qryType |= QEXPDATE;
  if (chkCurVal->getCheck())
    qtkt->qryType |= QCURVAL;
  if (chkPrnDate->getCheck())
    qtkt->qryType |= QPRNDATE;

  return 1;
}

int 
QTicketsBox::create_query(qticket_input_t *qtkt, qticket_qry_t *qry)
{
  char     *buf, b[512];
  int      ccount = 0;

  buf = qry->qryStr;
  sprintf(buf, "select * from tickets ");
  //bzero(b,512);
  memset(b, 0, 512);
  //face value
  if (qtkt->qryType & QFACEVAL){
    if (qtkt->faceVal1== qtkt->faceVal2){
      sprintf(b, "where faceval = %d", qtkt->faceVal1);
    }
    else{
      sprintf(b, "where faceval>=%d and faceval<=%d", qtkt->faceVal1, qtkt->faceVal2);
    }
    sprintf(buf, "%s %s", buf, b);
    ccount++;
  }
  //current credit valu
  if (qtkt->qryType & QCURVAL){
    if (qtkt->curVal1== qtkt->curVal2){
      sprintf(b, " credit = %d ", qtkt->curVal1);
    }
    else{
      sprintf(b, " credit>=%d and credit<=%d ", qtkt->curVal1, qtkt->curVal2);
    }
    if (ccount)
      sprintf(buf, "%s and %s", buf, b);
    else
      sprintf(buf, "%s where %s", buf, b);
    ccount++;
  }

  //print-out date
  if (qtkt->qryType & QPRNDATE){
    if (qtkt->prnDate1 == qtkt->prnDate2){
      sprintf(b, " pdate = %ld ", qtkt->prnDate1);
    }
    else{
      //dates between
      sprintf(b, " pdate>=%ld and pdate<=%ld ", qtkt->prnDate1, qtkt->prnDate2);
    }
    if (ccount)
      sprintf(buf, "%s and %s", buf, b);
    else
      sprintf(buf, "%s where %s", buf, b);
    ccount++;
  }
  //expiry date
  if (qtkt->qryType & QEXPDATE){
    if (qtkt->expDate1 == qtkt->expDate2){
      sprintf(b, " expdate = %ld ", qtkt->expDate1);
    }
    else{
      //dates between
      sprintf(b, " expdate>=%ld and expdate<=%ld ", qtkt->expDate1, qtkt->expDate2);
    }
    if (ccount)
      sprintf(buf, "%s and %s", buf, b);
    else
      sprintf(buf, "%s where %s", buf, b);
    ccount++;
  }
  if (!ccount)
    strcat(buf, " where");
  else
    strcat(buf, " and");
  //ticket type - expired , valid, all   -- LAST CONDITION
  switch (qtkt->tktType){
  case QTKT_EXPIRED:
    sprintf(buf, "%s expdate < %ld", buf, (long)time(NULL));
    break;
  case QTKT_VALID:
    sprintf(buf, "%s expdate >= %ld", buf, (long)time(NULL));
    break;
  default:
    sprintf(buf, "%s tarif >= 0", buf);
    break;
  }

  return 0;
}
  

int
getLiveCredit(int member, int dbcredit)
{
  FXuint client, owed, owed_val;

  for (int i=0; client = CCL_client_get_nth(i); i++){
	  if (member == CCL_client_member_get(client)){
		owed = CCL_client_owed_terminal(client);
		owed_val = dbcredit - owed;
		return owed_val;
	  }
  }
  return dbcredit;
}

int 
QTicketsBox::exec_query(qticket_qry_t *qry)
{
  int num, i; 
  char ticketStr[50], buf[128];
  CCL_ticket_entry *te = NULL;
  char pdate[64], expdate[64];
  int credit;

  lstTickets->clearItems();
#ifdef DEBUG
  printf("QTicketsBox::exec_query(): %s\n", qry->qryStr);
#endif 
  num = CCL_member_tickets_get(qry->qryStr, &te);

  for (i=0; i<num; i++){
    strftime(pdate,64,"%d/%m/%y",localtime(&(te[i].stdate)));
    strftime(expdate,64,"%d/%m/%y",localtime(&(te[i].enddate)));
    /*    snprintf(buf,128,"%d\t%s\t%.2f\t%.2f\t%s\t%s",i+1,
	     te[i].name, te[i].faceval/100.0, te[i].credit/100.0, 
	     pdate, expdate); */
    credit = getLiveCredit(te[i].id, te[i].credit);
    snprintf(buf,128,"%d\t%s\t%.2f\t%.2f\t%s\t%s",i+1,
	     te[i].name, te[i].faceval/100.0, credit/100.0, 
	     pdate, expdate); 
    lstTickets->appendItem(NULL,buf,NULL,NULL,(void *)te[i].id);
  }
  CCL_free(te);
#ifdef DEBUG
  printf("QTicketsBox::exec_query(): Query well executed.\n", qry->qryStr);
#endif 
  return 0;
}

int
QTicketsBox::clear_query(qticket_qry_t *qry)
{

  //do nothing

  return 0;
}

long 
QTicketsBox::onQuery(FXObject*,FXSelector,void*)
{
  qticket_input_t tkt;
  qticket_qry_t   qry;

  //update the credits
  /*  char *sqlstr = "update tickets set credit = (select credit from members where id = tickets.id)";
  CCL_ticket_entry *te = NULL;
  int nr = CCL_member_tickets_get(sqlstr, &te);
  CCL_free(te);*/
  //now re-read the values
  memset((void *)&tkt, 0, sizeof(tkt));
  memset((void *)&qry, 0, sizeof(qry));
  getInputVals(&tkt);
  create_query(&tkt, &qry);
  exec_query(&qry);
  clear_query(&qry);
  btnClear->enable();
  //ticketsqueried = TRUE;

  return 0;
}
 
typedef  struct {
    time_t d1;
    time_t d2;
} startexp_t;
    
long 
QTicketsBox::onTicketsList(FXObject*,FXSelector,void*)
{
 
    return 0;
}

long 
QTicketsBox::onPrint(FXObject*,FXSelector,void*)
{
  if (!printTickets()){
    char *msg = "Nothing to print. Generate First";
    FXMessageBox::information(getRoot(), MBOX_OK,_("Ticket Printing"), (char *)msg);
  }
  return 0;
}

void formatTicket_(FXString &tickettext, char *tktStr, CCL_ticket_entry &te)
{
  char buf[64];
  char misc[64];
  struct tm tm, *rtm;
  time_t t = time(NULL);

  tickettext += "<table><tr><td>";
  tickettext += _("Code:  ");
  formatTicketStr(misc, te.name, 3);
  tickettext += misc;
  tickettext += "</td></tr><tr><td>";
  tickettext += _("Value/Units:   ");
  snprintf(buf, sizeof(buf), "%.2f", te.faceval);
  tickettext += buf;
  tickettext += "</td></tr><tr><td>";
  //tickettext += "\n";
  tickettext += _("Valid From: ");
  rtm = localtime(&te.stdate);
  tm = *rtm;
  strftime(buf,sizeof(buf)/sizeof(char),"%d/%m/%Y",&tm);
  tickettext += buf;
  tickettext += _(" to ");
  rtm = localtime(&te.enddate);
  tm = *rtm;
  strftime(buf,sizeof(buf)/sizeof(char),"%d/%m/%Y",&tm);
  tickettext += buf;
  tickettext += "</td></tr><tr><td>";
  tickettext += _("Tariff/Notes: ");
  tickettext += "[ ";
  char *tName = CCL_tarif_name_get(te.tariff);
  tickettext += tName;
  CCL_free(tName);
  tickettext += _(" ] ");
  tickettext += tktStr;
  tickettext += "\n";
  tickettext += _("Printed by ");
  tName = (char *)CCL_employee_name_get(e_inf.empID);
  tickettext += tName;
  CCL_free(tName);
  tickettext += _(" on ");
  //localtime_r(&t, &tm);
  rtm = localtime(&t);
  tm = *rtm;
  strftime(buf,sizeof(buf)/sizeof(char),"%d/%m/%Y",&tm);
  tickettext += buf;
  tickettext += "</td></tr></table>";
}


#define COLS 3

long 
QTicketsBox::printTickets()
{
  char              buf[32], num[32], stdate[15], expdate[15];
  float             faceval, curval;
  CCL_ticket_entry  te;
  char              misc[64];
  struct tm         tm, *rtm;
  time_t            t = time(NULL);
  char             *notes = "";
  int               retval = 1;

  if (lstTickets->getNumItems()>0){
    FXFoldingItem  *lastItem = lstTickets->getLastItem();
    FXString        tickettext = "<html><head><title>Mkahawa Tickets</title></head>\n";
    int             counter = 0;

    tickettext += "<body>\n";
    tickettext += "<table border=1 cellpadding=2 cellspacing=1 frame=void>\n";
    for (FXFoldingItem *tktItem = lstTickets->getFirstItem(); ; tktItem = tktItem->getNext()){
      if (!(counter % COLS)){ //break up every 4 cells
	tickettext += "<tr>";
      }
      tickettext += "\n <td>";
      sscanf(tktItem->getText().text(), "%s\t%s\t%f\t%f\t%s\t%s", 
	     buf,te.name,&faceval,&curval,stdate,expdate); 
#ifdef DEBUG
      printf("QTicketsBox::printTickets(): %s=%s=%s\n", tktItem->getText().text(),
	     stdate, expdate);
#endif      
      te.stdate = date2time_t(stdate);
      te.enddate = date2time_t(expdate);
      te.faceval = faceval * 100;
      formatTicket(tickettext, notes, te);
      tickettext += "</td>";
      if (lastItem ==  tktItem){
	tickettext += "</tr>\n";
	break;
      }
      if ((counter % COLS) == COLS-1){
	tickettext += "</tr>\n";
      }
      counter++;
    }
    tickettext += "</table>";
    //printString(tickettext);
    FXString fname = outputToHTML(tickettext);
    FXString msg(_("Tickets have been saved in "));
    msg += fname;
    msg += "\n Open the file with any browser and print.";
    FXMessageBox::information(this,MBOX_OK,_("Ticket Printing"), msg.text());
  }

  return retval;
}

long
QTicketsBox::onClear(FXObject*,FXSelector,void*)
{
  lstTickets->clearItems();

  return 0;
}

/*
long
QTicketsBox::onExit(FXObject*,FXSelector,void*)
{

  //FXVerticalFrame::exit();
  return 0;
  }*/

long
QTicketsBox::onGenerate(FXObject*,FXSelector,void*)
{
  
  TicketsBox  tBox(this);

#ifdef DEBUG
  printf("onTickets(): tickets button was pressed\n");
#endif
  tBox.execute(PLACEMENT_SCREEN);

  return 1;  
}

long
QTicketsBox::onDelete(FXObject*,FXSelector,void*)
{
  char buf[512];
  long  id;
  if (lstTickets->getNumItems()==0)
    return -1;
  FXFoldingItem *lastItem = lstTickets->getLastItem();

  if (FXMessageBox::question(this,MBOX_YES_NO,_("Confirm"),
			     _("Do you really want to delete all these tickets?")) == MBOX_CLICKED_YES) {
    for (FXFoldingItem *tktItem = lstTickets->getFirstItem(); ; tktItem = tktItem->getNext()){
      id = (long) tktItem->getData();
      if (id >0 ){
	CCL_member_flags_toggle(id,MEMBER_DELETED,TRUE);
	sprintf(buf, "delete from tickets where id = %ld", id);
#ifdef DEBUG
	printf("QTicketsBox::onDelete(): %s\n", buf);
#endif
	CCL_member_ticket_del(-1, buf);
#ifdef DEBUG
	printf("QTicketsBox::onDelete(): %s\n", buf);
#endif
      }
      if (lastItem ==  tktItem)
	break;
    }
    lstTickets->clearItems();
  }

  return 0;
}
