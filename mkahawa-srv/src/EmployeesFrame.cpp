#include <ccls.h>
#include <fox-1.6/fx.h>
#include <fox-1.6/FXRex.h>
using namespace FX;

#include "cclfox.h"
#include "verifiers.h"
#include "EmployeesFrame.h"

//#define DEBUG_CCLFOX 1

extern FXGIFIcon *dbIcon01;
extern FXGIFIcon *dbIcon0;
extern FXGIFIcon *dbIcon1;
extern FXGIFIcon *dbIcon2;
extern FXGIFIcon *dbIcon3;

char pwdstr[20];
extern int activeEmpId;

FXDEFMAP(EmployeesFrame) EmployeesFrameMap[] =
{
  FXMAPFUNC(SEL_COMMAND,EmployeesFrame::ID_ADDEMPLOYEE,EmployeesFrame::onNewEmployee),
  FXMAPFUNC(SEL_COMMAND,EmployeesFrame::ID_DELEMPLOYEE,EmployeesFrame::onDelEmployee),
  FXMAPFUNC(SEL_COMMAND,EmployeesFrame::ID_APPLY,EmployeesFrame::onApplyChanges),
  FXMAPFUNC(SEL_COMMAND,EmployeesFrame::ID_EDIT,EmployeesFrame::onEdit),
  FXMAPFUNC(SEL_SELECTED,EmployeesFrame::ID_EMPLOYEESLIST,EmployeesFrame::onEmployeeSelect),
  FXMAPFUNC(SEL_COMMAND,EmployeesFrame::ID_SETUSRLVL,EmployeesFrame::onSetUsrlvl),
  FXMAPFUNC(SEL_COMMAND,EmployeesFrame::ID_RESETPASS,EmployeesFrame::onResetPass),
  FXMAPFUNC(SEL_CHANGED,EmployeesFrame::ID_FILTER,EmployeesFrame::onFilter)
};

FXIMPLEMENT(EmployeesFrame,FXVerticalFrame,EmployeesFrameMap,
	    ARRAYNUMBER(EmployeesFrameMap))

#define EMPLOYEE_DELETED	(1<<16)	// This employee was deleted

extern FXSettings *passwords;
  
