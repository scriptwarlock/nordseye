#include <ccls.h>
#include <fox-1.6/fx.h>
#include <fox-1.6/FXRex.h>
using namespace FX;

#include "cclfox.h"
#include "verifiers.h"
#include "EmployeesFrame.h"
#include "MembersFrame.h"

/*#define DEBUG 1*/

extern FXGIFIcon *dbIcon1;
extern FXGIFIcon *dbIcon2;
extern FXGIFIcon *dbIcon3;

FXDEFMAP(MembersFrame) MembersFrameMap[] = {
	FXMAPFUNC(SEL_COMMAND,MembersFrame::ID_ADDMEMBER,MembersFrame::onAddMember),
	FXMAPFUNC(SEL_COMMAND,MembersFrame::ID_DELMEMBER,MembersFrame::onDelMember),
	FXMAPFUNC(SEL_COMMAND,MembersFrame::ID_APPLY,MembersFrame::onApplyChanges),
	FXMAPFUNC(SEL_COMMAND,MembersFrame::ID_SETTARIF,MembersFrame::onSetTarif),
	FXMAPFUNC(SEL_COMMAND,MembersFrame::ID_ADDCREDIT,MembersFrame::onAddCredit),
	FXMAPFUNC(SEL_COMMAND,MembersFrame::ID_REFUNDCREDIT,MembersFrame::onSubCredit),
	FXMAPFUNC(SEL_COMMAND,MembersFrame::ID_RESETPASS,MembersFrame::onResetPass),
	//  FXMAPFUNC(SEL_COMMAND,MembersFrame::ID_TICKETS,MembersFrame::onTickets),
	FXMAPFUNC(SEL_CHANGED,MembersFrame::ID_FILTER,MembersFrame::onFilter),
	FXMAPFUNC(SEL_SELECTED,MembersFrame::ID_MEMBERSLIST,MembersFrame::onMemberSelect)
};

FXIMPLEMENT(MembersFrame,FXVerticalFrame,MembersFrameMap,
            ARRAYNUMBER(MembersFrameMap))

extern FXSettings *passwords;

