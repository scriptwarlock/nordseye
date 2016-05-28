#include <ccls.h>
#include <fox-1.6/fx.h>
using namespace FX;
using namespace std;

#include "cclfox.h"
#include "ProductsFrame.h"
#include "CashingFrame.h"
#include "verifiers.h"
#include "CCLWin.h"


extern FXGIFIcon *dbIcon01;
extern FXGIFIcon *dbIcon0;
extern FXGIFIcon *dbIcon1;
extern FXGIFIcon *dbIcon2;
extern FXGIFIcon *dbIcon3;

//extern FXGIFIcon *dbIcon4;
//extern FXGIFIcon *dbIcon5;
//extern FXGIFIcon *dbIcon6;

static void printTicket(const char *description, unsigned int amount);
static void openCashRegister();

void
ProductsFrame::noPermInfo()
{
    FXMessageBox::error(this,MBOX_OK,_("Permission"),
			_("Unable to access this feature.\n Contact the Administrator"));
    return;
}

void 
ProductsFrame::setPerms(long perm)
{
  if (!isPermitted(PERMPRODEDIT)){
    newbtn->disable();
    editbtn->disable();
  }
  else{
    newbtn->enable();
    editbtn->enable();
  }
  if (!isPermitted(PERMPRODSELL)){
    newsalebtn->disable();
    stockbtn->disable();
  }
  else{
    newsalebtn->enable();
    stockbtn->enable();
  }

  if (!isPermitted(PERMPRODSTOCK)){
    stockbtn->disable();
  }
  else{
    stockbtn->enable();
  }
}

FXDEFMAP(ProductsFrame) ProductsFrameMap[] =
{
  FXMAPFUNC(SEL_COMMAND,ProductsFrame::ID_NEWPRODUCT,
	    ProductsFrame::onNewProduct),
  FXMAPFUNC(SEL_COMMAND,ProductsFrame::ID_DELPRODUCT,
	    ProductsFrame::onDelProduct),
  FXMAPFUNC(SEL_COMMAND,ProductsFrame::ID_EDITPRODUCT,
	    ProductsFrame::onEditProduct),
  FXMAPFUNC(SEL_COMMAND,ProductsFrame::ID_NEWSALE,
	    ProductsFrame::onNewSale),
  FXMAPFUNC(SEL_COMMAND,ProductsFrame::ID_SETSTOCK,
	    ProductsFrame::onSetStock),
  FXMAPFUNC(SEL_DOUBLECLICKED,ProductsFrame::ID_PRODLIST,
	    ProductsFrame::onAddProduct),
  FXMAPFUNC(SEL_DOUBLECLICKED,ProductsFrame::ID_CLPRODLIST,
	    ProductsFrame::onSubProduct),
  /*  FXMAPFUNC(SEL_CHANGED,ProductsFrame::ID_CODE,ProductsFrame::onCodeChange),
  FXMAPFUNC(SEL_COMMAND,ProductsFrame::ID_ADDBYCODE,
  ProductsFrame::onAddByCode),*/
  FXMAPFUNC(SEL_COMMAND,ProductsFrame::ID_COMPLETESALE,
	    ProductsFrame::onCompleteSale)
};

FXIMPLEMENT(ProductsFrame,FXVerticalFrame,ProductsFrameMap,
	    ARRAYNUMBER(ProductsFrameMap))

extern CCLWin *mainwin;
extern CashingFrame *cashingframe;

static int _findProductByCode(const char *code);

int
pitemSortFunc(const FXFoldingItem * l,const FXFoldingItem * r)
{
  return compare(l->getText(),r->getText());
}

