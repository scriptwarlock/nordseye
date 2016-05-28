#ifndef PRODUCTSFRAME_H
#define PRODUCTSFRAME_H

class ProductsFrame : public FXVerticalFrame
{
FXDECLARE(ProductsFrame)
protected:
  FXSplitter    *phsplitter;
  FXFoldingList *prodlist;
  FXFoldingList *clprodlist;
  FXToolBar	*ptoolbar;
  FXTextField	*pcodetf;
  FXSpinner	*pamountsp;
  FXButton	*addbycodebtn;
  FXButton	*newsalebtn;
  FXButton	*completesalebtn;
  FXButton      *newbtn;
  FXButton      *delbtn;
  FXButton      *editbtn;
  FXButton      *stockbtn;
protected:
  bool		 onsale;
  struct sale_item {
    int id;
    unsigned int amount;
    unsigned int price;
  }		 products[256];
protected:
  ProductsFrame(){}
public:
  ProductsFrame(FXComposite *parent);
  ~ProductsFrame();
  void create();
public:
  void loadProducts();
  void addProduct(int id);
  void delProduct(int id);
  void updateClientProducts(int client);
  void updateSaleProducts();
  void noPermInfo();
  void setPerms(long perm);
public:
  long onNewProduct(FXObject*,FXSelector,void*);
  long onDelProduct(FXObject*,FXSelector,void*);
  long onEditProduct(FXObject*,FXSelector,void*);
  long onNewSale(FXObject*,FXSelector,void*);
  long onSetStock(FXObject*,FXSelector,void*);
  long onAddProduct(FXObject*,FXSelector,void* ptr);
  long onSubProduct(FXObject*,FXSelector,void* ptr);
  long onCodeChange(FXObject*,FXSelector,void* ptr);
  long onAddByCode(FXObject*,FXSelector,void* ptr);
  long onCompleteSale(FXObject*,FXSelector,void* ptr);
public:
  enum {
    ID_NEWPRODUCT = FXVerticalFrame::ID_LAST,ID_DELPRODUCT,ID_SETSTOCK,
    ID_EDITPRODUCT,ID_NEWSALE,ID_PRODLIST,ID_CLPRODLIST,ID_CODE,
    ID_ADDBYCODE,ID_COMPLETESALE,
    ID_LAST
  };
};
#endif
