#ifndef xgrowlineH
#define xgrowlinedH

#include "xbond.h"

#include "catom.h"
#include "satom.h"

BeginGxlNamespace()

class TXGrowPoint : public AGDrawObject  {
  TMatrixD Transform;
  TVPointD Center;
public:
  TXGrowPoint(const olxstr& collectionName,
               const TVPointD& center, const TMatrixD& transform, TGlRender *Render);
  void Create(const olxstr& cName = EmptyString);
  virtual ~TXGrowPoint();

  bool GetDimensions(TVPointD &Max, TVPointD &Min);

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data){  return true; }
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data){  return false; }
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data){  return false; }

  bool Orient(TGlPrimitive *P);
  void SetRadius(float V);
  inline double Radius() {  return Params()[0]; }

  const TVPointD& GetCenter()     const {  return Center;  }
  const TMatrixD& GetTransform() const {  return Transform;  }
};

EndGxlNamespace()
#endif
 
