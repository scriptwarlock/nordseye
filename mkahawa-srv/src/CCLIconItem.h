#ifndef CCLICONITEM_H
#define CCLICONITEM_H

#include <fox-1.6/FXIconList.h>

using namespace FX;

class CCLIconItem : public FXIconItem
{
FXDECLARE(CCLIconItem)
friend class FX::FXIconList;
protected:
  FXIcon *disconIcon;
  FXbool  flags;
protected:
  CCLIconItem() {}
  virtual void draw(const FXIconList *list,FXDC &dc,int x,int y,
		    int w,int h) const;
  virtual void drawFlagIcons(const FXIconList *list,FXDC &dc,int x,
			     int y,int w,int h) const;
protected:
  enum {
    FLAG_DISCONNECTED = (1<<0)
  };
public:
  CCLIconItem(const FXString &text,FXIcon *bi=NULL,FXIcon *mi = NULL,
	      void *ptr = NULL,FXIcon *di = NULL);
  virtual ~CCLIconItem();
  virtual void create();
  void setDisconIcon(FXIcon *icn) { disconIcon = icn; }
  FXIcon *getDisconIcon() const { return disconIcon; }
  void setShowDisconIcon(FXbool show = TRUE);
  FXbool getShowDisconIcon() { return flags & FLAG_DISCONNECTED; }
};

#endif
