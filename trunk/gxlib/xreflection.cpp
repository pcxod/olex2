//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TXReflection
// (c) Oleg V. Dolomanov, 2006
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "xreflection.h"
#include "gpcollection.h"

#include "glrender.h"

#include "styles.h"

#include "glmaterial.h"

//----------------------------------------------------------------------------//
// TXReflection function bodies
//----------------------------------------------------------------------------//
TXReflection::TXReflection(const olxstr& collectionName, THklFile& HklFile,
                            TReflection& R, TAsymmUnit* au, TGlRender *P) :
  AGDrawObject(collectionName)
{
  FParent = P;
  FReflection = &R;
  // scaling has to be optimised, but as it is for now
  vec3d v(R.GetH(), R.GetK(), R.GetL()), scaleV;
  scaleV[0] = scaleV[1] = scaleV[2] = 0.3;
  v = au->GetHklToCartesian()*v;
  scaleV = (au->GetHklToCartesian())*scaleV;
  double scale = olx_max(scaleV[2],  olx_max(scaleV[0], scaleV[1]) );
  v *= (2.0/scale);                    // extra scaling
  FCenter = v;
  Params().Resize(1);
  Params()[0] = (R.GetI() - HklFile.GetMinI()) / (HklFile.GetMaxI() + fabs(HklFile.GetMaxI()));
  if( Params()[0] > 0 )  Params()[0] = sqrt( Params()[0] ); 
}
//..............................................................................
void TXReflection::Create(const olxstr& cName)
{
  if( cName.Length() != 0 )  SetCollectionName(cName);
  TGlMaterial* GlM;
  TGlPrimitive *GlP;
  TGraphicsStyle *GS;
  TGPCollection *GPC;

  GPC = FParent->FindCollection( GetCollectionName() );
  if( !GPC )
    GPC = FParent->NewCollection( GetCollectionName() );
  else
  {
    if( GPC->PrimitiveCount() )
    {
      GPC->AddObject(this);
      return;
    }
  }
  GS = GPC->Style();
  GPC->AddObject(this);

  GlP = GPC->NewPrimitive("Reflection");

  GlM = const_cast<TGlMaterial*>(GS->Material("Reflection"));

  if( GlM->Mark() )
  {
    GlM->SetFlags( sglmAmbientF|sglmAmbientB);
    GlM->AmbientF = 0x000f00ff;
    GlM->DiffuseF = 0x000f00FF;
    GlM->AmbientB = 0x000f00FF;
    GlM->DiffuseB = 0x000f00FF;
    GlM->Mark(false);
  }
  GlP->SetProperties(GlM);
  double sz = 0.5;
/*
  GlP->Type(sgloQuads);
  GlP->Data().Resize(3, 12);  // three rectangles
  // xy
  GlP->Data()[0][0] = -sz;  GlP->Data()[1][0] = sz;
  GlP->Data()[0][1] = sz;  GlP->Data()[1][1] = sz;
  GlP->Data()[0][2] = sz;  GlP->Data()[1][2] = -sz;
  GlP->Data()[0][3] = -sz;  GlP->Data()[1][3] = -sz;
  // xz plane
  GlP->Data()[0][4] = -sz;  GlP->Data()[2][4] = sz;
  GlP->Data()[0][5] = sz;  GlP->Data()[2][5] = sz;
  GlP->Data()[0][6] = sz;  GlP->Data()[2][6] = -sz;
  GlP->Data()[0][7] = -sz;  GlP->Data()[2][7] = -sz;
  // zy plane x
  GlP->Data()[2][8] = -sz;  GlP->Data()[1][8] = sz;
  GlP->Data()[2][9] = sz;  GlP->Data()[1][9] = sz;
  GlP->Data()[2][10] = sz;  GlP->Data()[1][10] = -sz;
  GlP->Data()[2][11] = -sz;  GlP->Data()[1][11] = -sz;
  */
  GlP->Type(sgloTriangles);
  GlP->Data().Resize(3, 12);  // four rectangles
  double v = sqrt(3.0)/2;
  // bottom
  GlP->Data()[0][0] = 0;      GlP->Data()[1][0] = sz;
  GlP->Data()[0][1] = sz*v;   GlP->Data()[1][1] = -sz*v;
  GlP->Data()[0][2] = -sz*v;  GlP->Data()[1][2] = -sz*v;

  GlP->Data()[0][3] = 0;      GlP->Data()[1][3] = sz;
  GlP->Data()[0][4] = sz*v;   GlP->Data()[1][4] = -sz*v;
  GlP->Data()[0][5] = 0;      GlP->Data()[1][5] = 0;         GlP->Data()[2][5] = sz;

  GlP->Data()[0][6] = 0;      GlP->Data()[1][6] = sz;
  GlP->Data()[0][7] = -sz*v;  GlP->Data()[1][7] = -sz*v;
  GlP->Data()[0][8] = 0;      GlP->Data()[1][8] = 0;         GlP->Data()[2][8] = sz;

  GlP->Data()[0][9] = sz*v;   GlP->Data()[1][9] = -sz*v;
  GlP->Data()[0][10] = -sz*v; GlP->Data()[1][10] = -sz*v;
  GlP->Data()[0][11] = 0;     GlP->Data()[1][11] = 0;        GlP->Data()[2][11] = sz;

}
//..............................................................................
TXReflection::~TXReflection()  {  }
//..............................................................................
bool TXReflection::Orient(TGlPrimitive *GlP)
{
  Parent()->GlTranslate(FCenter);

  double scale1 = sqrt(atan(Params()[0])*2/M_PI);
  FParent->GlScale( (float)(1+scale1*6) );

  if( Selected() )  return false;
  TGlMaterial GlM;
  GlM = *(TGlMaterial*)GlP->GetProperties();

  double scale = 0.25 + Params()[0]*0.75;
  GlM.AmbientF *= scale;
  GlM.AmbientB *= scale;
  //GlM.Init();
  return false;
}
//..............................................................................
bool TXReflection::GetDimensions(vec3d &Max, vec3d &Min)
{
  Min = FCenter;
  Max = FCenter;
  return true;
};
//..............................................................................