EmployeesFrame::EmployeesFrame(FXComposite * parent)
:FXVerticalFrame(parent,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN,
		0,0,0,0,0,0,0,0,0,0)
{
  //FXHorizontalFrame *hframe0 = new FXHorizontalFrame(this,LAYOUT_FILL_X);
  //filtertf = new FXTextField(hframe0,40,this,ID_FILTER,TEXTFIELD_NORMAL);
  FXHorizontalFrame *hframe1 = new FXHorizontalFrame(this,LAYOUT_FILL_X);
  addemployee = new FXButton(hframe1,_("New"),dbIcon1,this,ID_ADDEMPLOYEE,
			   BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
  //applychanges = new FXButton(hframe1,_(""),NULL,this,
  //			      ID_APPLY,FRAME_RAISED|FRAME_THICK);
  editbtn = new FXButton(hframe1,_("Edit"),dbIcon1,this, ID_EDIT,
			   BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  delemployee = new FXButton(hframe1,_("Delete"),dbIcon1,this,ID_DELEMPLOYEE,
			   BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  FXVerticalFrame *employeesframe =
    new FXVerticalFrame(this,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
			0,0,0,0,0,0,0,0);
  employeeslist =
    new FXFoldingList(employeesframe,this,ID_EMPLOYEESLIST,
		      FOLDINGLIST_SINGLESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  employeeslist->appendHeader(_("UserID"),NULL,50);
  employeeslist->appendHeader(_("Name"),NULL,100);
  employeeslist->appendHeader(_("Phone"),NULL,100);
  employeeslist->appendHeader(_("Email"),NULL,100);
  employeeslist->appendHeader(_("Status"),NULL,100);
  new FXHorizontalSeparator(this);
  new FXLabel(this,_("Staff Details"),NULL,LAYOUT_CENTER_X);
  new FXHorizontalSeparator(this);
  FXHorizontalFrame *hframe2 = new FXHorizontalFrame(this,LAYOUT_FILL_X);
  new FXLabel(hframe2,_("Name:"));
  nametf = new FXTextField(hframe2,30,NULL,0,TEXTFIELD_NORMAL);
  FXHorizontalFrame *hframe3 = new FXHorizontalFrame(this,LAYOUT_FILL_X);
  new FXLabel(hframe3,_("E-Mail:"));
  emailtf = new FXTextField(hframe3,30,NULL,0,TEXTFIELD_NORMAL);
  FXHorizontalFrame *hframe4 = new FXHorizontalFrame(this,LAYOUT_FILL_X);
  new FXLabel(hframe4,_("Phone Number:"));
  phonetf = new FXTextField(hframe4,20,NULL,0,TEXTFIELD_NORMAL);
  nametf->disable();
  emailtf->disable();
  phonetf->disable();

  editedemployee = 0;
  usrlvlset = 0;
  clear();
}

EmployeesFrame::~EmployeesFrame()
{

}

void
EmployeesFrame::create()
{
  FXVerticalFrame::create();
}

void
EmployeesFrame::noPermInfo()
{
    FXMessageBox::error(this,MBOX_OK,_("Permission"),
			_("Unable to access this feature.\n Contact the Administrator"));
    return;
}

void 
EmployeesFrame::setPerms(long perm)
{
  if (!isPermitted(PERMEMPEDIT)){
    addemployee->disable();
    delemployee->disable();
    editbtn->disable();
  }
  else{
    addemployee->enable();
    delemployee->enable();
    editbtn->enable();
  }
}

void
EmployeesFrame::readAllEmployees(const char * filter)
{
  int id;
  const char *name;
  char buf[128];
  FXString regexp = ".*";
  
  if (filter)
    regexp = regexp + filter + ".*";
  FXRex rex(regexp,REX_ICASE);

  employeeslist->clearItems();
  for (FXuint i = 0; -1 != (id = CCL_employee_get_nth(i)); i++) {
    if (!(CCL_employee_flags_get(id) & EMPLOYEE_DELETED)) {
   
      CCL_employee_usrname_get(id);
#ifdef DEBUG_CCLFOX
      printf("[flags: %08X] [id = %02d] name: %s\n", 
	     CCL_employee_flags_get(id), id, name);
#endif
      if (rex.match(name)) {
	addEmployee(id);
	//snprintf(buf,128,"%d\t%s",id,name);
	//employeeslist->appendItem(NULL,buf,NULL,NULL,(void *)id);
      }
    }
  }
}

void
EmployeesFrame::readEmployee(int id)
{
  if (!CCL_employee_exists(id))
    return;

  editedemployee = id;
  nametf->setText(CCL_employee_name_get(id));
  emailtf->setText(CCL_employee_email_get(id));
  phonetf->setText(CCL_employee_phone_get(id));
#ifdef DEBUG_CCLFOX
  printf("readEmployee(): id = %d -> Phone: %s\n", id, CCL_employee_phone_get(id));
#endif
  //char *login_name = CCL_data_get_string(CCL_DATA_EMPLOYEE,id,"login_name",NULL);
  //logintf->setText(login_name);
  //CCL_free(login_name);
  //usrlvlset = CCL_employee_usrlvl_get(id);
}

void
EmployeesFrame::clear()
{
  nametf->setText("");
  emailtf->setText("");
  phonetf->setText("");
}


long
EmployeesFrame::onDelEmployee(FXObject*,FXSelector,void*)
{
  long id;
  FXFoldingItem *current = employeeslist->getCurrentItem();
  const char *name;

  if (current)
    id = (long) (employeeslist->getItemData(current));
  else
    return 1;
  name = CCL_employee_usrname_get(id);
  if (strncmp(name, "admin", 6)==0){
      FXMessageBox::error(this,MBOX_OK,_("Admin Deletion"),
			  _("Administrator cannot be deleted"));    
    return 1;
  }
  clear();
#ifdef DEBUG_CCLFOX
  printf("onDelEmployee(): [%s]Toggling Flags: %08X\n", name, 
	 CCL_employee_flags_get(id));
#endif
  CCL_employee_flags_toggle(id,EMPLOYEE_DELETED,TRUE);
  employeeslist->removeItem(current);
  //remove from data
  CCL_data_key_delete(CCL_DATA_EMPLOYEE,id,"username");
  CCL_data_key_delete(CCL_DATA_EMPLOYEE,id,"password");
#ifdef DEBUG_CCLFOX
  printf("onDelEmployee(): Toggled Flags: %08X\n", 
	 CCL_employee_flags_get(id));
#endif
  return 1;
}

long
EmployeesFrame::onApplyChanges(FXObject*,FXSelector,void*)
{
  FXFoldingItem *eitem = employeeslist->findItemByData((void*)editedemployee);
  FXString newname = nametf->getText();
  FXString newlogin = pwdstr;//logintf->getText();

  if (eitem) {
    newname.trim();
    //newlogin.trim();
    //CCL_employee_usrlvl_set(editedemployee,usrlvlset);
    //if (!newname.empty() && -1 == CCL_employee_find(newname.text()))
    //  CCL_employee_usrname_set(editedemployee,nametf->getText().text());
    CCL_employee_email_set(editedemployee,emailtf->getText().text());
    CCL_employee_phone_set(editedemployee,phonetf->getText().text());
    CCL_employee_name_set(editedemployee,nametf->getText().text());
    //if (newlogin.empty() && CCL_data_key_exists(CCL_DATA_EMPLOYEE,editedemployee,
    //						"login_name"))
    //CCL_data_key_delete(CCL_DATA_EMPLOYEE,editedemployee,"login_name");
    //else if (-1 == CCL_data_find_by_key_sval(CCL_DATA_EMPLOYEE,"login_name",
    //					     newlogin.text()))
    // CCL_data_set_string(CCL_DATA_EMPLOYEE,editedemployee,"login_name",
    //			  newlogin.text());
    eitem->setText(FXStringVal((FXint)editedemployee) + "\t" + CCL_employee_usrname_get(editedemployee));
    employeeslist->updateItem(eitem);
  }
  
  return 1;
}

long
EmployeesFrame::drawPermDialog(FXDialogBox &dlg, EmpRecStr &e)
{
  FXDataTarget usrtgt(e.usr);
  FXDataTarget nametgt(e.name);
  FXDataTarget pwdtgt(e.pwd);
  FXDataTarget emailtgt(e.email);
  FXDataTarget phonetgt(e.phone);
  //FXDataTarget lvltgt(e.lvl);
  long retval = 0;
  
  FXCheckButton *tarifeditcheck, *tarifselcheck, *memeditcheck, *memalloccheck;
  FXCheckButton *genconfcheck, *prodsellcheck, *prodeditcheck, *cashrcvcheck;
  FXCheckButton *casheditcheck, *cashdischeck, * cashcnclcheck;
  FXCheckButton *rptviewcheck, *rptsavecheck, *prodstockcheck;
  FXCheckButton *tktviewcheck, *tktprncheck, *tktgencheck;

  FXVerticalFrame *vframe = new FXVerticalFrame(&dlg,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXHorizontalFrame *hframe1 = new FXHorizontalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y);

  new FXLabel(hframe1,_("Username: "));
  FXTextField *usrtf = new FXTextField(hframe1,16,&usrtgt, FXDataTarget::ID_VALUE,
					FRAME_SUNKEN|FRAME_THICK);
  if (e.usr != "")
    usrtf->disable();

  new FXLabel(hframe1,_("Password: "));
  FXTextField *pwdtf = new FXTextField(hframe1,12,&pwdtgt, FXDataTarget::ID_VALUE,
				       TEXTFIELD_PASSWD|FRAME_SUNKEN|FRAME_THICK);
  FXHorizontalFrame *hframe2 = new FXHorizontalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(hframe2,_("Full Name: "));
  FXTextField *nametf = new FXTextField(hframe2,32,&nametgt, FXDataTarget::ID_VALUE,
				       FRAME_SUNKEN|FRAME_THICK);
  FXHorizontalFrame *hframe3 = new FXHorizontalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(hframe3,_("Email: "));
  FXTextField *emailtf = new FXTextField(hframe3,32,&emailtgt, FXDataTarget::ID_VALUE,
					 FRAME_SUNKEN|FRAME_THICK);
  new FXLabel(hframe3,_("Phone: "));
  FXTextField *phonetf = new FXTextField(hframe3,16,&phonetgt, FXDataTarget::ID_VALUE,
					 FRAME_SUNKEN|FRAME_THICK);
  new FXLabel(vframe,_("Permissions:- "));
  new FXHorizontalSeparator(vframe);
  //FXRealSpinner *lvltf = new FXRealSpinner(vframe,16,&lvltgt, FXDataTarget::ID_VALUE,
  //					    FRAME_SUNKEN|FRAME_THICK);
  //lvltf->setRange(0,2);
  FXHorizontalFrame *hframe4 = new FXHorizontalFrame(vframe,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXVerticalFrame *vframe1 = new FXVerticalFrame(hframe4,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  tarifeditcheck = new FXCheckButton(vframe1,_("Edit Tariffs"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  tarifselcheck = new FXCheckButton(vframe1,_("Change Tariffs"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  memeditcheck  = new FXCheckButton(vframe1,_("Edit Member Info"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  memalloccheck  = new FXCheckButton(vframe1,_("Set Member Clients"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  genconfcheck  = new FXCheckButton(vframe1,_("General Configuration"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  prodsellcheck  = new FXCheckButton(vframe1,_("Sell Products"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  prodeditcheck  = new FXCheckButton(vframe1,_("Edit Products"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  prodstockcheck  = new FXCheckButton(vframe1,_("Stock Products"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);

  FXVerticalFrame *vframe2 = new FXVerticalFrame(hframe4,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  cashrcvcheck  = new FXCheckButton(vframe2,_("Receive Payments"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  casheditcheck  = new FXCheckButton(vframe2,_("Edit Payments"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  cashdischeck  = new FXCheckButton(vframe2,_("Give Discounts"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  cashcnclcheck  = new FXCheckButton(vframe2,_("Cancel Payments"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  rptviewcheck  = new FXCheckButton(vframe2,_("View Reports"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  rptsavecheck  = new FXCheckButton(vframe2,_("Print Reports"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  tktviewcheck  = new FXCheckButton(vframe2,_("View Tickets"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  tktgencheck  = new FXCheckButton(vframe2,_("Generate Tickets"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);
  tktprncheck  = new FXCheckButton(vframe2,_("Print Tickets"),NULL,0,
				    CHECKBUTTON_NORMAL|LAYOUT_LEFT);


  new FXHorizontalSeparator(vframe);
  FXHorizontalFrame *hframe = new FXHorizontalFrame(vframe,LAYOUT_FILL_X);
  new FXButton(hframe,_("Cancel"),dbIcon2,&dlg,FXDialogBox::ID_CANCEL,
	       BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK|LAYOUT_LEFT);
  new FXButton(hframe,_("Accept"),dbIcon2,&dlg,FXDialogBox::ID_ACCEPT,
	       BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT);

  unsigned long permval = (unsigned long)e.lvl;
  //temporary
  //e.lvl = 0xFFFFFFFF;
  //permval = 0xFFFFFFFF;
  //e_inf.lvl = 0xFFFFFFFF;
  //get permissions
  if (permval & PERMTARIFEDIT ) tarifeditcheck->setCheck(TRUE);
  if (permval & PERMTARIFSELECT) tarifselcheck->setCheck(TRUE);
  if (permval & PERMMBREDIT) memeditcheck->setCheck(TRUE) ;
  if (permval & PERMMBRALLOC) memalloccheck->setCheck(TRUE) ;
  if (permval & PERMGENCONF) genconfcheck->setCheck(TRUE) ;
  if (permval & PERMPRODSELL) prodsellcheck->setCheck(TRUE);
  if (permval & PERMPRODEDIT) prodeditcheck->setCheck(TRUE) ;
  if (permval & PERMPRODSTOCK) prodstockcheck->setCheck(TRUE) ;
  if (permval & PERMCASHRECEIVE) cashrcvcheck->setCheck(TRUE);
  if (permval & PERMCASHEDIT) casheditcheck->setCheck(TRUE);
  if (permval & PERMCASHDISCOUNT) cashdischeck->setCheck(TRUE);
  if (permval & PERMCASHCANCEL) cashcnclcheck->setCheck(TRUE);
  if (permval & PERMLOGSUMVIEW) rptviewcheck->setCheck(TRUE);
  if (permval & PERMLOGSUMSAVE) rptsavecheck->setCheck(TRUE);
  if (permval & PERMTKTQRY) tktviewcheck->setCheck(TRUE);
  if (permval & PERMTKTGEN) tktgencheck->setCheck(TRUE);
  if (permval & PERMTKTPRN) tktprncheck->setCheck(TRUE);
 
  if (!isPermitted(PERMEMPEDIT)){
    //cannot edit own capabilities
    tarifeditcheck->disable();
    tarifselcheck->disable();
    memeditcheck->disable();
    memalloccheck->disable();
    genconfcheck->disable();
    prodsellcheck->disable();
    prodeditcheck->disable();
    prodstockcheck->disable();
    cashrcvcheck->disable();
    casheditcheck->disable();
    cashcnclcheck->disable();
    cashdischeck->disable();
    rptviewcheck->disable();
    rptsavecheck->disable();
    tktviewcheck->disable();
    tktprncheck->disable();
    tktgencheck->disable();
  }

  if (dlg.execute() && e.usr.length() ) {
    //get permission value
    permval = 0;
    if (tarifeditcheck->getCheck()) permval |= PERMTARIFEDIT;
    if (tarifselcheck->getCheck()) permval |= PERMTARIFSELECT;
    if (memeditcheck->getCheck()) permval |= PERMMBREDIT;
    if (memalloccheck->getCheck()) permval |= PERMMBRALLOC;
    if (genconfcheck->getCheck()) permval |= PERMGENCONF;
    if (prodsellcheck->getCheck()) permval |= PERMPRODSELL;
    if (prodeditcheck->getCheck()) permval |= PERMPRODEDIT;
    if (prodstockcheck->getCheck()) permval |= PERMPRODSTOCK;
    if (cashrcvcheck->getCheck()) permval |= PERMCASHRECEIVE;
    if (casheditcheck->getCheck()) permval |= PERMCASHEDIT;
    if (cashdischeck->getCheck()) permval |= PERMCASHDISCOUNT;
    if (cashcnclcheck->getCheck()) permval |= PERMCASHCANCEL;
    if (rptviewcheck->getCheck()) permval |= PERMLOGSUMVIEW;
    if (rptsavecheck->getCheck()) permval |= PERMLOGSUMSAVE;
    if (tktviewcheck->getCheck()) permval |= PERMTKTQRY;
    if (tktprncheck->getCheck()) permval |= PERMTKTPRN;
    if (tktgencheck->getCheck()) permval |= PERMTKTGEN;
#ifdef DEBUG_CCLFOX
    printf("drawPermDialog(): Edited Employee ID = %d\n", editedemployee); 
#endif
    e.lvl = permval;
    retval = TRUE;
  }
  /*
  //encrypt the password
  char pass[256];
  unsigned char digest[CCL_MD5_DIGEST_LENGTH];
    
  strncpy(pass, pwdtf->getText().text(), 256);
  CCL_MD5((FXuchar*)pass, strlen(pass), digest);
  //now write this to data
  CCL_data_set_blob(CCL_DATA_EMPLOYEE, editedemployee, "password",  
		    digest, CCL_MD5_DIGEST_LENGTH);
  */
  return retval;
}

void 
EmployeesFrame::updateEmpCreds(int id, char *pwd, char *usr)
{

    //set password
    char pass[256];
    unsigned char digest[CCL_MD5_DIGEST_LENGTH];
    
    strncpy(pass, pwd, 256);
    CCL_MD5((FXuchar*)pass, strlen(pass), digest);
    //write this to data
    CCL_data_set_blob(CCL_DATA_EMPLOYEE, id, "password",  digest,
		      CCL_MD5_DIGEST_LENGTH);
    if (usr){
      //add employee name to data
      CCL_data_set_string(CCL_DATA_EMPLOYEE, id,
			"username", usr);
    }
}


long
EmployeesFrame::onEdit(FXObject*,FXSelector,void*)
{
  FXFoldingItem *eitem = employeeslist->findItemByData((void*)editedemployee);
  FXDialogBox dlg(this,_("Edit Staff Info"));
  EmpRecStr e;

  e.usr = CCL_employee_usrname_get(editedemployee);
  e.name = CCL_employee_name_get(editedemployee);
  e.email = CCL_employee_email_get(editedemployee);
  e.phone = CCL_employee_phone_get(editedemployee);
  e.pwd = CCL_employee_password_get(editedemployee);
  e.lvl = CCL_employee_usrlvl_get(editedemployee);
#ifdef DEBUG_CCLFOX
  printf("onEdit(): Edited Employee ID = %d\n", editedemployee); 
#endif
  if (drawPermDialog(dlg, e)) {
    eitem->setText( FXString(CCL_employee_usrname_get(editedemployee)) + 
		    _("\t") + _(CCL_employee_name_get(editedemployee)) + 
		    "\t" + CCL_employee_phone_get(editedemployee) + 
		    "\t"+ CCL_employee_email_get(editedemployee));
    employeeslist->updateItem(eitem);

    if (!strncmp(e.usr.text(), "admin", 10)) { 
      //      permval = 0xFFFFFFFF;
      e.lvl = 0xFFFFFFFF;
    }
    /*CCL_employee_info_set(editedemployee, 
			  (char *)pwdtf->getText().text(),
			  (char *)nametf->getText().text(),
			  (char *)phonetf->getText().text(), 
			  (char *)emailtf->getText().text(),
			  permval); */
    CCL_employee_info_set(editedemployee, 
			  (char *)e.pwd.text(),
			  (char *)e.name.text(),
			  (char *)e.phone.text(), 
			  (char *)e.email.text(),
			  e.lvl);
    updateEmpCreds(editedemployee, (char *)e.pwd.text(), (char *)NULL);
    //now update the dialogs
    readEmployee(editedemployee);
  }

  return 1;
}


long
EmployeesFrame::onEmployeeSelect(FXObject*,FXSelector,void*)
{
  FXFoldingItem *current = employeeslist->getCurrentItem();

  if (current) {
    editedemployee = (long) (employeeslist->getItemData(current));
    //usrlvlset = CCL_employee_usrlvl_get(editedemployee);
    readEmployee(editedemployee);
#ifdef DEBUG_CCLFOX
    printf("onEmployeeSelect(): id = %d\n");
#endif    
    //enable the edit button only for the employee
    if (!isPermitted(PERMEMPEDIT)){
      if (editedemployee == e_inf.empID)
	editbtn->enable();
      else
	editbtn->disable();
    }
  }

  return 1;
}

long
EmployeesFrame::onSetUsrlvl(FXObject*,FXSelector,void*)
{
  int usrlvl = usrlvlset;
  FXDialogBox dlg(this,_("Usrlvl"));
  FXVerticalFrame *vframe =
    new FXVerticalFrame(&dlg,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXVerticalFrame *tlistframe =
    new FXVerticalFrame(vframe,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
			0,0,0,0,0,0,0,0);
  FXFoldingList *tlist =
    new FXFoldingList(tlistframe,NULL,0,
		      LAYOUT_FILL_X|LAYOUT_FILL_Y|FOLDINGLIST_SINGLESELECT);
  new FXButton(vframe,_("Select"),dbIcon2,&dlg,FXDialogBox::ID_ACCEPT,
	       BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
  char *lvls[] = {"0\tAdmin", "1\tSupervisor", "2\tOperator"};
  
  tlist->appendHeader(_("ID"),NULL,40);
  tlist->appendHeader(_("Name"),NULL,180);
  dlg.resize(250,200);

  //tlist->appendItem(NULL,_("0\tNo special usrlvl"),NULL,NULL,(void*)0);
  
  //for (int i = 0, t; -1 != (t = CCL_usrlvl_get_nth(i)); i++) {
  for (int i = 0, t=1; i<3; i++, t++) {
    //const char *name = NULL;
    //char buf[256];

    //name = CCL_data_get_string(CCL_DATA_USRLVL,t,"name",NULL);
    //snprintf(buf,256,"%d\t%s",t,name);
    //CCL_free(name);
    tlist->appendItem(NULL,lvls[i],NULL,NULL,(void*)t);
  }

  FXFoldingItem *ctitem = tlist->findItemByData((void*)usrlvl);
  if (ctitem) tlist->selectItem(ctitem);

  if (dlg.execute()) {
    FXFoldingItem *sitem = tlist->getCurrentItem();

    if (sitem)
      usrlvlset = (long)(sitem->getData());
  }

  return 1;
}

long
EmployeesFrame::onResetPass(FXObject*,FXSelector,void*)
{
  FXuchar digest[CCL_MD5_DIGEST_LENGTH];
  char password[256];
  FXString pstr;

  if (FXInputDialog::getString(pstr,this,_("Set Password"),
			   _("Type the password: ")) && pstr.length()) {
    //bzero(pwdstr, 20);
    memset(pwdstr,0,20);
	strncpy(pwdstr, pstr.text(), 20);
  }

  snprintf(password,sizeof(password)/sizeof(char),"%d",editedemployee);
  CCL_MD5((FXuchar*)password,strlen(password),digest);
  CCL_data_set_blob(CCL_DATA_EMPLOYEE,editedemployee,"password",digest,
		    CCL_MD5_DIGEST_LENGTH);
  return 1;
}

long
EmployeesFrame::onFilter(FXObject*,FXSelector,void* ptr)
{
  readAllEmployees((const char *)ptr);

  return 1;
}

/* Load all employees into the list */
void
EmployeesFrame::loadEmployees()
{
  int employee = 0;

  for (FXuint i = 0; -1 != (employee = CCL_employee_get_nth(i)); i++){
    addEmployee(employee);
  }
}

/* Add employee data to the list */
void
EmployeesFrame::addEmployee(int id)
{
  FXFoldingItem *prnt = NULL;
  char *usr, *name, *pwd, *phone, *email;
  const char *phonep, *emailp;
  unsigned lvl=0, hdate=0, superid=0, flags=0;
  char buf[256];

  if (!CCL_employee_info_get(id, &usr, &name, &pwd, &phone, &email,
			     &lvl, &hdate, &superid, &flags))
    return;
#ifdef DEBUG_CCLFOX
  printf("addEmployee(): %s:  Flags: %08X\n", usr, flags);
#endif
  phonep = (phone == NULL) ? "[NA]": phone;
  emailp = (email == NULL) ? "[NA]": email;
  if (!(flags & EMPLOYEE_DELETED)){
    snprintf(buf,256,"%s\t%s\t%s\t%s\t%08X",usr,name,phonep,emailp, flags);
    employeeslist->appendItem(NULL, buf, NULL,NULL, (void *)id);
  }
  CCL_free(usr);
  CCL_free(name);
  CCL_free(pwd);
  CCL_free(phone);
  CCL_free(email);
}


long
EmployeesFrame::onNewEmployee(FXObject*,FXSelector,void*)
{
  FXDialogBox dlg(this,_("Staff Member Info"));
  EmpRecStr e = {"","","","","",0};

  if (drawPermDialog(dlg, e)) {
    //write in the database
    int id = CCL_employee_new((char *)e.usr.text(), (char *)e.name.text(), 
			      (char *)e.pwd.text(), (char *)e.phone.text(), 
			      (char *)e.email.text(), e.lvl, e_inf.empID);
    //update credentials
    updateEmpCreds(id, (char *)e.pwd.text(), (char *)e.usr.text());
    addEmployee(id);
#ifdef DEBUG_CCLFOX
    printf("onNewEmployee(): ID = %d: userID: %s\n", id, e.usr.text()); 
#endif
  }

  return 1;
}
