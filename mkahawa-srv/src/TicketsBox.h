#ifndef TICKETSBOX_H
#define TICKETSBOX_H

typedef struct  {
  time_t stDate;
  time_t expDate;
  int faceVal;
  int tariff;
  int tktNr;
  int dgtNr;
  int tktLife;
  char notes[25];
} ticket_input_t;

class TicketsBox : public FXDialogBox
{
FXDECLARE(TicketsBox)
protected:
  FXButton	 *btnGenerate;
  FXButton	 *btnPrint;
  FXButton	 *btnClear;
  FXButton	 *btnTariff;
  FXButton	 *btnSave;
  //  FXButton	 *btnEdit;
  FXButton	 *btnExit;
  FXFoldingList  *lstTickets;
  FXTextField	 *tfStartDate;
  FXTextField	 *tfExpDate;
  FXTextField	 *tfFaceVal;
  FXTextField	 *tfNum;
  FXTextField	 *tfDigitsNum;
  FXTextField    *tfLifeTime;
  FXTextField    *tfNotes;
  FXLabel        *lblStartDatPe;
  FXLabel        *lblExpDate;
  FXLabel        *lblFaceVal;
  FXLabel        *lblNum;
  FXLabel        *lblDigits;
  FXLabel        *lblTariff;
private:
  int		  listsavedflag;;
  int             listgenflag;
  long            stariff;
protected:
  TicketsBox(){}
public:
  TicketsBox(FXComposite *parent);
  ~TicketsBox();
  void create();
public:
  void setDefaults();
  void setPrevious();
  void clearList();
  void clearFields();
  void setPerms(long perm);
  void generateTickets(int nr, int dgtNr);
  long printTickets();
  long saveTickets();
  long addTicket(char *ticketStr, char *stexp, char *faceval, int tariff);
  int  ticket_in_db(char *tktStr);
  int  getInputVals(ticket_input_t *tkt);
public:
  long onGenerate(FXObject*,FXSelector,void*);
  long onPrint(FXObject*,FXSelector,void*);
  long onSave(FXObject*,FXSelector,void*);
  long onClear(FXObject*,FXSelector,void*);
  long onExit(FXObject*,FXSelector,void*);
  //long onEdit(FXObject*,FXSelector,void*);
  long onSetTarif(FXObject*,FXSelector,void*);
  long onTicketsList(FXObject*,FXSelector,void*);
public:
  enum {
    ID_GENERATE = FXDialogBox::ID_LAST, ID_PRINT, ID_CLEAR, //ID_TKTEDIT
    ID_SAVE, ID_SETTARIFF, ID_EXIT, ID_TICKETSLIST, ID_LAST
  };
};

int make_ddate_str(char *datestr, time_t *dt1, time_t *dt2);

#endif
