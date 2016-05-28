#include <ccls.h>
#include <fox-1.6/fx.h>
#include <fox-1.6/FXRex.h>
#include <sys/time.h>
#include <ctype.h>

using namespace FX;

#include "cclfox.h"
#include "verifiers.h"
#include "MembersFrame.h" 
#include "EmployeesFrame.h"
#include "TicketsBox.h"
#include "SettingsBox.h"

extern FXGIFIcon *dbIcon01;
extern FXGIFIcon *dbIcon0;
extern FXGIFIcon *dbIcon1;
extern FXGIFIcon *dbIcon2;
extern FXGIFIcon *dbIcon3;

//#define DEBUG

FXDEFMAP(SettingsBox) SettingsBoxMap[] =
{
  FXMAPFUNC(SEL_COMMAND,SettingsBox::ID_SAVE,SettingsBox::onSave),
  FXMAPFUNC(SEL_COMMAND,SettingsBox::ID_CANCEL,SettingsBox::onCancel),
  FXMAPFUNC(SEL_COMMAND,SettingsBox::ID_EXIT,SettingsBox::onApply),
  FXMAPFUNC(SEL_COMMAND,SettingsBox::ID_EXIT,SettingsBox::onExit),
  FXMAPFUNC(SEL_COMMAND,SettingsBox::ID_SETTARIF,SettingsBox::onSetTarif),
  FXMAPFUNCS(SEL_COMMAND, SettingsBox::ID_RADIO11,SettingsBox::ID_RADIO15,     SettingsBox::onCmdRadRndOff),
  FXMAPFUNCS(SEL_UPDATE,  SettingsBox::ID_RADIO11,SettingsBox::ID_RADIO15,     SettingsBox::onUpdRadRndOff)

};

FXIMPLEMENT(SettingsBox,FXDialogBox,SettingsBoxMap,
	    ARRAYNUMBER(SettingsBoxMap))


CyberSettings::CyberSettings()
{
  if (loadSettings() < 0){
    cyber_op_mode = OPMODE_POSTPAID;
    round_off = RND_00;
    strncpy ( currency, "KES", 3);
    default_tariff = 1;
    poll_interval = 10;
  }
}

void 
CyberSettings::setOpMode(int opmode)
{
  cyber_op_mode = (cyb_opmode_t)opmode;
  CCL_data_set_int(CCL_DATA_SETTINGS, 0, "opmode",  cyber_op_mode);
}
void 
CyberSettings::setRoundOff(int roundoff)
{
   round_off = (enum ROUNDOFF)roundoff;
  CCL_data_set_int(CCL_DATA_SETTINGS, 0, "roundoff", round_off);
}
void 
CyberSettings::setCurrency(const char *currency)
{
  strncpy( CyberSettings::currency, currency, 3);
  CCL_data_set_string(CCL_DATA_SETTINGS, 0, "currency", currency);
}
void 
CyberSettings::setDefaultTariff(long ldefault_tariff)
{
  default_tariff = ldefault_tariff;
  CCL_data_set_int(CCL_DATA_SETTINGS, 0, "tariff", default_tariff);
}
void 
CyberSettings::setPollInterval(long poll_interval)
{
   poll_interval = poll_interval;
  CCL_data_set_int(CCL_DATA_SETTINGS, 0, "pollitvl", poll_interval);
}


cyb_opmode_t
CyberSettings::getOpMode()
{
  return  cyber_op_mode;
}
enum ROUNDOFF
CyberSettings::getRoundOff()
{
  return  round_off;
}

char *  
CyberSettings::getCurrency()
{
	
  //strndup(currency, 3);
	strdup(currency);
}
long
CyberSettings::getDefaultTariff()
{
  return  default_tariff;
}
long
CyberSettings::getPollInterval()
{
  return  poll_interval;
}

int
CyberSettings::saveSettings()
{
#ifdef DEBUG
  printf("CyberSettings::saveSettings()->Start: opmode[%08X] roundoff[%08X] currency[%s] tariff[%d] poll[%d]\n",
	 cyber_op_mode, round_off, currency, default_tariff, poll_interval);
#endif
  CCL_data_set_int(CCL_DATA_SETTINGS, 0, "opmode", cyber_op_mode);
  CCL_data_set_int(CCL_DATA_SETTINGS, 0, "roundoff", round_off);
  CCL_data_set_string(CCL_DATA_SETTINGS, 0, "currency", currency);
  CCL_data_set_int(CCL_DATA_SETTINGS, 0, "tariff", default_tariff);
  CCL_data_set_int(CCL_DATA_SETTINGS, 0, "pollitvl", poll_interval);
#ifdef DEBUG
  printf("CyberSettings::saveSettings()->End: opmode[%08X] roundoff[%08X] currency[%s] tariff[%d] poll[%d]\n",
	 cyber_op_mode, round_off, currency, default_tariff, poll_interval);
#endif

  return 0;
}

