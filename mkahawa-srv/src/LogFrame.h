#ifndef LOGFRAME_H
#define LOGFRAME_H

enum CURLIST { 
  SESSLIST=0,
  SESSSUMLIST,
  PRODLIST,
  PRODSUMLIST,
  EXPENSELIST
};

class LogFrame : public FXVerticalFrame
{
FXDECLARE(LogFrame)
protected:
  FXSwitcher	  *listswitcher;
  FXFoldingList	  *sessionslist;
  FXFoldingList	  *productslist;
  FXFoldingList	  *expenseslist;
  FXFoldingList	  *clientslist;
  FXFoldingList	  *sumprodlist;
  FXTextField	  *stimetf;
  FXTextField	  *etimetf;
  FXTextField	  *sdatetf;
  FXTextField	  *edatetf;
  FXTextField	  *membertf;
  FXTextField	  *stafftf;
  FXToggleButton  *daybtn[7];
  FXTextField	  *strangetf;
  FXTextField	  *etrangetf;
  FXCheckButton	  *canceledcheck;
  FXCheckButton	  *summarycheck;
  FXButton	  *clearbtn;
  FXButton	  *refreshbtn;
  FXButton	  *resetbtn;
  FXButton	  *logexpensebtn;
  FXButton	  *saveonfilebtn;
  FXButton	  *saverptbtn;
  FXButton	  *savelogbtn;
  FXButton	  *sesssumbtn;
  FXButton	  *prodsumbtn;
  FXButton	  *expensebtn;
  FXLabel	  *totallbl;
  FXLabel	  *ptotallbl;
  FXLabel	  *stotallbl;
  FXLabel	  *etotallbl;
  FXLabel	  *ttotallbl;
  FXRealSpinner	  *startcashsp;
protected:
  double	   stotal;
  double	   ptotal;
  double	   etotal;
  long             ttotal;
  long             ctotal;
protected:
  LogFrame(){}
public:
  LogFrame(FXComposite *parent);
  ~LogFrame();
  void create();
public:
  void reset();
  void clear();
  void readLog();
  bool saveLog(const char *filename);
  bool saveReport(const char *filename);
  void setPerms(long perm);
  void readProductLogs();
  void readSessionLogs();
  void readExpenseLogs();
  void readProductSummary();
  void readSessionSummary();
  int  getLogQuery(CCL_log_search_rules *);
  void displaySummary();
  void clearSummary();
  void refreshList(enum CURLIST clist);
public:
  long onRefresh(FXObject*,FXSelector,void*);
  long onClear(FXObject*,FXSelector,void*);
  long onCheckValid(FXObject*,FXSelector,void*);
  long onReset(FXObject*,FXSelector,void*);
  long onVerify(FXObject* sender,FXSelector,void* ptr);
  long onSessionSelect(FXObject*,FXSelector,void*);
  long onSwitchToSessions(FXObject*,FXSelector,void*);
  long onSwitchToProducts(FXObject*,FXSelector,void*);
  long onSwitchToExpenses(FXObject*,FXSelector,void*);
  long onLogExpense(FXObject*,FXSelector,void*);
  long onSaveLog(FXObject*,FXSelector,void*);
  long onSaveReport(FXObject*,FXSelector,void*);
  long onSaveOnFile(FXObject*,FXSelector,void*);
  long onStartingCashChange(FXObject*,FXSelector,void* ptr);
  long onSortLogs(FXObject*,FXSelector,void* ptr);
public:
  enum {
    ID_REFRESH = FXVerticalFrame::ID_LAST,ID_CLEAR,ID_EXPENSES,
    ID_CHECKVALID,ID_RESET,ID_SESSIONLIST,ID_SESSIONS,ID_PRODUCTS,
    ID_LOGEXPENSE,ID_SAVELOG,ID_SAVEREPORT,ID_STARTCASH, ID_SORTLOG,
    ID_LAST
  };
};
#endif
