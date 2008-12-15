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
                         const smatd& transform, TGlRender *Render) :
  TXBond(collectionName, *(TSBond*)NULL, Render)
{
  AGDrawObject::Groupable(false);
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
  Transform = transform;
}
//..............................................................................
void TXGrowLine::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
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
    vec3d V;
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
