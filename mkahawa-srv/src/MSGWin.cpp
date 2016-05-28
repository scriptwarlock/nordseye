#include <ccls.h>
#include <fox-1.6/fx.h>
using namespace FX;
using namespace std;

#include "cclfox.h"
#include "MSGWin.h"


FXDEFMAP(MSGWin) MSGWinMap[] =
{
  FXMAPFUNC(SEL_COMMAND,MSGWin::ID_OKBTN,MSGWin::onCommand),
  FXMAPFUNC(SEL_COMMAND,MSGWin::ID_CANCELBTN,MSGWin::onCommand),
  FXMAPFUNC(SEL_TIMEOUT,MSGWin::ID_CHECKEVENTS,MSGWin::onCheckEvents),
};

FXIMPLEMENT(MSGWin,FXShell,MSGWinMap,ARRAYNUMBER(MSGWinMap))

MSGWin::MSGWin(FXApp * app)
:FXShell(app,0,0,0,180,100)
{
  FXVerticalFrame *vframe;
  FXHorizontalFrame *hframe1;
  FXHorizontalFrame *hframe2;
  FXVerticalFrame *vframe1;
  FXVerticalFrame *vframe2;
  FXVerticalFrame *vframe3;
 
  enable();
  vframe = new FXVerticalFrame(this,LAYOUT_FILL_Y|LAYOUT_FILL_X|FRAME_LINE,
			       0,0,180,100,0,0,0,0,0,0);
  hframe1 = new FXHorizontalFrame(vframe,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,
				  0,0,5,0,0,0,0,0);
  vframe1 = new FXVerticalFrame(hframe1,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0,
				0,0,0,0,0,0);
  FXLabel *label1 = new FXLabel(vframe1,_("Time:"),NULL,LABEL_NORMAL,
				3,0,0,0,0,0,0,0);
  timelbl = new FXLabel(vframe1,"00:00",NULL,
			   LAYOUT_FILL_Y|LAYOUT_FILL_X|FRAME_LINE);
  FXLabel *lable2 = new FXLabel(vframe1,_("From:"),NULL,LABEL_NORMAL,
				3,0,0,0,0,0,0,0);
  msgsrc = new FXLabel(vframe1,"00:00",NULL,
			   LAYOUT_FILL_Y|LAYOUT_FILL_X|FRAME_LINE);

  new FXSplitter(this,FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_FILL_Y|
			     SPLITTER_REVERSED,0,0,0,0);
  FXHorizontalFrame *infoframe =
    new FXHorizontalFrame(this,LAYOUT_FILL_X|LAYOUT_SIDE_BOTTOM|FRAME_SUNKEN,
			  0,0,0,0,0,0,0,0,10,0);

}

MSGWin::~MSGWin()
{

}

void
MSGWin::create()
{

}

long
MSGWin::onCommand(FX::FXObject*, unsigned int, void*)
{

 

}

long
MSGWin::onCheckEvents(FX::FXObject*, unsigned int, void*)
{


}

int
MSGWin::getDefaultHeight()
{


}

int
MSGWin::getDefaultWidth()
{


}

void 
enableOkBtn()
{


}

void 
disableCancelBtn()
{


}

void 
setMsg(const FXString &text)
{

}

void 
setMsgSource(const FXString &text)
{


}
 
void
setTime(const FXString &text)
{


}
