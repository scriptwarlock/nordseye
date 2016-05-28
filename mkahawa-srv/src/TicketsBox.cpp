#include <ccls.h>
#include <fox-1.6/fx.h>
#include <fox-1.6/FXRex.h>

#ifdef WIN32
#include <time.h>
#include <windows.h>
#else
#include <sys/time.h>
#endif 
using namespace FX;

#include "cclfox.h"
#include "verifiers.h"
#include "MembersFrame.h" 
#include "EmployeesFrame.h"
#include "TicketsBox.h"


//#define DEBUG
//#define DEBUG_TICKET 

extern FXGIFIcon *dbIcon01;
extern FXGIFIcon *dbIcon0;
extern FXGIFIcon *dbIcon1;
extern FXGIFIcon *dbIcon2;
extern FXGIFIcon *dbIcon3;

int        formatTicketStr(char *retStr, char *tktStr, int bsize);
void       formatTicket(FXString &tickettext, char *tktStr, CCL_ticket_entry &te);
FXString   outputToHTML(FXString &tickettext);

FXDEFMAP(TicketsBox) TicketsBoxMap[] =
{
  FXMAPFUNC(SEL_COMMAND,TicketsBox::ID_GENERATE,TicketsBox::onGenerate),
  FXMAPFUNC(SEL_COMMAND,TicketsBox::ID_PRINT,TicketsBox::onPrint),
  FXMAPFUNC(SEL_COMMAND,TicketsBox::ID_SAVE,TicketsBox::onSave),
  FXMAPFUNC(SEL_COMMAND,TicketsBox::ID_CLEAR,TicketsBox::onClear),
  //  FXMAPFUNC(SEL_COMMAND,TicketsBox::ID_TKTEDIT,TicketsBox::onEdit),
  FXMAPFUNC(SEL_COMMAND,TicketsBox::ID_SETTARIFF,TicketsBox::onSetTarif),
  FXMAPFUNC(SEL_COMMAND,TicketsBox::ID_EXIT,TicketsBox::onExit),
  FXMAPFUNC(SEL_SELECTED,TicketsBox::ID_TICKETSLIST,TicketsBox::onTicketsList)
};

FXIMPLEMENT(TicketsBox,FXDialogBox,TicketsBoxMap,
	    ARRAYNUMBER(TicketsBoxMap))

TicketsBox::~TicketsBox()
{
  
}

void
TicketsBox::create()
{
  //#ifdef DEBUG
  //printf("TicketsBox::create(): create TicketsBox\n");
  //#endif
  FXDialogBox::create();
}

