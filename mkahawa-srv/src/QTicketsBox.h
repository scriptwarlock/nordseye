#ifndef QTICKETSBOX_H
#define QTICKETSBOX_H

#define QFACEVAL 1
#define QCURVAL  (1<<1)
#define QEXPDATE (1<<2)
#define QPRNDATE (1<<3)

typedef struct  {
  time_t prnDate1;
  time_t expDate1;
  time_t prnDate2;
  time_t expDate2;
  int faceVal1;
  int curVal1;
  int faceVal2;
  int curVal2;
  int tktType;  //all, expired, valid
  long qryType; //
} qticket_input_t;

typedef struct{
  int qtype;
  int val1;
  int val2;
  char qryStr[256];
} qticket_qry_t;
  

class QTicketsBox : public FXVerticalFrame
{
FXDECLARE(QTicketsBox)
protected:
  FXButton	 *btnQuery;
  FXButton	 *btnClear;
  FXButton	 *btnDelete;
  FXButton	 *btnPrint;
  //  FXButton	 *btnExit;
  FXButton	 *btnGenerate;
  FXFoldingList  *lstTickets;

  FXTextField	 *tfFaceVal1;
  FXTextField	 *tfFaceVal2;
  FXTextField	 *tfExpDate1;
  FXTextField	 *tfExpDate2;
  FXTextField	 *tfCurVal1;
  FXTextField	 *tfCurVal2;
  FXTextField	 *tfPrnDate1;
  FXTextField	 *tfPrnDate2;

  FXRadioButton  *rbtnAll;
  FXRadioButton  *rbtnExpired;
  FXRadioButton  *rbtnValid;

  FXCheckButton *chkFaceVal;
  FXCheckButton *chkExpDate;
  FXCheckButton *chkCurVal;
  FXCheckButton *chkPrnDate;
  FXDataTarget rtgt;
private:
  int		  ticketsqueried;
  qticket_qry_t   qqt;
  int             tktType;
protected:
  QTicketsBox(){}
public:
  QTicketsBox(FXComposite *parent);
  ~QTicketsBox();
  void create();
public:
  void setDefaults();
  void setPrevious();
  void clearList();
  void clearFields();
  void setPerms(long perm);
  void queryTickets();
  void deleteTickets();
  long printTickets();
  int  getInputVals(qticket_input_t *qtkt);
  int create_query(qticket_input_t *qtkt, qticket_qry_t *qry);
  int exec_query(qticket_qry_t *qry);
  int clear_query(qticket_qry_t *qry);
public:
  long onQuery(FXObject*,FXSelector,void*);
  long onClear(FXObject*,FXSelector,void*);
  long onPrint(FXObject*,FXSelector,void*);
  long onDelete(FXObject*,FXSelector,void*);
  long onGenerate(FXObject*,FXSelector,void*);
  //  long onExit(FXObject*,FXSelector,void*);
  long onCmdRadio(FXObject*,FXSelector,void*);
  long onUpdRadio(FXObject*,FXSelector,void*);

  long onTicketsList(FXObject*,FXSelector,void*);
public:
  enum {
    ID_QUERY = FXVerticalFrame::ID_LAST, ID_DELETE, ID_CLEAR,
    ID_GENERATE, ID_TICKETSLIST, ID_PRINT,//ID_EXIT,
    ID_RADIO1,
    ID_RADIO2,
    ID_RADIO3,
    ID_LAST
  };
  enum  {
    QTKT_ALL, QTKT_EXPIRED, QTKT_VALID
  };
};

#endif
