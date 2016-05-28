#ifndef SETTINGSBOX_H
#define SETTINGSBOX_H

#define OPMODE_POSTPAID  1
#define OPMODE_TICKET    2
#define OPMODE_MEMBER    4

enum CYBOPMODE {  PREPAID=1, POSTPAID=2 };
enum LISTTYPE { SET_VIEWICONS=1, SET_VIEWLIST=2 };
enum ROUNDOFF { RND_00=1, RND_05=2, RND_50=3, RND_01=4, RND_10=5, RND_25=6 };

typedef unsigned long cyb_opmode_t;

typedef struct{
  cyb_opmode_t  cyber_op_mode;
  enum ROUNDOFF  round_off;
  enum LISTTYPE  list_type;
  char           currency[10];
  long           default_tariff;
  long           poll_interval;
  char           warnTime;
  char           warnCredit;
  long           warnTimes[5];
  long           warnCredits[5];

} cyber_settings_t;

class CyberSettings {
 public:
  cyb_opmode_t  cyber_op_mode;
  enum ROUNDOFF  round_off;
  enum LISTTYPE  list_type;
  char           currency[3];
  long           default_tariff;
  long           poll_interval;
  
 public:
  CyberSettings();
  void setPerms(long perm);
  void setOpMode(int opmode);
  void setRoundOff(int roundoff);
  void setCurrency(const char *currency);
  void setDefaultTariff(long default_tariff);
  void setPollInterval(long poll_interval);
  cyb_opmode_t getOpMode();
  enum ROUNDOFF  getRoundOff();
  char *getCurrency();
  long getDefaultTariff();
  long getPollInterval();

  int  loadSettings();
  int  saveSettings();

};


class SettingsBox : public FXDialogBox
{
FXDECLARE(SettingsBox)
protected:
  FXButton	 *btnSave;
  FXButton	 *btnApply;
  FXButton	 *btnSetTarif;
  FXButton	 *btnCancel;
  FXButton	 *btnExit;

  FXTextField	 *tfCurrency;
  FXTextField	 *tfPollItvl;
  FXLabel	 *lblTariff;


  FXCheckButton  *chkPostpaid;
  FXCheckButton  *chkTicket;
  FXCheckButton  *chkMember;

  FXRadioButton  *rbtnRnd00;
  FXRadioButton  *rbtnRnd50;
  FXRadioButton  *rbtnRnd25;
  FXRadioButton  *rbtnRnd05;
  FXRadioButton  *rbtnRnd01;
  FXRadioButton  *rbtnRnd10;

  FXDataTarget    rtgtRndOff;
private:
  CyberSettings    *cst;
  int              opMode;
  int              rndOff;
  bool             inpSaved;
  long             stariff;

 
protected:
  SettingsBox(){}
  int  loadSettings(CyberSettings &st);
  int  saveSettings(CyberSettings &st);
  int  getInputVals(CyberSettings *st);
  void dispInputVals(CyberSettings *st);
  void setDefaults();
  public:
  SettingsBox(FXComposite *parent);
  ~SettingsBox();
  void create();
public:
  int  loadSettings();
  void setPerms(long perm);
  void setOpMode(int opmode);
  void setRoundOff(int roundoff);
  void setCurrency(const char *currency);
  void setDefaultTariff(long default_tariff);
  void setPollInterval(long poll_interval);
  void setPrevious();

  cyb_opmode_t getOpMode();
  enum ROUNDOFF  getRoundOff();
  char *getCurrency();
  long getDefaultTariff();
  long getPollInterval();
  
public:
  long onCancel(FXObject*,FXSelector,void*);
  long onExit(FXObject*,FXSelector,void*);
  long onSave(FXObject*,FXSelector,void*);
  long onApply(FXObject*,FXSelector,void*);
  long onSetTarif(FXObject*,FXSelector,void*);
  long onCmdRadOpMode(FXObject*,FXSelector,void*);
  long onUpdRadOpMode(FXObject*,FXSelector,void*);
  long onCmdRadRndOff(FXObject*,FXSelector,void*);
  long onUpdRadRndOff(FXObject*,FXSelector,void*);
  long execute();
public:
  enum {
    ID_SAVE = FXDialogBox::ID_LAST, ID_CANCEL, ID_EXIT, ID_SETTARIF, ID_APPLY,
    ID_RADIO11,
    ID_RADIO12,
    ID_RADIO13,
    ID_RADIO14,
    ID_RADIO15,
    ID_LAST
  };
};

#endif
