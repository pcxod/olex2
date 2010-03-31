//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TXGrowLine
// (c) Oleg V. Dolomanov, 2006
//----------------------------------------------------------------------------//
#include "xgrowline.h"
#include "gpcollection.h"
#include "asymmunit.h"
//----------------------------------------------------------------------------//
// TXGrowLine function bodies
//----------------------------------------------------------------------------//
TXGrowLine::TXGrowLine(TGlRenderer& r, const olxstr& collectionName, TSAtom *A, TCAtom* CA,
                         const smatd& transform) :
  TXBond(r, collectionName, *(TSBond*)NULL), Transform(transform)
{
  AGDrawObject::SetGroupable(false);
  vec3d C = transform * CA->ccrd();
  A->CAtom().GetParent()->CellToCartesian(C);

  FBase = A->crd();
  FEdge = C;

  C -= FBase;
  if( !C.IsNull() )  {
    Params()[3] = C.Length();
    C.Normalise();
    Params()[0] = (float)(acos(C[2])*180/M_PI);
    Params()[1] = -C[1];
    Params()[2] = C[0];
  }
  FSAtom = A;
  FCAtom = CA;
}
//..............................................................................
void TXGrowLine::Create(const olxstr& cName, const ACreationParams* cpar)  {
  TXBond::Create( cName );
}
//..............................................................................
TXGrowLine::~TXGrowLine()  {
  ;
}
//..............................................................................
bool TXGrowLine::Orient(TGlPrimitive& GlP)  {
  static olxstr Length;
  if( GlP.GetType() == sgloText )  {
    Length = olxstr::FormatFloat(3, Params()[3]);
    vec3d V = (FEdge+FBase)/2;
    V += Parent.GetBasis().GetCenter();
    V = Parent.GetBasis().GetMatrix()*V;
    olx_gl::rasterPos(V[0]+0.15, V[1]+0.15, V[2]+5);
    GlP.SetString(&Length);
    GlP.Draw();
    return true;
  }
  else  {
    olx_gl::translate(FBase);
    olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
    olx_gl::scale(Params()[4], Params()[4], Params()[3]);
  }
  return false;
} 
//..............................................................................
void TXGrowLine::Radius(float V)
{
  Params()[4] = V;
}
//..............................................................................
void TXGrowLine::Length(float V)
{
  Params()[3] = V;
}
//..............................................................................
