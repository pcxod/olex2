#ifndef xgrowlineH
#define xgrowlinedH

#include "xbond.h"

#include "catom.h"
#include "satom.h"

BeginGxlNamespace()

class TXGrowLine : public TXBond  {
  TSAtom *FSAtom;
  TCAtom *FCAtom;
  TMatrixD Transform;
  TVPointD  FEdge, FBase;
public:
  TXGrowLine(const olxstr& collectionName, TSAtom *A,
               TCAtom* CA, const TMatrixD& transform, TGlRender *Render);
  void Create(const olxstr& cName = EmptyString);
  virtual ~TXGrowLine();

  bool GetDimensions(TVPointD &Max, TVPointD &Min){  return false; };

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data){  return true; }
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data){  return false; }
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data){  return false; }

  bool Orient(TGlPrimitive *P);
  void Radius(float V);
  inline double Radius()     {  return Params()[4]; }
  void Length(float V);
  inline double Length()     {  return Params()[3]; }

  TSAtom *SAtom()      const {  return FSAtom;  }
  TCAtom *CAtom()      const {  return FCAtom;  }
  const TMatrixD& GetTransform()  const {  return Transform;  }
};

EndGxlNamespace()
#endif
 