MembersFrame::MembersFrame(FXComposite * parent)
	:FXVerticalFrame(parent,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN,
	                 0,0,0,0,0,0,0,0,0,0)
{
	FXVerticalFrame *membersframe =
	    new FXVerticalFrame(this,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
	                        0,0,0,0,0,0,0,0);
	memberslist =
	    new FXFoldingList(membersframe,this,ID_MEMBERSLIST,
	                      FOLDINGLIST_SINGLESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
	memberslist->appendHeader(_("User ID"),NULL,50);
	memberslist->appendHeader(_("Full Name"),NULL,200);
	FXHorizontalFrame *hframe1 = new FXHorizontalFrame(this,LAYOUT_FILL_X);

	/*  min10btn =
	  new FXButton(speedbar,_("\tGrant 10 Minutes"),min10icon,this,ID_10MIN,
		 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,
		 0,0,0,0,0,0,0,0);*/

	addmember = new FXButton(hframe1,_("New"),dbIcon1,this,ID_ADDMEMBER,
	                         BUTTON_TOOLBAR|FRAME_RAISED|FRAME_LINE);
	delmember = new FXButton(hframe1,_("Delete"),dbIcon1,this,ID_DELMEMBER,
	                         BUTTON_TOOLBAR|FRAME_RAISED|FRAME_LINE);
	settarif = new FXButton(hframe1,_("Tariff"),dbIcon1,this,ID_SETTARIF,
	                        BUTTON_TOOLBAR|FRAME_RAISED|FRAME_LINE);
	applychanges = new FXButton(hframe1,_("Save"),dbIcon1,this,ID_APPLY,
	                            BUTTON_TOOLBAR|FRAME_RAISED|FRAME_LINE);
	//  btnTickets = new FXButton(hframe1,_("Tickets"),dbIcon1,this,ID_TICKETS,
	//		   BUTTON_TOOLBAR|FRAME_RAISED|FRAME_LINE|LAYOUT_RIGHT);
	new FXHorizontalSeparator(this);
	new FXLabel(this,_("Member Details"),NULL,LAYOUT_CENTER_X);
	new FXHorizontalSeparator(this);

	FXHorizontalFrame *hframe2 = new FXHorizontalFrame(this,LAYOUT_FILL_X);
	new FXLabel(hframe2,_("Name:"));
	nametf = new FXTextField(hframe2,15,NULL,0,TEXTFIELD_NORMAL);

	creditlbl = new FXLabel(hframe2, _("Credit: "));
	FXHorizontalFrame *hframe3 = new FXHorizontalFrame(this,LAYOUT_FILL_X);
	new FXLabel(hframe3,_("Login ID:"));
	logintf = new FXTextField(hframe3,7,NULL,0,TEXTFIELD_NORMAL);
	new FXLabel(hframe3,_("Phone"));
	phonetf = new FXTextField(hframe3,12,NULL,0,TEXTFIELD_NORMAL);
	//FXHorizontalFrame *hframe5 = new FXHorizontalFrame(this,LAYOUT_FILL_X);
	FXHorizontalFrame *hframe5 = new FXHorizontalFrame(this,LAYOUT_FILL_X);
	new FXLabel(hframe5,_("Email:"));
	emailtf = new FXTextField(hframe5,30,NULL,0,TEXTFIELD_NORMAL);

	FXHorizontalFrame *hframe6 = new FXHorizontalFrame(this,LAYOUT_FILL_X);
	addcredit = new FXButton(hframe6,_(" Add Credit "),dbIcon3,this,ID_ADDCREDIT,
	                         BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
	refundcredit = new FXButton(hframe6,_("Refund Credit"),dbIcon3,this,ID_REFUNDCREDIT,
	                            BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
	resetpass = new FXButton(hframe6,_("Reset Password"),dbIcon3,this,ID_RESETPASS,
	                         BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT);
	editedmember = 0;
	tarifset = 0;
	clear();
}


MembersFrame::~MembersFrame()
{

}

void
MembersFrame::create()
{
	FXVerticalFrame::create();
}

void
MembersFrame::setPerms(long perm)
{
	if (!isPermitted(PERMMBREDIT)) {
		addmember->disable();
		delmember->disable();
		applychanges->disable();
		//btnTickets->disable();
		settarif->disable();
		resetpass->disable();
		addcredit->disable();
		refundcredit->disable();
	} else {
		addmember->enable();
		delmember->enable();
		applychanges->enable();
		//btnTickets->enable();
		settarif->enable();
		resetpass->enable();
		addcredit->enable();
		refundcredit->enable();
	}

	if (!isPermitted(PERMMBRALLOC)) {

	} else {

	}
}

void
MembersFrame::readAllMembers(const char * filter)
{
	long id;
	const char *name;
	char buf[128];
	FXString regexp = ".*";

	if (filter)
		regexp = regexp + filter + ".*";
	FXRex rex(regexp,REX_ICASE);

	memberslist->clearItems();
	for (FXuint i = 0; -1 != (id = CCL_member_get_nth(i)); i++) {
		if (!(CCL_member_flags_get(id) & (MEMBER_DELETED | MEMBER_TICKET))) {
			//if (!(CCL_member_flags_get(id) & (MEMBER_DELETED))) {
			name = CCL_member_name_get(id);
			if (rex.match(name)) {
				snprintf(buf,128,"%ld\t%s",id,name);
				memberslist->appendItem(NULL,buf,NULL,NULL,(void *)id);
			}
		}
	}
}

void
MembersFrame::readMember(int id)
{
	char buf[64];

	if (!CCL_member_exists(id))
		return;

	editedmember = (long)id;
	nametf->setText(CCL_member_name_get(id));
	emailtf->setText(CCL_member_email_get(id));
	phonetf->setText(CCL_member_other_get(id));
	snprintf(buf,64,"Credit: %.2f", (double)(CCL_member_credit_get(id)/100));
	creditlbl->setText(buf);

	char *login_name = CCL_data_get_string(CCL_DATA_MEMBER,id,"login_name",NULL);
	logintf->setText(login_name);
	CCL_free(login_name);

	tarifset = CCL_member_tarif_get(id);
}

void
MembersFrame::clear()
{
	nametf->setText("");
	emailtf->setText("");
	phonetf->setText("");
	creditlbl->setText("");
}

long
MembersFrame::onAddMember(FXObject*,FXSelector,void*)
{
	FXString name;

	if (FXInputDialog::getString(name,this,_("Add new member"),
	                             _("Type the name:")) && name.length()) {
		int id = CCL_member_new(name.text(), e_inf.empID);

		if (-1 != id) {
			CCL_member_flags_toggle(id,MEMBER_DELETED,FALSE);
			readAllMembers();
			readMember(id);
		} else
			FXMessageBox::error(this,MBOX_OK,_("Error"),
			                    _("The new member could not be created"));
	}

	return 1;
}

long
MembersFrame::onDelMember(FXObject*,FXSelector,void*)
{
	long id;

	FXFoldingItem *current = memberslist->getCurrentItem();
	if (current)
		id = (long) (memberslist->getItemData(current));
	else
		return 1;
	if (FXMessageBox::question(this,MBOX_YES_NO,_("Confirm"),
	                           _("Do you really want to delete this member?")) == MBOX_CLICKED_YES) {
		clear();
		CCL_member_flags_toggle(id,MEMBER_DELETED,TRUE);
		memberslist->removeItem(current);
	}
	return 1;
}

long
MembersFrame::onApplyChanges(FXObject*,FXSelector,void*)
{
	FXFoldingItem *mitem = memberslist->findItemByData((void*)editedmember);
	FXString newname = nametf->getText();
	FXString newlogin = logintf->getText();

	if (mitem) {
		newname.trim();
		newlogin.trim();
		CCL_member_tarif_set(editedmember,tarifset);
		if (!newname.empty() && -1 == CCL_member_find(newname.text()))
			CCL_member_name_set(editedmember,nametf->getText().text());
		CCL_member_email_set(editedmember,emailtf->getText().text());
		CCL_member_other_set(editedmember,phonetf->getText().text());
		if (newlogin.empty() && CCL_data_key_exists(CCL_DATA_MEMBER,editedmember,
		        "login_name"))
			CCL_data_key_delete(CCL_DATA_MEMBER,editedmember,"login_name");
		else if (-1 == CCL_data_find_by_key_sval(CCL_DATA_MEMBER,"login_name",
		         newlogin.text()))
			CCL_data_set_string(CCL_DATA_MEMBER,editedmember,"login_name",
			                    newlogin.text());
		mitem->setText(FXStringVal((FXint)editedmember) + "\t" + CCL_member_name_get(editedmember));
		memberslist->updateItem(mitem);
	}
#ifdef DEBUG
	printf("onApplyChanges(): Apply Changes Button was pressed\n");
#endif

	return 1;
}

/*#include "TicketsBox.h"

long
MembersFrame::onTickets(FXObject*,FXSelector,void*)
{
  TicketsBox  tBox(this);

#ifdef DEBUG
  printf("onTickets(): tickets button was pressed\n");
#endif
  tBox.execute();

  return 1;
}

*/

long
MembersFrame::onMemberSelect(FXObject*,FXSelector,void*)
{
	FXFoldingItem *current = memberslist->getCurrentItem();

	if (current) {
		editedmember = (long) (memberslist->getItemData(current));
		tarifset = CCL_member_tarif_get(editedmember);
		readMember(editedmember);
	}

	return 1;
}

long
MembersFrame::onSetTarif(FXObject*,FXSelector,void*)
{
	int tarif = tarifset;
	FXDialogBox dlg(this,_("Tariff"));
	FXVerticalFrame *vframe =
	    new FXVerticalFrame(&dlg,LAYOUT_FILL_X|LAYOUT_FILL_Y);
	FXVerticalFrame *tlistframe =
	    new FXVerticalFrame(vframe,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
	                        0,0,0,0,0,0,0,0);
	FXFoldingList *tlist =
	    new FXFoldingList(tlistframe,NULL,0,
	                      LAYOUT_FILL_X|LAYOUT_FILL_Y|FOLDINGLIST_SINGLESELECT);
	new FXButton(vframe,_("Select Tariff"),dbIcon3,&dlg,FXDialogBox::ID_ACCEPT,
	             FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT);

	tlist->appendHeader(_("ID"),NULL,40);
	tlist->appendHeader(_("Name"),NULL,180);
	dlg.resize(250,200);

	tlist->appendItem(NULL,_("0\tNo special tariff"),NULL,NULL,(void*)0);

	for (int i = 0, t; -1 != (t = CCL_tarif_get_nth(i)); i++) {
		const char *name = NULL;
		char buf[256];

		//name = CCL_data_get_string(CCL_DATA_TARIF,t,"name",NULL);
		name = CCL_tarif_name_get(t);
		snprintf(buf,256,"%d\t%s",t,name);
		CCL_free(name);
		tlist->appendItem(NULL,buf,NULL,NULL,(void*)(long)t);
	}

	FXFoldingItem *ctitem = tlist->findItemByData((void*)(long)tarif);
	if (ctitem) tlist->selectItem(ctitem);

	if (dlg.execute()) {
		FXFoldingItem *sitem = tlist->getCurrentItem();

		if (sitem)
			tarifset = (long)(sitem->getData());
	}

	return 1;
}

long
MembersFrame::onAddCredit(FXObject*,FXSelector,void*)
{
	FXdouble  amount = 0.0;
	long      id=0;
	int       retval;
	char      buf[64];

	FXFoldingItem *current = memberslist->getCurrentItem();
	if (current)
		id = (long) (memberslist->getItemData(current));
	else
		return 1;
	if (FXInputDialog::getReal(amount,this,_("Add Member Credit"),
	                           _("Amount: ")) && amount >= 0) {
		retval = CCL_pay_account(id, (int)(amount*100), e_inf.empID);
		snprintf(buf,64,"Credit: %.2f",(double)(CCL_member_credit_get(id) / 100));
		creditlbl->setText(buf);
	}
#ifdef DEBUG
	printf("onAddCredit(): [ID=%d, emp=%d] %s\n", id, e_inf.empID, buf);
#endif
	return 1;
}

long
MembersFrame::onSubCredit(FXObject*,FXSelector,void*)
{
	FXdouble  amount = 0.0;
	long      id;
	int       retval;
	char      buf[64];

	FXFoldingItem *current = memberslist->getCurrentItem();
	if (current)
		id = (long) (memberslist->getItemData(current));
	else
		return 1;

	if (current) {
		if (FXInputDialog::getReal(amount,this,_("Refund Member Credit"),
		                           _("Amount: ")) && amount >= 0) {
			retval = CCL_pay_account(id, (int)(-amount*100), e_inf.empID);
			if (retval < 0)
				FXMessageBox::error(this, MBOX_OK, _("Error"),
				                    _("Could not deduct the amount"));
		}
		snprintf(buf,64,"Credit: %.2f", (double)(CCL_member_credit_get(id)/100));
		creditlbl->setText(buf);
	}
#ifdef DEBUG
	printf("onSubCredit(): [current=%d] %s\n", id, buf);
#endif

	return 1;
}

long
MembersFrame::onResetPass(FXObject*,FXSelector,void*)
{
	FXuchar digest[CCL_MD5_DIGEST_LENGTH];
	char password[256];

	snprintf(password,sizeof(password)/sizeof(char),"%ld",editedmember);
	CCL_MD5((FXuchar*)password,strlen(password),digest);
	CCL_data_set_blob(CCL_DATA_MEMBER,editedmember,"password",digest,
	                  CCL_MD5_DIGEST_LENGTH);

#ifdef DEBUG
	printf("onResetPass(): [current=%d] %s\n", editedmember, password);
#endif

	return 1;
}

long
MembersFrame::onFilter(FXObject*,FXSelector,void* ptr)
{
	readAllMembers((const char *)ptr);

	return 1;
}
