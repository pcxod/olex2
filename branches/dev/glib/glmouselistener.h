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


class TGlMouseListener: public AGDrawObject  {
protected:
  int SX, SY;
  short Flags;
public:
  TGlMouseListener(const olxstr& collectionName, TGlRenderer *R);
  virtual ~TGlMouseListener();
  TEBasis Basis;

  inline bool IsMove2D() const {  return (Flags & glmlMove2d) ==  glmlMove2d; }
  virtual void SetMove2D(bool On){  SetBit(On, Flags, glmlMove2d); }

  inline bool IsMoveable() const {  return (Flags & glmlMoveable) == glmlMoveable; }
  virtual void SetMoveable(bool On){  SetBit(On, Flags, glmlMoveable); }

  inline bool IsRoteable() const {  return (Flags & glmlRoteable) == glmlRoteable; }
  virtual void SetRoteable(bool On){  SetBit(On, Flags, glmlRoteable); }

  inline bool IsZoomable() const {  return (Flags & glmlZoomable) == glmlZoomable; }
  virtual void SetZoomable(bool On){  SetBit(On, Flags, glmlZoomable); }

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data);
  bool OnDblClick(const IEObject *Sender, const TMouseData *Data)  {  return false;  }
};

EndGlNamespace()
#endif
