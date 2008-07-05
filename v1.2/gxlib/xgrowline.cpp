//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TXGrowLine
// (c) Oleg V. Dolomanov, 2006
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "xgrowline.h"
#include "gpcollection.h"

#include "asymmunit.h"

#include "glscene.h"

//----------------------------------------------------------------------------//
// TXGrowLine function bodies
//----------------------------------------------------------------------------//
TXGrowLine::TXGrowLine(const olxstr& collectionName, TSAtom *A, TCAtom* CA,
                         const TMatrixD& transform, TGlRender *Render) :
  TXBond(collectionName, *(TSBond*)NULL, Render)
{
  TVPointD C;
  AGDrawObject::Groupable(false);
  C = CA->CCenter();
  C = transform * C;
  C[0] += transform[0][3];  C[1] += transform[1][3];  C[2] += transform[2][3];

  A->CAtom().GetParent()->CellToCartesian(C);

  FBase = A->Center();
  FEdge = C;

  C -= FBase;
  if( C.Length() != 0 )  {
    Params()[3] = C.Length();
    C.Normalise();
    Params()[0] = (float)(acos(C[2])*180/M_PI);
    Params()[1] = -C[1];
    Params()[2] = C[0];
  }
  FSAtom = A;
  FCAtom = CA;
  Transform = transform;
}
//..............................................................................
void TXGrowLine::Create(const olxstr& cName)  {
  if( cName.Length() != 0 )  SetCollectionName(cName);
  TXBond::Create( GetCollectionName() );
  TGPCollection* GPC = FParent->FindCollection( GetCollectionName() );
  GPC->AddObject(this);
}
//..............................................................................
TXGrowLine::~TXGrowLine()  {
  ;
}
//..............................................................................
bool TXGrowLine::Orient(TGlPrimitive *GlP)
{
  static olxstr Length;
  if( GlP->Type() == sgloText )  {
    Length = olxstr::FormatFloat(3, Params()[3]);
    TVPointD V;
    V = (FEdge+FBase)/2;
    V += FParent->GetBasis().GetCenter();
    V = FParent->GetBasis().GetMatrix()*V;
    glRasterPos3d(V[0]+0.15, V[1]+0.15, V[2]+5);
    GlP->String(&Length);
    GlP->Draw();
    return true;
  }
  else  {
    Parent()->GlTranslate(FBase);
    Parent()->GlRotate((float)Params()[0], (float)Params()[1], (float)Params()[2], 0.0);
    Parent()->GlScale((float)Params()[4], (float)Params()[4], (float)Params()[3]);
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
