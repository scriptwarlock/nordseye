#include <ccls.h>
#include <fox-1.6/fx.h>
using namespace FX;
using namespace std;

#include "cclfox.h"
#include "NotpaidFrame.h"
#include "CashingFrame.h"
#include "CCLWin.h"

//#define DEBUG 1

FXDEFMAP(NotpaidFrame) NotpaidFrameMap[] =
{
  FXMAPFUNC(SEL_SELECTED,NotpaidFrame::ID_NOTPAIDLIST,NotpaidFrame::onSelected)
};

FXIMPLEMENT(NotpaidFrame,FXVerticalFrame,NotpaidFrameMap,
	    ARRAYNUMBER(NotpaidFrameMap))

extern CCLWin *mainwin;
extern CashingFrame *cashingframe;

NotpaidFrame::NotpaidFrame(FXComposite * parent)
:FXVerticalFrame(parent,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN,
		0,0,0,0,0,0,0,0,0,0)
{
  notpaidlist = new FXFoldingList(this,this,ID_NOTPAIDLIST,
				  FOLDINGLIST_SINGLESELECT|LAYOUT_FILL_X|
				  LAYOUT_FILL_Y);
  notpaidlist->appendHeader(_("Client"),NULL,80);
  notpaidlist->appendHeader(_("Time"),NULL,60);
  notpaidlist->appendHeader(_("Start"),NULL,120);
  notpaidlist->appendHeader(_("End"),NULL,120);
}

NotpaidFrame::~NotpaidFrame()
{
}

void
NotpaidFrame::create()
{
  FXVerticalFrame::create();
}

void
NotpaidFrame::readNotPaid()
{
  CCL_log_search_rules sr;
  CCL_log_session_entry *se = NULL;
  int num = 0;

  sr.flags_not = PAID|CANCELED;
  sr.rulemask = CCL_SR_FLAGSNOT;
  num = CCL_log_sessions_get(&sr,&se);
  notpaidlist->clearItems();

  for (int i = 0; i < num; i++) {
    char entry[512];
    char ststr[64];
    char etstr[64];
    const char *cname = CCL_client_name_get(se[i].client);
    const char *mname = CCL_member_exists(se[i].member)
			  ? CCL_member_name_get(se[i].member) : "";

    strftime(ststr,64,"%d/%m/%y %H:%M",localtime(&(se[i].stime)));
    strftime(etstr,64,"%d/%m/%y %H:%M",localtime(&(se[i].etime)));
    snprintf(entry,512,"%s:%s\t%.2d:%.2d:%.2d\t%s\t%s",
	     cname,mname,se[i].time / 3600,(se[i].time % 3600) / 60,
	     (se[i].time % 3600) % 60,ststr,etstr);
    notpaidlist->prependItem(NULL,entry,NULL,NULL,
			      (void *)(se[i].id));
  }

  CCL_free(se);
}

long
NotpaidFrame::onSelected(FXObject*,FXSelector,void* ptr)
{
  FXFoldingItem* item = (FXFoldingItem*) ptr;
  long session = (long) item->getData();

  cashingframe->setSession(session);
  mainwin->showCashing();
  /* sideshow */
#ifdef DEBUG
  if (isUnpaid(0, session))
    printf ("Unpaid: Session %d\n", session);
  if (isUnpaid(0, session+1))
    printf ("Unpaid: Session %d\n", session+1);
#endif
  return 1;
}

bool
NotpaidFrame::isUnpaid(int client, int session)
{
  FXFoldingItem *item = notpaidlist->findItemByData((void *)session);
  if (item != NULL)
    return TRUE;

  return FALSE;
}
