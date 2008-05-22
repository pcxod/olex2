#ifndef xlineH
#define xlineH

#include "xbond.h"
#include "xatom.h"

BeginGxlNamespace()

class TXLine: public TXBond  {
  TVPointD FBase, FEdge;
public:
  TXLine(const olxstr& collectionName, const TVPointD& base, const TVPointD& edge, TGlRender *Render);
  void Create(const olxstr& cName=EmptyString);
  virtual ~TXLine();

  TVPointD&  Base()  {  return FBase;  }
  TVPointD&  Edge()  {  return FEdge;  }

  bool GetDimensions(TVPointD &Max, TVPointD &Min){  return false; };

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data){  return true; };
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data){  return false; };
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data){  return false; };

  bool Orient(TGlPrimitive *P);
  void Radius(float V);
  inline double Radius()  {  return Params()[4]; }
  void Length(float V);
  inline double Length()  {  return Params()[3]; }
};

EndGxlNamespace()
#endif
 