ProductsFrame::ProductsFrame(FXComposite * parent)
:FXVerticalFrame(parent,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN,
		0,0,0,0,0,0,0,0,0,0)
{
  ptoolbar = new FXToolBar(this,FRAME_RAISED|LAYOUT_TOP|LAYOUT_FILL_X);

  newbtn = new FXButton(ptoolbar,_("New"),dbIcon1,this,ID_NEWPRODUCT,
	       BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  delbtn = new FXButton(ptoolbar,_("Delete"),dbIcon1,this,ID_DELPRODUCT,
	       BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  editbtn = new FXButton(ptoolbar,_("Edit"),dbIcon1,this,ID_EDITPRODUCT,
	       BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  stockbtn = new FXButton(ptoolbar,_("Stock"),dbIcon1,this,ID_SETSTOCK,
	       BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  newsalebtn =
    new FXButton(ptoolbar,_("Sell"),dbIcon1,this,ID_NEWSALE,
		 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  completesalebtn =
    new FXButton(ptoolbar,_("Complete Sale"),dbIcon3,this,ID_COMPLETESALE,
		 BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  completesalebtn->disable();

  phsplitter = new FXSplitter(this,FRAME_SUNKEN|SPLITTER_VERTICAL|
			      SPLITTER_REVERSED|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  FXVerticalFrame *treeframe =
    new FXVerticalFrame(phsplitter,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
			0,0,0,0,0,0,0,0,0,0);
  prodlist = new FXFoldingList(treeframe,this,ID_PRODLIST,
			       FOLDINGLIST_SHOWS_BOXES|FOLDINGLIST_SHOWS_LINES|
			       FOLDINGLIST_ROOT_BOXES|FOLDINGLIST_SINGLESELECT|
			       LAYOUT_FILL_X|LAYOUT_FILL_Y);

  prodlist->appendHeader(_("Name"),NULL,180);
  prodlist->appendHeader(_("Price"),NULL,55);
  prodlist->appendHeader(_("Stock"),NULL,45);

  prodlist->setSortFunc(pitemSortFunc);

  FXVerticalFrame *listframe =
    new FXVerticalFrame(phsplitter,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y,
			0,0,0,0,0,0,0,0);

  clprodlist =
    new FXFoldingList(listframe,this,ID_CLPRODLIST,
		      FOLDINGLIST_SHOWS_BOXES|FOLDINGLIST_SHOWS_LINES|
		      FOLDINGLIST_ROOT_BOXES|FOLDINGLIST_SINGLESELECT|
		      LAYOUT_FILL_X|LAYOUT_FILL_Y);

  clprodlist->appendHeader(_("Name"),NULL,210);
  clprodlist->appendHeader(_("Quantity"),NULL,60);
  prodlist->setSortFunc(pitemSortFunc);
  /*
  FXHorizontalFrame *codeframe =
    new FXHorizontalFrame(listframe,FRAME_SUNKEN|LAYOUT_FILL_X,
			  0,0,0,0,0,0,0,0);
  new FXLabel(codeframe,_("Code:"));
  pcodetf = new FXTextField(codeframe,20,this,ID_CODE,TEXTFIELD_NORMAL);
  pamountsp = new FXSpinner(codeframe,3,NULL,0,FRAME_SUNKEN|FRAME_THICK);
  pamountsp->setRange(1,999);
  addbycodebtn = new FXButton(codeframe,_("Add"),NULL,this,ID_ADDBYCODE);
  addbycodebtn->disable();

  phsplitter->setSplit(1,200);
  */
  products[0].id = 0;
  onsale = FALSE;
}

ProductsFrame::~ProductsFrame()
{
}

void
ProductsFrame::create()
{
  FXVerticalFrame::create();
}

void
ProductsFrame::loadProducts()
{
  int product = 0;

  for (FXuint i = 0; -1 != (product = CCL_product_get_nth(i)); i++)
    addProduct(product);
}
void
ProductsFrame::addProduct(int id)
{
  FXFoldingItem *prnt = NULL;
  char *category;
  char *name;
  FXuint price;
  int stock;

  if (!CCL_product_info_get(id,&category,&name,&price))
    return;

  stock = CCL_product_stock_get(id);
  char buf[256];

  snprintf(buf,256,"%s\t%.2f\t%d",name,price / 100.0,stock);

  prnt = prodlist->findItem(category);
  if (!prnt)
    prnt = prodlist->prependItem(NULL,category);

  if (!prodlist->findItem(buf,prnt->getFirst()))
    prodlist->prependItem(prnt,buf,NULL,NULL,(void *)id);

  prodlist->sortItems();
  CCL_free(name);
  CCL_free(category);
}

void
ProductsFrame::delProduct(int id)
{
  char *category;

  if (!(CCL_product_info_get(id,&category,NULL,NULL)))
    return;

  FXFoldingItem *parent = prodlist->findItem(category);
  FXFoldingItem *child = NULL;

  CCL_free(category);
  if (parent) {
    child = parent->getFirst();
    while (child) {
      if (((long) (child->getData())) == id) {
	prodlist->removeItem(child);
	child = NULL;
      } else
	child = child->getNext();
    }

    if (!parent->getFirst())
      prodlist->removeItem(parent);
  }
}

void
ProductsFrame::updateClientProducts(int client)
{
  clprodlist->clearItems();
  onsale = FALSE;
  newsalebtn->enable();
  completesalebtn->disable();

  if (client) {
    char *category;
    char *name;
    int id;
    FXuint amount;

    for (int i = 0; CCL_client_product_get_nth(client,i,&id,&amount); i++) {
      if (CCL_product_info_get(id,&category,&name,NULL)) {
	char buf[256];
	
	snprintf(buf,256,"%s::%s\t%u",category,name,amount);
	clprodlist->prependItem(NULL,buf,NULL,NULL,(void *)id);
	CCL_free(category);
	CCL_free(name);
      }
    }
  }
  clprodlist->sortItems();
}

void
ProductsFrame::updateSaleProducts()
{
  clprodlist->clearItems();
  if (onsale) {
    char buf[256];

    for (int i = 0; 0 < products[i].id; i++) {
      int id = products[i].id;
      unsigned int amount = products[i].amount;
      char *category;
      char *name;
      unsigned int price = products[i].price;

      if (0 < amount && CCL_product_info_get(id,&category,&name,&price)) {
	snprintf(buf,256,"%s::%s\t%u\t%.2f",category,name,
		 amount,price / 100.0);
	clprodlist->prependItem(NULL,buf,NULL,NULL,(void*)id);
	CCL_free(category);
	CCL_free(name);
      }
    }
  }
}

long
ProductsFrame::onNewProduct(FXObject*,FXSelector,void*)
{
  FXString name;
  FXString category;
  double price = 0.0;
  FXString code;
  FXFoldingItem *current = prodlist->getCurrentItem();

  if (current) {
    if (current->getParent())
      category = current->getParent()->getText();
    else
      category = current->getText();
  }
  FXDataTarget nametgt(name);
  FXDataTarget categorytgt(category);
  FXDataTarget pricetgt(price);
  /*  FXDataTarget codetgt(code);*/

  FXDialogBox dlg(this,_("New Product"));
  FXVerticalFrame *vframe =
    new FXVerticalFrame(&dlg,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(vframe,_("Category:"));
  FXTextField *pcateg = new FXTextField(vframe,16,&categorytgt,
					FXDataTarget::ID_VALUE,
					FRAME_SUNKEN|FRAME_THICK);
  pcateg->setText(category);
  new FXLabel(vframe,_("Name:"));
  FXTextField *pname = new FXTextField(vframe,16,&nametgt,
				       FXDataTarget::ID_VALUE,
				       FRAME_SUNKEN|FRAME_THICK);
  new FXLabel(vframe,_("Price:"));
  FXRealSpinner *pprice = new FXRealSpinner(vframe,16,&pricetgt,
					    FXDataTarget::ID_VALUE,
					    FRAME_SUNKEN|FRAME_THICK);
  pprice->setRange(0,9999999.0);
  /*  new FXLabel(vframe,_("Code:"));
  FXTextField *pcode = new FXTextField(vframe,16,&codetgt,
				       FXDataTarget::ID_VALUE,
				       FRAME_SUNKEN|FRAME_THICK);*/
  new FXHorizontalSeparator(vframe);
  FXHorizontalFrame *hframe = new FXHorizontalFrame(vframe,LAYOUT_FILL_X);
  new FXButton(hframe,_("Cancel"),dbIcon1,&dlg,FXDialogBox::ID_CANCEL,
	       BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
  new FXButton(hframe,_("Accept"),dbIcon1,&dlg,FXDialogBox::ID_ACCEPT,
	       BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);

  if (dlg.execute() && category.length() && name.length() && price >= 0.0) {
    int id = CCL_product_new(category.text(),name.text(),(int)(price * 100));
    /*    if (0 < code.length()) {
      // FIXME: check if code already exists
      CCL_data_set_string(CCL_DATA_PRODUCT,id,"code",code.text());
    }
    */
    addProduct(id);
  }

  return 1;
}

long
ProductsFrame::onEditProduct(FXObject*,FXSelector,void*)
{
  FXFoldingItem *current = prodlist->getCurrentItem();

  if (current && current->getParent()) {
    long pid = (long) (current->getData());
    double price = 0.0;
    char *cod = CCL_data_get_string(CCL_DATA_PRODUCT,pid,"code",NULL);
    FXString code = cod;
    FXuint pri = 0;

    CCL_free(cod);

    CCL_product_info_get(pid,NULL,NULL,&pri);
    price = pri/100.0;

    FXDataTarget pricetgt(price);
    /*    FXDataTarget codetgt(code);*/

    FXDialogBox dlg(this,_("Edit Product"));
    FXVerticalFrame *vframe =
      new FXVerticalFrame(&dlg,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(vframe,_("Price:"));
    FXRealSpinner *pprice = new FXRealSpinner(vframe,16,&pricetgt,
					      FXDataTarget::ID_VALUE,
					      FRAME_SUNKEN|FRAME_THICK);
    pprice->setRange(0,9999999.0);
    /*    new FXLabel(vframe,_("Code:"));
    FXTextField *pcode = new FXTextField(vframe,16,&codetgt,
					 FXDataTarget::ID_VALUE,
					 FRAME_SUNKEN|FRAME_THICK);*/
    new FXHorizontalSeparator(vframe);
    FXHorizontalFrame *hframe = new FXHorizontalFrame(vframe,LAYOUT_FILL_X);
    new FXButton(hframe,_("Cancel"),dbIcon1,&dlg,FXDialogBox::ID_CANCEL,
		 BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);
    new FXButton(hframe,_("Accept"),dbIcon1,&dlg,FXDialogBox::ID_ACCEPT,
		 BUTTON_TOOLBAR|FRAME_RAISED|FRAME_THICK);

    if (dlg.execute() && price >= 0.0) {
      CCL_product_price_set(pid,(FXuint) (price * 100));
      /*if (0 < code.length()) {
	// FIXME: check if code already exists
	CCL_data_set_string(CCL_DATA_PRODUCT,pid,"code",code.text());
	}*/
      delProduct(pid);
      addProduct(pid);
    }
  }

  return 1;
}

long
ProductsFrame::onNewSale(FXObject*,FXSelector,void*)
{
  memset(products,0,sizeof(products));
  onsale = TRUE;
  newsalebtn->disable();
  completesalebtn->enable();
  updateSaleProducts();

  return 1;
}

long
ProductsFrame::onSetStock(FXObject*,FXSelector,void*)
{
  long pid;
  FXFoldingItem *current = prodlist->getCurrentItem();

  if (current && current->getParent()) {
    pid = (long) (current->getData());
    int oldamount = CCL_product_stock_get(pid);
    int amount = 0;

    if (FXInputDialog::getInteger(amount,this,_("Adjust Stock"),
				  _("Quantity to add:"), NULL)) {
      double cash = 0.0;
      if (FXInputDialog::getReal(cash,this,_("Log Expense"),
				 _("Enter the price (0 to not log):"),
				 NULL,0.0,999999.0)) {
	char description[128];
	char *pname = NULL;
	char *pcategory = NULL;

	// Register the action
	if (cash > 0.0) {
	  CCL_product_info_get(pid,&pcategory,&pname,NULL);
	  snprintf(description,128,_("Purchase: %s (%d)"),pname,oldamount);
	  CCL_log_expense(description,(FXuint) (cash * 100),0);
	  printTicket(description,(FXuint) (cash * 100));
	  openCashRegister();
	}
	// Set the new stock and update the list
	CCL_product_stock_set(pid,oldamount + amount);
	delProduct(pid);
	addProduct(pid);

	CCL_free(pname);
	CCL_free(pcategory);
      }
    }
  }

  return 1;
}

long
ProductsFrame::onDelProduct(FXObject*,FXSelector,void*)
{
  FXFoldingItem *current = prodlist->getCurrentItem();

  if (current) {
    FXFoldingItem *parent = current->getParent();
    if (parent) {
      long id = (long) current->getData();

      delProduct(id);
      CCL_product_delete(id);
    }
  }
  return 1;
}

long
ProductsFrame::onAddProduct(FXObject*,FXSelector,void* ptr)
{
  FXFoldingItem *child = (FXFoldingItem *) ptr;
  FXFoldingItem *prnt = child->getParent();
  int amount = 0;
  int client = onsale ? 0 : mainwin->getSelectedClient();

  if (!prnt || -1 == client)
    return 0;
  if (!isPermitted(PERMPRODSELL))
    return 0;
  if (FXInputDialog::getInteger(amount,this,_("Sell Products"),
				_("Quantity Sold:")) && amount >= 1) {
    long pid = (long) child->getData();
    unsigned int pprice;

    CCL_product_info_get(pid,NULL,NULL,&pprice);
    if (onsale) {
      int i;
      
      for (i = 0; 0 < products[i].id && products[i].id != pid; i++)
	;
      products[i].id = pid;
      products[i].amount += amount;
      products[i].price = pprice * products[i].amount;
      updateSaleProducts();
    } else {
      CCL_client_product_add(client,pid,amount);
      updateClientProducts(client);
      delProduct(pid);
      addProduct(pid);
    }
  }

  return 1;
}

long
ProductsFrame::onSubProduct(FXObject*,FXSelector,void* ptr)
{
  FXFoldingItem *item = (FXFoldingItem *) ptr;
  int amount = 0;
  int client = onsale ? 0 : mainwin->getSelectedClient();

  if (-1 == client)
    return 0;
  if (!isPermitted(PERMPRODSELL))
    return 0;
  if (FXInputDialog::getInteger(amount,this,_("Return Products"),
				_("Quantity Returned:")) && amount >= 1) {
    long pid = (long) item->getData();
    if (onsale) {
      int i;
      
      for (i = 0; 0 < products[i].id && products[i].id != pid; i++)
	;
      if (products[i].id == pid)
	products[i].amount = (amount >= products[i].amount)
			      ? 0 : products[i].amount - amount;

      updateSaleProducts();
    } else {
      CCL_client_product_sub(client,pid,amount);
      updateClientProducts(client);
      delProduct(pid);
      addProduct(pid);
    }
  }
  return 1;
}

long
ProductsFrame::onCodeChange(FXObject*,FXSelector,void* ptr)
{
  int id = _findProductByCode((const char *)ptr);
  
  if (-1 != id && CCL_DELETEDPRODUCT != CCL_product_stock_get(id))
    addbycodebtn->enable();
  else
    addbycodebtn->disable();
  return 1;
}

long
ProductsFrame::onAddByCode(FXObject*,FXSelector,void* ptr)
{
  unsigned int amount = pamountsp->getValue();
  const char *code = pcodetf->getText().text();
  int pid = _findProductByCode(code);
  int client = onsale ? 0 : mainwin->getSelectedClient();

  if (-1 == client)
    return 0;
  if (onsale) {
    int i;
    unsigned int pprice;

    CCL_product_info_get(pid,NULL,NULL,&pprice);
    for (i = 0; 0 < products[i].id && products[i].id != pid; i++)
      ;
    products[i].id = pid;
    products[i].amount += amount;
    products[i].price = pprice * products[i].amount;

  } 
  else {
    CCL_client_product_add(client,pid,amount);
    pcodetf->setText("");
    pamountsp->setValue(1);
    addbycodebtn->disable();
    updateClientProducts(client);
    delProduct(pid);
    addProduct(pid);
  }
  return 1;
}

long
ProductsFrame::onCompleteSale(FXObject*,FXSelector,void* ptr)
{
  onsale = FALSE;
  clprodlist->clearItems();
  completesalebtn->disable();
  newsalebtn->enable();
  cashingframe->setProductsSale(products);
  mainwin->showCashing();
#ifdef DEBUG
  printf("onCompleteSale(): Sale Completed.\n"); 
#endif
  return 1;
}

/********************************/

static int
_findProductByCode(const char *code)
{
  return CCL_data_find_by_key_sval(CCL_DATA_PRODUCT,"code",code);
}

static void
printTicket(const char *description, unsigned int amount)
{
  char buf[256];
  int ticketnum = 1 + CCL_data_get_int(CCL_DATA_NONE,0,
				       "ticket/number",0);
//  int last_day = CCL_data_get_int(CCL_DATA_NONE,0,
//				  "ticket/last_day",-1);
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);

//  if (-1 == last_day || tm->tm_yday != last_day) {
//    ticketnum = 1;
//    CCL_data_set_int(CCL_DATA_NONE,0,"ticket/last_day",tm->tm_yday);
//  }

  FXString tickettext = "";

  snprintf(buf,sizeof(buf)/sizeof(char),"%05d",ticketnum);

  tickettext += _("Ticket number:");
  tickettext += " ";
  tickettext += buf;
  tickettext += "\n";

  strftime(buf,sizeof(buf)/sizeof(char),"%H:%M:%S %d/%m/%Y",tm);

  tickettext += _("Date:");
  tickettext += " ";
  tickettext += buf;
  tickettext += "\n\n";

  tickettext += _("Description:");
  tickettext += " ";
  tickettext += description;
  tickettext += "\n\n";

  snprintf(buf,64,"$ %.2f",amount/100.0);

  tickettext += _("Cost:");
  tickettext += " ";
  tickettext += buf;
  tickettext += "\n\n\n\n\n";

#ifdef WIN32
  FILE *p = fopen("PRN","w");
  fwrite(tickettext.text(),sizeof(char),tickettext.length(),p);
  fclose(p);
#else
  FXString command = "lpr ";
  FXString filename = FXPath::unique("__ticket.txt");
  FILE *p = fopen(filename.text(),"w");
  
  command += filename;
  fwrite(tickettext.text(),sizeof(char),tickettext.length(),p);
  fclose(p);
  system(command.text());
//    FXFile::remove(filename);
#endif
  
  CCL_data_set_int(CCL_DATA_NONE,0,"ticket/number",ticketnum);
}

static void
openCashRegister()
{
#ifdef WIN32
  FILE *p = fopen("PRN","w");
  char code = 0x07;

  fwrite(&code,sizeof(code),1,p);
  fclose(p);
#endif
}
