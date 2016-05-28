#ifndef EMPLOYEESFRAME_H
#define EMPLOYEESFRAME_H

typedef struct _EmpInfo{
  unsigned int empID;
  unsigned int lvl;
} EmpInfo;

typedef struct _FXEmpRec{
  FXString usr, name, pwd, email, phone;
  int lvl, superid;
} EmpRecStr;

extern EmpInfo e_inf;

class EmployeesFrame : public FXVerticalFrame
{
FXDECLARE(EmployeesFrame)
protected:
  FXTextField	 *filtertf;
  FXButton	 *setusrlvl;
  FXButton	 *resetpass;
  FXButton	 *addemployee;
  FXButton	 *delemployee;
  FXButton	 *applychanges;
  FXButton	 *editbtn;
  FXFoldingList  *employeeslist;
  FXTextField	 *nametf;
  FXTextField	 *emailtf;
  FXTextField	 *phonetf;
  FXTextField	 *logintf;
private:
  long		  editedemployee;
  long   	  usrlvlset;
protected:
  EmployeesFrame(){}
  void drawEmployeeDialog(FXDialogBox *, EmpRecStr *);
public:
  EmployeesFrame(FXComposite *parent);
  ~EmployeesFrame();
  void create();
  void setPerms(long perm);
  void noPermInfo();
public:
  void readAllEmployees(const char * filter = NULL);
  void readEmployee(int id);
  void clear();
  void addEmployee(int id);
  void loadEmployees();
  void updateEmpCreds(int id, char *pwd, char *usr);
  long drawPermDialog(FXDialogBox &dlg, EmpRecStr &e);
public:
  long onAddEmployee(FXObject*,FXSelector,void*);
  long onDelEmployee(FXObject*,FXSelector,void*);
  long onApplyChanges(FXObject*,FXSelector,void*);
  long onEdit(FXObject*,FXSelector,void*);
  long onEmployeeSelect(FXObject*,FXSelector,void*);
  long onSetUsrlvl(FXObject*,FXSelector,void*);
  long onFilter(FXObject*,FXSelector,void* ptr);
  long onResetPass(FXObject*,FXSelector,void*);
  long onNewEmployee(FXObject*,FXSelector,void*);
public:
  enum {
    ID_ADDEMPLOYEE = FXVerticalFrame::ID_LAST,ID_DELEMPLOYEE,ID_APPLY,
    ID_CHECKVALID,ID_EMPLOYEESLIST,ID_SETUSRLVL,ID_FILTER,ID_RESETPASS,
    ID_EDIT, ID_LAST
  };
};
#endif  
