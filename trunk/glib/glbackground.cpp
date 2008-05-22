//----------------------------------------------------------------------------//
// namespace TGlObj
// TDFrame - a drawing object for selection frame
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "glbackground.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "styles.h"

UseGlNamespace()
//..............................................................................
//..............................................................................

TGlBackground::TGlBackground(const olxstr& collectionName, TGlRender *Render, bool ceiling):
  AGDrawObject(collectionName)
{
  AGDrawObject::Parent(Render);
  FCeiling = ceiling;
  Groupable(false);
}
//..............................................................................
void TGlBackground::Create(const olxstr& cName)
{
  if( cName.Length() )  SetCollectionName(cName);
  TGlPrimitive *GlP;
  TGPCollection *GPC;
  TGlMaterial GlM;
  if( FCeiling )
    GlM.SetFlags(sglmAmbientF|sglmDiffuseF|sglmTransparent|sglmIdentityDraw);
  else
    GlM.SetFlags(sglmAmbientF|sglmIdentityDraw);
  GlM.AmbientF = 0x7f4f4f4f;
  GlM.DiffuseF = 0x7f4f4f4f;
  GlM.AmbientB = 0x7f4f4f4f;
  GlM.DiffuseB = 0x7f4f4f4f;

  GPC = FParent->FindCollection( GetCollectionName() );
  if( !GPC )    GPC = FParent->NewCollection( GetCollectionName() );
  GPC->AddObject(this);

  TGraphicsStyle *GS = GPC->Style();
  FColors[0] = GS->ParameterValue("A", 0xffffffff).ToInt();
  FColors[1] = GS->ParameterValue("B", 0xffffffff).ToInt();
  FColors[2] = GS->ParameterValue("C", 0).ToInt();
  FColors[3] = GS->ParameterValue("D", 0).ToInt();

  FPrimitive = GlP = GPC->NewPrimitive("Plane");
  GlP->SetProperties(&GlM);
  GlP->Type(sgloQuads);
  GlP->Data().Resize(4, 4);
  Orient(GlP);
}
//..............................................................................
bool TGlBackground::Orient(TGlPrimitive *P)  {
  if( !P ) return true;
  P = FPrimitive;
  ((TGlMaterial*)P->GetProperties())->Init();
  double Scale = Parent()->GetScale();
  double HW = Parent()->GetWidth()/2*Scale;
  double HH = Parent()->GetHeight()/2*Scale;
  double MaxZ = FParent->GetMaxRasterZ();
  MaxZ -= 0.01;
//  if( !MaxZ )  MaxZ = -0.0001;
  if( !FCeiling )  MaxZ = -MaxZ;

  P->Data()[0][0] = -HW;
  P->Data()[1][0] = -HH;
  P->Data()[2][0] = MaxZ;
  P->Data()[3][0] = FColors[0].GetRGB();

  P->Data()[0][1] = HW;
  P->Data()[1][1] = -HH;
  P->Data()[2][1] = MaxZ;
  P->Data()[3][1] = FColors[1].GetRGB();

  P->Data()[0][2] = HW;
  P->Data()[1][2] = HH;
  P->Data()[2][2] = MaxZ;
  P->Data()[3][2] = FColors[2].GetRGB();

  P->Data()[0][3] = -HW;
  P->Data()[1][3] = HH;
  P->Data()[2][3] = MaxZ;
  P->Data()[3][3] = FColors[3].GetRGB();
  return false;
}
//..............................................................................
void TGlBackground::LT(const TGlOption& v)  {
  Primitives()->Style()->SetParameter("A", v.GetRGB());
  FColors[0] = v;
}
//..............................................................................
void TGlBackground::RT(const TGlOption& v)  {
  Primitives()->Style()->SetParameter("B", v.GetRGB());
  FColors[1] = v;
}
//..............................................................................
void TGlBackground::RB(const TGlOption& v)  {
  Primitives()->Style()->SetParameter("C", v.GetRGB());
  FColors[2] = v;
}
//..............................................................................
void TGlBackground::LB(const TGlOption& v)  {
  Primitives()->Style()->SetParameter("D", v.GetRGB());
  FColors[3] = v;
}
//..............................................................................
