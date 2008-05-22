//---------------------------------------------------------------------------

#ifndef glmouselistenerH
#define glmouselistenerH
#include "glbase.h"
#include "gdrawobject.h"
#include "ebasis.h"

BeginGlNamespace()

// TGlMouseListner flags
const short  glmlMove2d    = 0x0001,
             glmlMoveable  = 0x0002,
             glmlRoteable  = 0x0004,
             glmlZoomable  = 0x0008;


class TGlMouseListener: public AGDrawObject
{
protected:
  int SX, SY;
  short Flags;
public:
  TGlMouseListener(const olxstr& collectionName, TGlRender *R);
  virtual ~TGlMouseListener();
  TEBasis Basis;

  inline bool Move2D() const {  return (Flags & glmlMove2d) ==  glmlMove2d; }
  virtual void Move2D(bool On){  SetBit(On, Flags, glmlMove2d); }

  inline bool Moveable() const {  return (Flags & glmlMoveable) == glmlMoveable; }
  virtual void Moveable(bool On){  SetBit(On, Flags, glmlMoveable); }

  inline bool Roteable() const {  return (Flags & glmlRoteable) == glmlRoteable; }
  virtual void Roteable(bool On){  SetBit(On, Flags, glmlRoteable); }

  inline bool Zoomable() const {  return (Flags & glmlZoomable) == glmlZoomable; }
  virtual void Zoomable(bool On){  SetBit(On, Flags, glmlZoomable); }

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data);
  bool OnDblClick(const IEObject *Sender, const TMouseData *Data)  {  return false;  }
};

EndGlNamespace()
#endif
