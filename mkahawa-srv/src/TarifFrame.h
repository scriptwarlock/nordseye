#ifndef TARIFFRAME_H
#define TARIFFRAME_H

class TarifFrame : public FXVerticalFrame
{
FXDECLARE(TarifFrame)
protected:
  FXButton	 *settarif;
  FXButton	 *addpart;
  FXButton	 *delpart;
  FXButton	 *applychanges;
  FXButton	 *newtarif;
  FXFoldingList  *tariflist;
  FXTextField	 *tnametf;
  FXTextField	 *hpricetf;
  FXTextField	 *stimetf;
  FXTextField	 *ipricetf;
  FXToggleButton *daybtn[7];
  FXButton	 *addprice;
  FXButton	 *delprice;
  FXFoldingList  *pricelist;
  FXSpinner	 *perminafter;
private:
  long    	  editedpart;
protected:
  TarifFrame(){}
public:
  TarifFrame(FXComposite *parent);
  ~TarifFrame();
  void create();
public:
  void readTarif();
  void readTarifPart(int id);
  void clear();
  void noPermInfo();
  void setPerms(long perm);
public:
  long checkValid(FXObject*,FXSelector,void*);
  long onAddPart(FXObject*,FXSelector,void*);
  long onDelPart(FXObject*,FXSelector,void*);
  long onApplyChanges(FXObject*,FXSelector,void*);
  long onAddPrice(FXObject*,FXSelector,void*);
  long onDelPrice(FXObject*,FXSelector,void*);
  long onPartSelect(FXObject*,FXSelector,void*);
  long onPerminSet(FXObject*,FXSelector,void*);
  long onVerify(FXObject* sender,FXSelector,void* ptr);
  long onSetTarif(FXObject*,FXSelector,void*);
  long onNewTarif(FXObject*,FXSelector,void*);
  long onTarifName(FXObject*,FXSelector,void*);
public:
  enum {
    ID_ADDPART = FXVerticalFrame::ID_LAST,ID_DELPART,ID_APPLY,ID_PERMIN,
    ID_ADDPRICE,ID_DELPRICE,ID_CHECKVALID,ID_PARTLIST,ID_STIME,
    ID_SETTARIF,ID_NEWTARIF,ID_TARIFNAME,
    ID_LAST
  };
};
#endif  