TicketsBox::TicketsBox(FXComposite * parent)
  :FXDialogBox(parent,_("Generate and Print Tickets"))
{
  //FXDialogBox *ticketsBox = new FXDialogBox(parent,_("Generate and Print Tickets"));
  //mother vertical frame with 2 horizontal frames

  FXVerticalFrame *vf0 =
    new FXVerticalFrame(this,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
	0,0,0,0,0,0,0,0);
  //First horizontal frame
  FXHorizontalFrame *hf1 = new FXHorizontalFrame(vf0,LAYOUT_FILL_X);
  //Right side of horizontal frame 1
  FXVerticalFrame *vf1 =
    new FXVerticalFrame(hf1,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
			0,0,0,0,0,0,0,0);
  //Labels & TextFields Vertical field
  //Start Date
  FXHorizontalFrame *hfvf11 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  new FXLabel(hfvf11,_("Start Date"));
  tfStartDate = new FXTextField(hfvf11,10,NULL,0,TEXTFIELD_NORMAL);

  //Expiry Date
  FXHorizontalFrame *hfvf12 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  new FXLabel(hfvf12,_("Expiry Date"));
  tfExpDate = new FXTextField(hfvf12,10,NULL,0,TEXTFIELD_NORMAL);

  //Number of Tickets
  FXHorizontalFrame *hfvf13 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  new FXLabel(hfvf13,_("Ticket Value"));
  tfFaceVal = new FXTextField(hfvf13,5,NULL,0,TEXTFIELD_NORMAL);

  //Number of Digits
  FXHorizontalFrame *hfvf14 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  new FXLabel(hfvf14,_("Number of Digits"));
  tfDigitsNum = new FXTextField(hfvf14,5,NULL,0,TEXTFIELD_NORMAL);

  //Tariff
  FXHorizontalFrame *hfvf15 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  new FXLabel(hfvf15,_("Tariff: "));
  lblTariff = new FXLabel(hfvf15, "");
  lblTariff->setBackColor(FXRGB(0xdc,0xe8,0xf6));
  btnTariff = new FXButton(hfvf15, _("Change"),dbIcon1,this,ID_SETTARIFF,
			   BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
  //Number of tickets
  FXHorizontalFrame *hfvf16 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  new FXLabel(hfvf16,_("Number of Tickets"));
  tfNum = new FXTextField(hfvf16,5,NULL,0,TEXTFIELD_NORMAL);

  //Number of tickets
  FXHorizontalFrame *hfvf17 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  new FXLabel(hfvf17,_("Ticket Lifetime"));
  tfLifeTime = new FXTextField(hfvf17,5,NULL,0,TEXTFIELD_NORMAL);
  
  //Number of tickets
  FXHorizontalFrame *hfvf18 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  new FXLabel(hfvf18,_("Notes: "));
  tfNotes = new FXTextField(hfvf18,15,NULL,0,TEXTFIELD_NORMAL);
  
  //Tickets List Vertical Field
  FXVerticalFrame *vf2 =
    new FXVerticalFrame(hf1,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
			0,0,0,0,0,0,0,0);
  new FXLabel(vf2,_("Generated Tickets List"));
  new FXHorizontalSeparator(vf2);
  lstTickets = new FXFoldingList(vf2,this,ID_TICKETSLIST,
		      FOLDINGLIST_SINGLESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  lstTickets->appendHeader(_("##"),NULL,30);
  lstTickets->appendHeader(_("Ticket Number"),NULL,100);

  //End of Horizontal Frame 1
  new FXHorizontalSeparator(this);
  //Start of Horizontal Frame 2 with buttons

  FXHorizontalFrame *hf2 = new FXHorizontalFrame(vf0,LAYOUT_FILL_X);
  btnGenerate = new FXButton(hf2,_("Generate"),dbIcon2,this,ID_GENERATE,
			   BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
  //  btnPrint = new FXButton(hf2,_("Print"),dbIcon1,this,ID_PRINT,
  //			   BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
  btnSave = new FXButton(hf2,_("Save"),dbIcon1,this,ID_SAVE,
			   BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
  btnClear = new FXButton(hf2,_("Clear"),dbIcon1,this,ID_CLEAR,
			  BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
  btnExit = new FXButton(hf2,_("Exit"),dbIcon1,this,FXDialogBox::ID_ACCEPT,
	       BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT);
  //  btnEdit = new FXButton(hf2,_("Query"),dbIcon1,this,ID_TKTEDIT,
  //              BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT);
  setPerms(0);
  setDefaults();
  listsavedflag = TRUE;
  listgenflag = FALSE;
}

void 
TicketsBox::setPerms(long perm)
{
  if (!isPermitted(PERMTKTGEN)){
    btnTariff->disable();
    btnGenerate->disable();
    //btnPrint->disable();
    btnClear->disable();
    //btnEdit->disable();
    btnExit->disable();
  }
  else{
    btnTariff->enable();
    btnGenerate->enable();
    //btnPrint->enable();
    btnClear->enable();
    //btnEdit->enable();
    btnExit->enable();
  }

  if (!isPermitted(PERMTKTQRY)){
    
  }
  else{
    
  }
}

void 
TicketsBox::setDefaults()
{
  time_t     t = time(NULL);
  time_t     t2 = t + 24*3600*14;
  char       buf[64]; 

  struct tm *tm = localtime(&t);
  strftime(buf,64,"%d/%m/%y",tm);
  tfStartDate->setText(buf);
  tm = localtime(&t2);
  strftime(buf,64,"%d/%m/%y",tm);
  tfExpDate->setText(buf);
  tfFaceVal->setText("100");
  tfDigitsNum->setText("9");
  tfNum->setText("10");
  tfLifeTime->setText("9");

  stariff = 1;
  char *tName = CCL_tarif_name_get(stariff);
  lblTariff->setText(tName);
  CCL_free(tName);

}

void 
TicketsBox::setPrevious()
{
  time_t     t = time(NULL);
  struct tm *tm = localtime(&t);
  time_t     t2 = t + 24*3600*14;
  struct tm *tm2 = localtime(&t2);
  char       buf[64]; 

  strftime(buf,64,"%d/%m/%y",tm);
  tfStartDate->setText(buf);
  strftime(buf,64,"%d/%m/%y",tm2);
  tfExpDate->setText(buf);
  tfFaceVal->setText("100");
  tfDigitsNum->setText("9");
}

void
TicketsBox::clearList()
{


}


void
TicketsBox::clearFields()
{


}

char assign_rand_value(int num)
{
  char retval;

  if (num<27)
    retval = 64+num;
  else 
    retval = 47+num-26;

  return retval;
}

int get_ticket_str(int length, char *rand_id)
{
  int i, num=0;
  int rndval;

  if( length>0) 
  { 
    //bzero(rand_id, 20);
    memset(rand_id, 0, 20);
	
	for(i=1;i<length;i++){

      rndval = rand();
      num = (((double)rndval * 36 )/ RAND_MAX) + 1;
      rand_id[i] = assign_rand_value( num);
      if (rand_id[i] == '0' || rand_id[i] == 'O')
		i--;
    }
  }
#ifdef DEBUG
  printf("%s\n", rand_id);
#endif
} 

int get_ticket_str(int length, const char *rand_id);

int
TicketsBox::ticket_in_db(char *tktStr)
{
  return 0;
}

void
TicketsBox::generateTickets(int nr, int dgtNr)
{
  int i; 
  char ticketStr[50], buf[128];
  struct timeval t;

#ifndef WIN32
      gettimeofday(&t, NULL);
      srand( (double)t.tv_usec);
#else
      FILETIME ft;
	  GetSystemTimeAsFileTime(&ft);
	  srand(ft.dwLowDateTime);
#endif

  lstTickets->clearItems();
  for (i=0; i<nr; i++){
    get_ticket_str(dgtNr, ticketStr);
    ticketStr[0] = 48 + rand() % 9 + 1;  // *** HACK  *** ticket numbers start with a number
    if (ticket_in_db(ticketStr)){
      i--;
    }
    else{
      snprintf(buf,128,"%d\t%s",i+1,ticketStr);
      lstTickets->appendItem(NULL,buf,NULL,NULL,(void *)i);
    }
  }
  if (i > 0)
    listgenflag = TRUE;
}
  
int formatTicketStr(char *retStr, char *tktStr, int bsize)
{
  int i, j;

  for (i=0,j=1; tktStr[i]; i++,j++){
    *(retStr++) = tktStr[i];
    if (!(j % bsize) && tktStr[j])
      *(retStr++) = '-';
  }
  *retStr = 0;
}

void formatTicket(FXString &tickettext, char *tktStr, CCL_ticket_entry &te)
{
  char buf[64];
  char misc[64];
  struct tm tm, *rtm;
  time_t t = time(NULL);

  tickettext += "<td>";
  tickettext += "<table><tr><td>";
  tickettext += _("Code:  <b>");
  formatTicketStr(misc, te.name, 3);
  tickettext += misc;
  tickettext += _("</b> [Units: <b>");
  snprintf(buf, sizeof(buf), "%.2f", te.faceval/100.0);
  tickettext += buf;
  tickettext += "]</b></td></tr><tr><td>";
  //  tickettext += "</td></tr><tr><td>";
  tickettext += _("Valid Period: ");
  tickettext += "</td></tr><tr halign=\"right\"><td>";
  tickettext += _("    From: <b>");
  //localtime_r(&te.stdate, &tm);
  rtm = localtime(&te.stdate);
  tm = *rtm;
  strftime(buf,sizeof(buf)/sizeof(char),"%d/%m/%Y",&tm);
  tickettext += buf;
  tickettext += _("</b> to <b>");
  //localtime_r(&te.enddate, &tm);
  rtm = localtime(&te.enddate);
  tm = *rtm;
  strftime(buf,sizeof(buf)/sizeof(char),"%d/%m/%Y",&tm);
  tickettext += buf;
  //  tickettext += "</td></tr><tr><td>";
  //  tickettext += _("Tariff/Notes: ");
  //  tickettext += "[ ";
  //  char *tName = CCL_tarif_name_get(te.tariff);
  //  tickettext += tName;
  //  CCL_free(tName);
  //  tickettext += _(" ] ");
  //  tickettext += tktStr;
  tickettext += "</b></td></tr><tr><td>";
  tickettext += _("Printed: ");
  // tName = CCL_emp_name_get(empId);
  //CCL_free(tName);
  //localtime_r(&t, &tm);
  rtm = localtime(&t);
  tm = *rtm;
  strftime(buf,sizeof(buf)/sizeof(char),"%d/%m/%Y",&tm);
  tickettext += buf;

  tickettext += _("[");
  char *tName = (char *)CCL_employee_name_get(e_inf.empID);
  tickettext += tName;
  tickettext += _("]");

  tickettext += "</td></tr></table>";
}

int tkt2te(CCL_ticket_entry &te, ticket_input_t &tkt)
{
  te.stdate = tkt.stDate;
  te.enddate = tkt.expDate;
  te.faceval = tkt.faceVal;
}

FXString
outputToHTML(FXString &tickettext)
{
#ifdef DEBUG
  printf("%s\n", tickettext.text());
#endif
  FXString filename = FXPath::unique("_ticket.html");
#ifdef WIN32
  FXString command = "firefox.exe ";
#else
  FXString command = "/usr/bin/firefox ";
#endif
  FILE *p = fopen(filename.text(),"w");
  fwrite(tickettext.text(),sizeof(char),tickettext.length(),p);
  fclose(p);
  command += filename;
  system(command.text());

  return FXPath::absolute(filename);
 }

#define COL 3

long
TicketsBox::printTickets()
{
  char              buf[32], num[32], pdate[15], expdate[15];
  int               faceval, nbuf;
  char              misc[64];
  struct tm         tm, *rtm;
  time_t            t = time(NULL);
  char             *notes = "";
  ticket_input_t    tkt;
  CCL_ticket_entry  te;

  getInputVals(&tkt);
  if (lstTickets->getNumItems()>0){
    FXFoldingItem *lastItem = lstTickets->getLastItem();
    FXString       tickettext = "<html><head><title>Mkahawa Tickets</title></head>\n<body>\n";
    int            counter=0;

    tickettext += "<table border=1 cellpadding=2 cellspacing=1 frame=void>\n";
    for (FXFoldingItem *tktItem = lstTickets->getFirstItem(); ; tktItem = tktItem->getNext()){
      if (!(counter % COL)){ //break up every 4 cells
	tickettext += "<tr>";
      }
      tickettext += "\n <td>";
      sscanf(tktItem->getText().text(), "%s %s", buf, te.name);
      formatTicket(tickettext, notes, te);
      tickettext += "</td>";
      if (lastItem ==  tktItem){
	tickettext += "</tr>\n";
	break;
      }
      if ((counter % COL) == COL-1){
	tickettext += "</tr>\n";
      }
      counter++;
    }
    tickettext += "</table>";

    FXString fname = outputToHTML(tickettext);
    FXString msg(_("Tickets have been saved in "));
    msg += fname;
    msg += "\n Open the file with any browser and print.";
    FXMessageBox::information(this,MBOX_OK,_("Ticket Printing"), msg.text());
    return 1;
  }

  return 1;
}


#ifdef WIN32
#include "strptime.h"
#endif
int  
TicketsBox::getInputVals(ticket_input_t *tkt)
{
  struct tm res;
  
  memset(&res,0, sizeof(struct tm));
  strptime(tfStartDate->getText().text(), "%d/%m/%y", &res);
  //sscanf(tfStartDate->getText().text(),"%d/%d/%d",&res.tm_mday,&res.tm_mon, &res.tm_year);
  //res.tm_year += 100;
  tkt->stDate = mktime(&res);
  memset(&res,0, sizeof(struct tm));
  strptime(tfExpDate->getText().text(), "%d/%m/%y", &res);
  //sscanf(tfExpDate->getText().text(),"%d/%d/%d",&res.tm_mday,&res.tm_mon, &res.tm_year);
  //res.tm_year += 100;
  tkt->expDate = mktime(&res);
  tkt->faceVal = (int)atof(tfFaceVal->getText().text())*100;
  tkt->tariff = stariff;
  tkt->dgtNr = atoi( tfDigitsNum->getText().text());
  if (tkt->dgtNr > 12) tkt->dgtNr = 12;
  tkt->tktNr = atoi( tfNum->getText().text());
  strncpy(tkt->notes, tfNotes->getText().text(), 25);

#ifdef DEBUG_TICKET
  printf ("TicketsBox::getInputVals(): tarif = %d, stariff: %d\n", tkt->tariff, stariff);
#endif
  
  return 1;
}

long 
TicketsBox::onGenerate(FXObject*,FXSelector,void*)
{
  ticket_input_t tkt;
  getInputVals(&tkt);

  generateTickets(tkt.tktNr, tkt.dgtNr);
  btnGenerate->disable();
  btnClear->enable();
  listsavedflag = FALSE;

  return 0;
}

typedef  struct {
    time_t d1;
    time_t d2;
} startexp_t;
    
long
TicketsBox::addTicket(char *ticketStr, char *stexp, char *faceVal, int tariff)
{
  FXString name;
  int id = CCL_member_new(ticketStr, e_inf.empID);
  int faceval = atoi(faceVal);

  if (-1 != id) {
    CCL_member_flags_toggle(id, MEMBER_TICKET, TRUE);
    CCL_member_other_set(id, stexp);
    CCL_member_email_set(id, faceVal);
    CCL_member_tarif_set(id, tariff);
    CCL_pay_account(id, (int)(faceval), e_inf.empID);
  } else{
    ;//Illegal ticket
  }
  return id;
}

int make_ddate_str(char *datestr, time_t *dt1, time_t *dt2)
{
#define DLEN 8
  char buf[17];
  struct tm tm, *rtm;

  //bzero(buf, 17);
  memset(buf, 0, 17);
  //localtime_r(dt1, &tm);
  rtm = localtime(dt1);
  tm = *rtm;
  sprintf(buf, "%02d%02d%02d",  tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);
  //localtime_r(dt2, &tm);
  rtm = localtime(dt2);
  tm = *rtm;
  sprintf(buf+DLEN, "%02d%02d%02d",  tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);
  strncpy(datestr, buf, 17);
}

long 
TicketsBox::saveTickets()
{
  if (listsavedflag == TRUE)
    return 1;
  char  buf[512];
  char  other[64];
  char  email[64];
  char  tktStr[64];
  int   tariff = 0;
  ticket_input_t tkt;
  int   id, counter=0; 

  getInputVals(&tkt);
  make_ddate_str(other, &tkt.stDate, &tkt.expDate);
  sprintf(email, "%d", tkt.faceVal);
  
  FXFoldingItem *lastItem = lstTickets->getLastItem();
  for (FXFoldingItem *tktItem = lstTickets->getFirstItem(); ; tktItem = tktItem->getNext()){
    sscanf(tktItem->getText().text(), "%s %s", buf, tktStr);
    id = addTicket(tktStr, other, email, tkt.tariff);
    if (id > 0 ){
      sprintf(buf, "insert into tickets (id,name,pdate,tarif,stdate,expdate,empid,faceval,credit,flags) values (%d, '%s', %ld, %d, %ld, %ld, %d, %d, %d, %d)", id, tktStr, time(NULL), tkt.tariff, tkt.stDate,
	      tkt.expDate, e_inf.empID, tkt.faceVal, tkt.faceVal, MEMBER_TICKET);
#ifdef DEBUG_TICKET
      printf("%s\n", buf); 
#endif
      CCL_member_ticket_new(-1, buf);
    }
    if (lastItem ==  tktItem)
      break;
    counter++;
  }
  if (counter) 
    listsavedflag = TRUE;
  return 1;
}

long 
TicketsBox::onPrint(FXObject*,FXSelector,void*)
{
  saveTickets();
  if (!printTickets()){
    char *msg = _("Nothing to print. Generate or Query First");
    FXMessageBox::information(getRoot(), MBOX_OK,_("Ticket Printing"), (char *)msg);
  }
  return 0;
}

long 
TicketsBox::onSave(FXObject*,FXSelector,void*)
{
  saveTickets();
  FXString msg(_("Tickets have been saved in the DB."));

  FXMessageBox::information(this,MBOX_OK,_("Save Tickets"), msg.text());
  return 0;
}

long 
TicketsBox::onTicketsList(FXObject*,FXSelector,void*)
{
 
    return 0;
}

/*long
TicketsBox::onEdit(FXObject*,FXSelector,void*)
{
  QTicketsBox  tBox(this);

#ifdef DEBUG
  printf("onTickets(): tickets button was pressed\n");
#endif
  tBox.execute();

  return 1;  
}
*/


long
TicketsBox::onClear(FXObject*,FXSelector,void*)
{
  lstTickets->clearItems();
  btnGenerate->enable();
  btnClear->disable();

  return 0;
}

long
TicketsBox::onSetTarif(FXObject*,FXSelector,void*)
{
  /*  if (!isPermitted(PERMTKTEDIT)){
    noPermInfo();
    return 0;
    }*/
  int tarif = CCL_tarif_get();
  FXDialogBox dlg(this,_("Tarif"));
  FXVerticalFrame *vframe = new FXVerticalFrame(&dlg,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXVerticalFrame *tlistframe = new FXVerticalFrame(vframe,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
			0,0,0,0,0,0,0,0);
  FXFoldingList *tlist = new FXFoldingList(tlistframe,NULL,0,
		      LAYOUT_FILL_X|LAYOUT_FILL_Y|FOLDINGLIST_SINGLESELECT);
  new FXButton(vframe,_("Accept"),dbIcon1,&dlg,FXDialogBox::ID_ACCEPT, 
	       BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT);
  tlist->appendHeader(_("ID"),NULL,40);
  tlist->appendHeader(_("Name"),NULL,180);
  dlg.resize(250,200);
  for (int i = 0, t; -1 != (t = CCL_tarif_get_nth(i)); i++) {
    const char *name = NULL;
    char buf[256];

    name = CCL_tarif_name_get(t);
    snprintf(buf,256,"%d\t%s",t,name);
    CCL_free(name);
    tlist->appendItem(NULL,buf,NULL,NULL,(void*)t);
  }
  FXFoldingItem *ctitem = tlist->findItemByData((void*)tarif);
  if (ctitem) 
    tlist->selectItem(ctitem);
  if (dlg.execute()) {
    FXFoldingItem *sitem = tlist->getCurrentItem();

    if (sitem){
      stariff = (long)(sitem->getData());
      char *tName = CCL_tarif_name_get(stariff);
      lblTariff->setText(tName);
      CCL_free(tName);
    }
  }
  return 1;
}

long
TicketsBox::onExit(FXObject*,FXSelector,void*)
{
  //FXDialogBox::exit();
  return 0;
}
