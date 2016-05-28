#ifndef MEMBERSFRAME_H
#define MEMBERSFRAME_H

#define MEMBER_LOGGEDIN	(1<<18)	// This member is logged in
#define MEMBER_TICKET	(1<<17)	// This member is a ticket
#define MEMBER_DELETED	(1<<16)	// This member was deleted

class MembersFrame : public FXVerticalFrame
{
FXDECLARE(MembersFrame)
protected:
  FXTextField	 *filtertf;
  FXButton	 *settarif;
  FXButton	 *resetpass;
  FXButton	 *addmember;
  FXButton	 *delmember;
  FXButton	 *addcredit;
  FXButton	 *refundcredit;
  FXButton	 *applychanges;
  FXButton	 *btnTickets;
  FXFoldingList  *memberslist;
  FXTextField	 *nametf;
  FXTextField	 *emailtf;
  FXTextField	 *phonetf;
  FXTextField	 *logintf;
  FXLabel        *creditlbl;
private:
  long		  editedmember;
  long	          tarifset;
protected:
  MembersFrame(){}
public:
  MembersFrame(FXComposite *parent);
  ~MembersFrame();
  void create();
public:
  void readAllMembers(const char * filter = NULL);
  void readMember(int id);
  void clear();
  void setPerms(long perm);
public:
  long onAddMember(FXObject*,FXSelector,void*);
  long onDelMember(FXObject*,FXSelector,void*);
  long onApplyChanges(FXObject*,FXSelector,void*);
  long onMemberSelect(FXObject*,FXSelector,void*);
  long onSetTarif(FXObject*,FXSelector,void*);
  long onFilter(FXObject*,FXSelector,void* ptr);
  long onResetPass(FXObject*,FXSelector,void*);
  long onAddCredit(FXObject*,FXSelector,void*);
  long onSubCredit(FXObject*,FXSelector,void*);
  //  long onTickets(FXObject*,FXSelector,void*);
public:
  enum {
    ID_ADDMEMBER = FXVerticalFrame::ID_LAST, ID_DELMEMBER, ID_APPLY,
    ID_CHECKVALID, ID_MEMBERSLIST, ID_SETTARIF, ID_FILTER,
    ID_ADDCREDIT, ID_REFUNDCREDIT, ID_RESETPASS, ID_TICKETS,
    ID_LAST
  };
};
#endif  
