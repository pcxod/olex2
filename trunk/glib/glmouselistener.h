#ifndef __olx_gl_mouselistener_H
#define __olx_gl_mouselistener_H
#include "glbase.h"
#include "gdrawobject.h"
#include "ebasis.h"

BeginGlNamespace()

// TGlMouseListner flags
const uint16_t
  glmlMove2d    = 0x0001,
  glmlMoveable  = 0x0002,
  glmlRoteable  = 0x0004,
  glmlZoomable  = 0x0008,
  glmlMove2dz   = 0x0010;  // the translation is devided by current zoom

class TGlMouseListener: public AGDrawObject  {
protected:
  int SX, SY;
  short Flags;
public:
  TGlMouseListener(TGlRenderer& R, const olxstr& collectionName);
  virtual ~TGlMouseListener();
  TEBasis Basis;
  // Is/Set
  DefPropBFIsSet(Move2DZ,  Flags, glmlMove2dz)
  DefPropBFIsSet(Move2D,  Flags, glmlMove2d)
  DefPropBFIsSet(Moveable,  Flags, glmlMoveable)
  DefPropBFIsSet(Roteable,  Flags, glmlRoteable)
  DefPropBFIsSet(Zoomable,  Flags, glmlZoomable)

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data);
  bool OnDblClick(const IEObject *Sender, const TMouseData *Data)  {  return false;  }
};

EndGlNamespace()
#endif