SettingsBox::SettingsBox(FXComposite * parent)
  :FXDialogBox(parent,_("Mkahawa Cyber Settings"))
{
  FXVerticalFrame *vf0 =
    new FXVerticalFrame(this,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
	0,0,0,0,0,0,0,0);

  //options vertical frame
  FXVerticalFrame *vf1 =
    new FXVerticalFrame(vf0,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
			0,0,0,0,0,0,0,0);
  //Labels & TextFields  field
  opMode = 0;
  new FXLabel(vf1,_("Cyber Operation Mode "));
  FXHorizontalFrame *hfvf10 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  chkPostpaid = new FXCheckButton(hfvf10,_("Postpaid"),NULL,0,CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  chkTicket = new FXCheckButton(hfvf10,_("Use Tickets"),NULL,0,CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  chkMember = new FXCheckButton(hfvf10,_("Prepaid Members"),NULL,0,CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  // vertical Frame 1
  new FXHorizontalSeparator(vf1);
  //currency
  new FXLabel(vf1,_("Currency "));
  FXHorizontalFrame *hfvf11 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  tfCurrency = new FXTextField(hfvf11,8,NULL,0,TEXTFIELD_NORMAL);
  new FXLabel(hfvf11,_("(USD, GBP, EUR, KES)"));
  // vertical Frame 1
  new FXHorizontalSeparator(vf1);
  //currency
  new FXLabel(vf1,_("Client Polling Interval "));
  FXHorizontalFrame *hfvf14 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  tfPollItvl = new FXTextField(hfvf14,5,NULL,0,TEXTFIELD_NORMAL);
  new FXLabel(hfvf14,_("Second(s)"));
  //vertical Frame 1
  new FXHorizontalSeparator(vf1);
  //Current Value
  new FXLabel(vf1,_("Rounding Off "));
  //Radio Buttons
  rndOff = 0;
  rtgtRndOff.connect(rndOff);
  FXHorizontalFrame *hfvf12 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  rbtnRnd00 = new FXRadioButton(hfvf12,_(".00"), &rtgtRndOff, FXDataTarget::ID_OPTION+1);
  rbtnRnd01 = new FXRadioButton(hfvf12,_(".01"), &rtgtRndOff, FXDataTarget::ID_OPTION+2);
  rbtnRnd05 = new FXRadioButton(hfvf12,_(".05"), &rtgtRndOff, FXDataTarget::ID_OPTION+3);
  rbtnRnd10 = new FXRadioButton(hfvf12,_(".10"), &rtgtRndOff, FXDataTarget::ID_OPTION+4);
  rbtnRnd25 = new FXRadioButton(hfvf12,_(".25"), &rtgtRndOff, FXDataTarget::ID_OPTION+6);
  rbtnRnd50 = new FXRadioButton(hfvf12,_(".50"), &rtgtRndOff, FXDataTarget::ID_OPTION+5);
  //vertical Frame 1
  new FXHorizontalSeparator(vf1);
  //Current Value
  new FXLabel(vf1,_("Default Tariff"));

  FXHorizontalFrame *hfvf13 = new FXHorizontalFrame(vf1,LAYOUT_FILL_X);
  lblTariff = new FXLabel(hfvf13,_("[ - ]"));
  btnSetTarif = new FXButton(hfvf13, _("Change"),dbIcon1,this,ID_SETTARIF,
			   BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);

  //End of vertical Frame 1
  new FXHorizontalSeparator(this);
  //Start of Horizontal Frame 2 with buttons
  FXHorizontalFrame *hf2 = new FXHorizontalFrame(vf0,LAYOUT_FILL_X);
  btnCancel = new FXButton(hf2,_("Cancel"),dbIcon1,this,FXDialogBox::ID_CANCEL,
			  BUTTON_TOOLBAR|FRAME_RAISED);
  btnSave = new FXButton(hf2,_("Save"),dbIcon1,this,ID_SAVE,
			   BUTTON_TOOLBAR|FRAME_RAISED);
  btnExit = new FXButton(hf2,_("Exit"),dbIcon1,this,FXDialogBox::ID_ACCEPT,
			 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_RIGHT);
  /*  playbutton =
    new FXButton(ctoolbar,_("\tStart a Session"),playicon,this,ID_START,
		 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,
		 0,0,0,0,0,0,0,0);*/
  btnApply = new FXButton(hf2,_("Apply\tApply Settings to all Clients"),dbIcon1,this,FXDialogBox::ID_ACCEPT,
			 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_RIGHT);
  cst = new CyberSettings();
  stariff = 1;
  setPerms(0);
}

SettingsBox::~SettingsBox()
{
  delete cst;
}

void
SettingsBox::create()
{
  //#ifdef DEBUG
  //printf("SettingsBox::create(): create SettingsBox\n");
  //#endif
  FXDialogBox::create();
}

long
SettingsBox::execute()
{
  setPrevious();
  return FXDialogBox::execute();
}

// Set choice
long SettingsBox::onCmdRadOpMode(FXObject*,FXSelector sel,void*){
#ifdef DEBUG
  printf("SettingsBox::onCmdRadOpMode(): sel=%08X   opMode=%08X\n", FXSELID(sel), opMode);
#endif
  opMode=FXSELID(sel);
  return 1;
}

// Update menu
long SettingsBox::onUpdRadOpMode(FXObject* sender,FXSelector sel,void*){
#ifdef DEBUG
  printf("SettingsBox::onUpdRadOpMode():  sel=%08X  opMode = %08X\n", FXSELID(sel), opMode);
#endif
  sender->handle(this,(FXSELID(sel)==opMode)?FXSEL(SEL_COMMAND,ID_CHECK):FXSEL(SEL_COMMAND,ID_UNCHECK),(void*)&opMode);
  return 1;
}

// Set choice
long SettingsBox::onCmdRadRndOff(FXObject*,FXSelector sel,void*){
#ifdef DEBUG
  printf("SettingsBox::onCmdRadRndOff():  sel=%08X  opMode = %08X\n", opMode);
  rndOff=FXSELID(sel);
#endif
  return 1;
}

// Update menu
long SettingsBox::onUpdRadRndOff(FXObject* sender,FXSelector sel,void*){
#ifdef DEBUG
  printf("SettingsBox::onUpdRadRndOff():  sel=%08X  opMode = %08X\n", opMode);
#endif
  sender->handle(this,(FXSELID(sel)==rndOff)?FXSEL(SEL_COMMAND,ID_CHECK):FXSEL(SEL_COMMAND,ID_UNCHECK),(void*)&rndOff);
  return 1;
}

void 
SettingsBox::setPerms(long perm)
{
  if (!isPermitted(PERMSETTINGS)){
    btnSave->disable();
    btnCancel->disable();
    btnExit->disable();
    btnSetTarif->disable();
  }
  else{
    btnSave->enable();
    btnCancel->enable();
    btnExit->enable();
    btnSetTarif->enable();
  }

  if (!isPermitted(PERMSETTINGS)){
    
  }
  else{
    
  }
}

void 
SettingsBox::setPrevious()
{
  CyberSettings st;

  if (st.loadSettings() < 0)
    setDefaults();
  dispInputVals(&st);
}

void 
SettingsBox::setDefaults()
{
  cst->cyber_op_mode = OPMODE_TICKET;
  cst->round_off = RND_00;
  strncpy (cst->currency, "KES", 3);
  cst->default_tariff = 1;
  cst->poll_interval = 5;
}

void 
SettingsBox::dispInputVals(CyberSettings *st)
{
  //cyber operation mode
  if (st->cyber_op_mode & OPMODE_TICKET)
    chkTicket->setCheck(TRUE);
  if (st->cyber_op_mode & OPMODE_MEMBER)
    chkMember->setCheck(TRUE);
  if (st->cyber_op_mode & OPMODE_POSTPAID)
    chkPostpaid->setCheck(TRUE);
  //Currency
  tfCurrency->setText(FXString(st->currency).left(3));
  //Poll Interval
  tfPollItvl->setText(FXStringVal((FXint)st->poll_interval));
  //tariff
  char *tName = CCL_tarif_name_get(st->default_tariff);
  lblTariff->setText("[ "+ FXString(tName) + " ]");
  CCL_free(tName);
  //round-off buttons
  rbtnRnd00->setCheck(FALSE);
  rbtnRnd01->setCheck(FALSE);
  rbtnRnd05->setCheck(FALSE);
  rbtnRnd10->setCheck(FALSE);
  rbtnRnd25->setCheck(FALSE);
  rbtnRnd50->setCheck(FALSE);
  switch(st->round_off){
  case RND_00: rbtnRnd00->setCheck(TRUE,TRUE); break;
  case RND_01: rbtnRnd01->setCheck(TRUE,TRUE); break;
  case RND_05: rbtnRnd05->setCheck(TRUE,TRUE); break;
  case RND_10: rbtnRnd10->setCheck(TRUE,TRUE); break;
  case RND_25: rbtnRnd25->setCheck(TRUE,TRUE); break;
  case RND_50: rbtnRnd50->setCheck(TRUE,TRUE); break;
  }
}

int  
SettingsBox::getInputVals(CyberSettings *st)
{
  //prepaid / postpaid
  st->cyber_op_mode = 0;
  if (chkTicket->getCheck())
    st->cyber_op_mode |= OPMODE_TICKET;
  if (chkMember->getCheck())
    st->cyber_op_mode |= OPMODE_MEMBER;
  if (chkPostpaid->getCheck())
    st->cyber_op_mode |= OPMODE_POSTPAID;
  //round off
  if (rbtnRnd00->getCheck())
    st->round_off = RND_00;
  else if (rbtnRnd01->getCheck())
    st->round_off = RND_01;
  else if (rbtnRnd05->getCheck())
    st->round_off = RND_05;
  else if (rbtnRnd10->getCheck())
    st->round_off = RND_10;
  else if (rbtnRnd25->getCheck())
    st->round_off = RND_25;
  else if (rbtnRnd50->getCheck())
    st->round_off = RND_50;
  //tariff
  st->default_tariff = stariff;
  st->poll_interval = (long)FXIntVal(tfPollItvl->getText());
  //char *cp = (char *)tfCurrency->getText().text();
  FXString  str(tfCurrency->getText().upper());
  strncpy(st->currency, str.text(), 3);
  st->currency[3] = 0;
#ifdef DEBUG
  printf("CyberSettings::getInputVals(): opmode[%08X] roundoff[%08X] currency[%s] tariff[%d] poll[%d]\n",
	 st->cyber_op_mode, st->round_off, st->currency, st->default_tariff, st->poll_interval);
#endif

  return 1;
}

int 
compare_settings(CyberSettings *st1, CyberSettings *st2)
{
  if ( st1 == NULL && st2 == NULL) 
    return 1; //equal
  if (st1 == NULL || st2 == NULL)
    return 0;  // unequal
  if ( (st1->cyber_op_mode == st2->cyber_op_mode) &&
	    (st1->round_off == st2->round_off)  &&
	    (st1->default_tariff == st2->default_tariff) &&
	    (st1->poll_interval == st2->poll_interval ) && 
	    (!strncmp(st1->currency, st2->currency, 3)) )
    return 1;  //equal

  return 0; //unequal
}

long 
SettingsBox::onSave(FXObject*,FXSelector,void*)
{
  CyberSettings st;

  getInputVals(&st);
  if (!compare_settings(&st,cst)){
    st.saveSettings();
  }
  return 0;
}

long 
SettingsBox::onApply(FXObject*,FXSelector,void*)
{
  CyberSettings st;

  getInputVals(&st);
  if (!compare_settings(&st,cst)){
    st.saveSettings();
  }
  return 0;
}

long 
SettingsBox::onSetTarif(FXObject*,FXSelector,void*)
{
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
SettingsBox::onExit(FXObject*,FXSelector,void*)
{
  CyberSettings st;

  getInputVals(&st);
  //save 
  if (!compare_settings(&st,cst)){
    if (FXMessageBox::question(this,MBOX_YES_NO,_("Confirm"),
			       _("Do you want to save changes?")) == MBOX_CLICKED_YES) {
      st.saveSettings();
    }
  }
  FXDialogBox::close();
  return 0;
}

long
SettingsBox::onCancel(FXObject*,FXSelector,void*)
{

  FXDialogBox::close();
  return 0;
}

int
SettingsBox::loadSettings(CyberSettings &st)
{
  return st.loadSettings();
}

int
CyberSettings::loadSettings()
{
  char *cp = NULL; 
  int retval = -100, ret;;

#ifdef DEBUG
  printf("CyberSettings::loadSettings(): Entering method\n");
#endif
  
  ret = CCL_data_get_int(CCL_DATA_SETTINGS, 0, "opmode", retval);
  if (ret == retval)
    return ret;
  cyber_op_mode = (cyb_opmode_t) ret;

  ret = CCL_data_get_int(CCL_DATA_SETTINGS, 0, "roundoff", retval);
  if (ret == retval)
    return ret;
  round_off = (enum ROUNDOFF) ret;

  cp = CCL_data_get_string(CCL_DATA_SETTINGS, 0, "currency", NULL);
  if (cp == NULL)
    return retval;
  strncpy(currency, cp, 3);
  CCL_free(cp);
  
  ret =  CCL_data_get_int(CCL_DATA_SETTINGS, 0, "tariff", retval);
  if (ret == retval)
    return ret;
  default_tariff = ret;
    
  ret = CCL_data_get_int(CCL_DATA_SETTINGS, 0, "pollitvl", retval);
  if (ret == retval)
    return ret;
  poll_interval = ret;
#ifdef DEBUG
  printf("CyberSettings::loadSettings(): opmode[%08X] roundoff[%08X] currency[%s] tariff[%d] poll[%d]\n",
	 cyber_op_mode, round_off, currency, default_tariff, poll_interval);
#endif

  return 0;
}
