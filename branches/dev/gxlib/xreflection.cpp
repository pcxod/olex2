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
TXReflection::TXReflection(const olxstr& collectionName, double minI, double maxI,
                            const TReflection& R, TAsymmUnit* au, TGlRenderer *P) :
  AGDrawObject(collectionName)
{
  FParent = P;
  hkl = vec3i( R.GetH(), R.GetK(), R.GetL() );
  I = R.GetI();
  // scaling has to be optimised, but as it is for now
  vec3d v(hkl), scaleV;
  scaleV[0] = scaleV[1] = scaleV[2] = 0.3;
  v = au->GetHklToCartesian()*v;
  scaleV = (au->GetHklToCartesian())*scaleV;
  double scale = olx_max(scaleV[2],  olx_max(scaleV[0], scaleV[1]) );
  v *= (2.0/scale);                    // extra scaling
  FCenter = v;
  Params().Resize(1);
  Params()[0] = (I - minI) / (maxI + olx_abs(minI));
  if( Params()[0] > 0 )  
    Params()[0] = sqrt( Params()[0] ); 
}
//..............................................................................
void TXReflection::Create(const olxstr& cName, const ACreationParams* cpar) {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);

  TGPCollection* GPC = FParent->FindCollection( GetCollectionName() );
  if( GPC == NULL )
    GPC = FParent->NewCollection( GetCollectionName() );
  GPC->AddObject(this);
  if( GPC->PrimitiveCount() != 0 )  return;

  TGraphicsStyle* GS = GPC->Style();
  TGlPrimitive* GlP = GPC->NewPrimitive("Reflection", sgloTriangles);

  TGlMaterial* GlM = const_cast<TGlMaterial*>(GS->Material("Reflection"));

  if( GlM->HasMark() )  {
    GlM->FromString("85;1.000,0.000,0.059,0.000;2138535799;1.000,1.000,1.000,0.500;36");
    GlM->SetMark(false);
  }
  GlP->SetProperties(GlM);
  double sz = 0.5;
  GlP->Data.Resize(4, 12);  // four triangles + normals
  static const double c_30 = sqrt(3.0)/2, s_30 = 0.5;
  TArrayList<vec3d> vecs;
  vecs.Add( vec3d(0, 0, sz) );
  vecs.Add( vec3d(0, sz, 0) );
  vecs.Add( vec3d(sz*c_30, -sz*s_30, 0) );
  vecs.Add( vec3d(-sz*c_30, -sz*s_30, 0) );
  GlP->Data[0][0] = vecs[0][0];  GlP->Data[0][1] = vecs[2][0];  GlP->Data[0][2] = vecs[1][0];
  GlP->Data[1][0] = vecs[0][1];  GlP->Data[1][1] = vecs[2][1];  GlP->Data[1][2] = vecs[1][1];
  GlP->Data[2][0] = vecs[0][2];  GlP->Data[2][1] = vecs[2][2];  GlP->Data[2][2] = vecs[1][2];
  vec3d n = vec3d(vecs[2]-vecs[0]).XProdVec(vecs[1]-vecs[0]).Normalise();
  GlP->Data[3][0] = n[0];  GlP->Data[3][1] = n[1];  GlP->Data[3][2] = n[2];  

  GlP->Data[0][3] = vecs[0][0];  GlP->Data[0][4] = vecs[3][0];  GlP->Data[0][5] = vecs[2][0];
  GlP->Data[1][3] = vecs[0][1];  GlP->Data[1][4] = vecs[3][1];  GlP->Data[1][5] = vecs[2][1];
  GlP->Data[2][3] = vecs[0][2];  GlP->Data[2][4] = vecs[3][2];  GlP->Data[2][5] = vecs[2][2];
  n = vec3d(vecs[3]-vecs[0]).XProdVec(vecs[2]-vecs[0]).Normalise();
  GlP->Data[3][0] = n[0];  GlP->Data[3][1] = n[1];  GlP->Data[3][2] = n[2];  

  GlP->Data[0][6] = vecs[0][0];  GlP->Data[0][7] = vecs[1][0];  GlP->Data[0][8] = vecs[3][0];
  GlP->Data[1][6] = vecs[0][1];  GlP->Data[1][7] = vecs[1][1];  GlP->Data[1][8] = vecs[3][1];
  GlP->Data[2][6] = vecs[0][2];  GlP->Data[2][7] = vecs[1][2];  GlP->Data[2][8] = vecs[3][2];
  n = vec3d(vecs[1]-vecs[0]).XProdVec(vecs[3]-vecs[0]).Normalise();
  GlP->Data[3][0] = n[0];  GlP->Data[3][1] = n[1];  GlP->Data[3][2] = n[2];  

  GlP->Data[0][9] = vecs[1][0];  GlP->Data[0][10] = vecs[2][0];  GlP->Data[0][11] = vecs[3][0];
  GlP->Data[1][9] = vecs[1][1];  GlP->Data[1][10] = vecs[2][1];  GlP->Data[1][11] = vecs[3][1];
  GlP->Data[2][9] = vecs[1][2];  GlP->Data[2][10] = vecs[2][2];  GlP->Data[2][11] = vecs[3][2];
  n = vec3d(vecs[2]-vecs[1]).XProdVec(vecs[3]-vecs[1]).Normalise();
  GlP->Data[3][0] = n[0];  GlP->Data[3][1] = n[1];  GlP->Data[3][2] = n[2];  
}
//..............................................................................
TXReflection::~TXReflection()  {  }
//..............................................................................
bool TXReflection::Orient(TGlPrimitive *GlP)  {
  Parent()->GlTranslate(FCenter);
  // scale the larger reflections up
  double scale1 = sqrt(atan(FParams[0])*2/M_PI);
  FParent->GlScale( (float)(1+scale1*6) );

  if( Selected() || !((TGlMaterial*)GlP->GetProperties())->GetTransparent() )  
    return false;

  TGlMaterial GlM = *(TGlMaterial*)GlP->GetProperties();

  double scale = (1.0-FParams[0]);
  GlM.AmbientF[3] = (float)scale;
  GlM.Init();
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
