#include <fox-1.6/xincs.h>
#include <fox-1.6/fxver.h>
#include <fox-1.6/fxdefs.h>
#include <fox-1.6/fxkeys.h>
#include <fox-1.6/FXHash.h>
#include <fox-1.6/FXThread.h>
#include <fox-1.6/FXStream.h>
#include <fox-1.6/FXString.h>
#include <fox-1.6/FXSize.h>
#include <fox-1.6/FXPoint.h>
#include <fox-1.6/FXObjectList.h>
#include <fox-1.6/FXRectangle.h>
#include <fox-1.6/FXRegistry.h>
#include <fox-1.6/FXAccelTable.h>
#include <fox-1.6/FXApp.h>
#include <fox-1.6/FXDCWindow.h>
#include <fox-1.6/FXFont.h>
#include <fox-1.6/FXImage.h>
#include <fox-1.6/FXIcon.h>
#include <fox-1.6/FXButton.h>
#include <fox-1.6/FXScrollBar.h>
#include <fox-1.6/FXScrollArea.h>
#include <fox-1.6/FXHeader.h>
#include <fox-1.6/FXIconList.h>

#include "CCLIconItem.h"

FXIMPLEMENT(CCLIconItem,FXIconItem,NULL,0)

CCLIconItem::CCLIconItem(const FXString & text,FXIcon * bi,FXIcon * mi,
			 void * ptr,FXIcon * di)
:FXIconItem(text,bi,mi,ptr)
{
  disconIcon = di;
  flags = 0;
}

CCLIconItem::~CCLIconItem()
{
  disconIcon = (FXIcon *) - 1L;
}

void
CCLIconItem::create()
{
  FXIconItem::create();
  if (disconIcon)
    disconIcon->create();
}

void
CCLIconItem::draw(const FXIconList * list,FXDC & dc,FXint x,FXint y,
		  FXint w,FXint h) const
{
  register FXuint options = list->getListStyle();

  if (options & ICONLIST_BIG_ICONS)
    drawBigIcon(list,dc,x,y,w,h);
  else if (options & ICONLIST_MINI_ICONS)
    drawMiniIcon(list,dc,x,y,w,h);
  else
    drawDetails(list,dc,x,y,w,h);
  if (flags & FLAG_DISCONNECTED)
    drawFlagIcons(list,dc,x,y,w,h);
}

void
CCLIconItem::drawFlagIcons(const FXIconList * list,FXDC & dc,FXint x,
			   FXint y,FXint w,FXint h) const
{
  register FXuint options = list->getListStyle();

  if ((options & ICONLIST_BIG_ICONS) && (flags & FLAG_DISCONNECTED)
      && disconIcon && bigIcon) {
    FXint iw = disconIcon->getWidth();
    FXint ih = disconIcon->getHeight();
    FXint xi = x + (w - iw);
    FXint yi = y + bigIcon->getHeight() - ih;

    if (isSelected())
      dc.drawIconShaded(disconIcon,xi,yi);
    else
      dc.drawIcon(disconIcon,xi,yi);
  }
}

void
CCLIconItem::setShowDisconIcon(FXbool show)
{
  if (show)
    flags |= FLAG_DISCONNECTED;
  else
    flags &= ~FLAG_DISCONNECTED;
}
