//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TXLine
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "xline.h"
#include "gpcollection.h"
#include "glscene.h"

#include "xatom.h"

//----------------------------------------------------------------------------//
// TXLine function bodies
//----------------------------------------------------------------------------//
TXLine::TXLine(const olxstr& collectionName, const vec3d& base, const vec3d& edge, TGlRender *Render): 
  TXBond(collectionName, *(TSBond*)NULL, Render)
{
  FBase = base;
  FEdge = edge;
  vec3d C( FEdge - FBase);
  if( !C.IsNull() )  {
    Params()[3] = C.Length();
    C.Normalise();
    Params()[0] = (float)(acos(C[2])*180/M_PI);
    Params()[1] = -C[1];
    Params()[2] = C[0];
  }
}
//..............................................................................
void TXLine::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);

  TGPCollection* GPC = FParent->FindCollection( GetCollectionName() );
  if( GPC == NULL )  {
    TXBond::Create();
    TGraphicsStyle *GS = Primitives()->Style();

    for( int i=0; i < Primitives()->PrimitiveCount(); i++ )  {
      TGlPrimitive* GlP = Primitives()->Primitive(i);
      TGlMaterial* GlM = const_cast<TGlMaterial*>(GS->Material(GlP->Name()));
      GlM->SetIdentityDraw( false );
      GlP->SetProperties(GlM);
    }
  }
  else  
    GPC->AddObject(this);
}
//..............................................................................
TXLine::~TXLine(){}
//..............................................................................
bool TXLine::Orient(TGlPrimitive *GlP)
{
  static olxstr Length;
  Length = olxstr::FormatFloat(3, Params()[3]);
  if( GlP->Type() == sgloText )
  {
    vec3d V;
    V = (FEdge+FBase)/2;
    V += FParent->GetBasis().GetCenter();
    V = FParent->GetBasis().GetMatrix()*V;
    glRasterPos3d(V[0]+0.15, V[1]+0.15, V[2]+5);
    GlP->String(&Length);
    GlP->Draw();
    return true;
  }
  else
  {
    Parent()->GlTranslate(FBase);
    Parent()->GlRotate((float)Params()[0], (float)Params()[1], (float)Params()[2], 0.0);
    Parent()->GlScale((float)Params()[4], (float)Params()[4], (float)Params()[3]);
  }
  return false;
} 
//..............................................................................
void TXLine::Radius(float V)
{
  Params()[4] = V;
}
//..............................................................................
void TXLine::Length(float V)
{
  Params()[3] = V;
}
//..............................................................................
