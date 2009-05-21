//----------------------------------------------------------------------------//
// namespace TEXLib
// TDBasis - a drawing object for basis
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "dbasis.h"
#include "gpcollection.h"

#include "glrender.h"
#include "glscene.h"

#include "styles.h"

#include "glmaterial.h"
#include "glprimitive.h"

TDBasis::TDBasis(TGlRenderer& Render, const olxstr& collectionName) : 
TGlMouseListener(Render, collectionName)  
{
  SetMove2D(true);
  SetMoveable(true);
  SetZoomable(true);
  SetGroupable(false);
}
void TDBasis::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  olxstr NewL;
  TGPCollection* GPC = Parent.FindCollectionX( GetCollectionName(), NewL);
  if( GPC == NULL )
    GPC = &Parent.NewCollection(NewL);
  GPC->AddObject(*this);
  if( GPC->PrimitiveCount() != 0 )  return;

  ematd M, M2;
  M.Assign(FAU->GetCellToCartesian(), 3, 3);
  const ematd M1(M);
  for( int i=0; i < 3; i++ )  {
    if( M1[i].Length() == 0 )  
      M1[i][i] = 1;
    else
      M1[i].Normalise();
  }
  TGlMaterial GlM, GlM1;
  GlM.SetFlags(0);   
  GlM.ShininessF = 128;      
  GlM.SpecularF = 0x03030303;  
  GlM.SpecularF = 0x03030303;
  
  GlM1.SetFlags(0);  
  GlM1.ShininessF = 128;     
  GlM1.SpecularF = 0x03030303;  
  GlM1.SpecularB = 0x03030303;

  const double ConeH = 0.8, ConeW = 0.2; // cylinder dimensions
  const int CQ = 5; // cylinder quality
  TGraphicsStyle& GS = GPC->GetStyle();

  GlM.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|
                sglmAmbientB|sglmDiffuseB|sglmSpecularB);
  GlM1.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|  // non transluetn sphere
                sglmAmbientB|sglmDiffuseB|sglmSpecularB);

  TGlPrimitive* GlP = &GPC->NewPrimitive("Sphere", sgloSphere);  // a sphere at the basis of the object {0,0,0}
  GlM.AmbientF = 0x800f0f0f;
  GlP->SetProperties(GlM1);
  GlP->Params[0] = ConeW/1.5;  GlP->Params[1] = 6;  GlP->Params[2] = 6;

  GlM.AmbientF = 0x800000ff;
  M2 = M1;  M2.SwapRows(0, 1);  M2.SwapRows(1, 2);
  GlP = &GPC->NewPrimitive("ConeX", sgloCylinder);  // X cone
  GlP->SetProperties( GS.GetMaterial("ConeX", GlM) );
  TEBasis* EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M2);  EB->SetCenter( M[0]/5 );
  GlP->Params[0] = ConeW;  GlP->Params[1] = 0;
  GlP->Params[2] = ConeH; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlP = &GPC->NewPrimitive("DiskX", sgloDisk);  // X cone bottom
  GlP->SetProperties( GS.GetMaterial("DiskX", GlM) );
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M2);  EB->SetCenter( M[0]/5 );
  GlP->Params[0] = 0;  GlP->Params[1] = ConeW;
  GlP->Params[2] = CQ; GlP->Params[3] = CQ;
  GlP->SetQuadricOrientation(GLU_INSIDE);

  GlP = &GPC->NewPrimitive("CylinderX",sgloCylinder);  // X axis
  GlP->SetProperties( GS.GetMaterial("CylinderX", GlM) );
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M2);
  GlP->Params[0] = ConeW/2;  GlP->Params[1] = ConeW/2;
  GlP->Params[2] = M[0].Length()/5; GlP->Params[3] = CQ; GlP->Params[4] = CQ;


  GlM.AmbientF = 0x8000ff00;
  M2 = M1;  M2.SwapRows(0, 1);  M2.SwapRows(0, 2);
  GlP = &GPC->NewPrimitive("ConeY", sgloCylinder);  // Y
  GlP->SetProperties( GS.GetMaterial("ConeY", GlM) );
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M2);  EB->SetCenter( M[1]/5 );
  GlP->Params[0] = ConeW;  GlP->Params[1] = 0;
  GlP->Params[2] = ConeH; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlP = &GPC->NewPrimitive("DiskY", sgloDisk);  // Y cone bottom
  GlP->SetProperties( GS.GetMaterial("DiskY", GlM) );
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M2);  EB->SetCenter( M[1]/5 );
  GlP->Params[0] = 0;  GlP->Params[1] = ConeW;
  GlP->Params[2] = CQ; GlP->Params[3] = CQ;
  GlP->SetQuadricOrientation(GLU_INSIDE);

  GlP = &GPC->NewPrimitive("CylinderY", sgloCylinder);  // y axis
  GlP->SetProperties( GS.GetMaterial("CylinderY", GlM) );
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M2);
  GlP->Params[0] = ConeW/2;  GlP->Params[1] = ConeW/2;
  GlP->Params[2] = M[1].Length()/5; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlM.AmbientF  = 0x80ff0000;
  GlP = &GPC->NewPrimitive("ConeZ", sgloCylinder);  //Z cone
  GlP->SetProperties( GS.GetMaterial("ConeZ", GlM) );
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M1);  EB->SetCenter( M[2]/5 );
  GlP->Params[0] = ConeW;  GlP->Params[1] = 0;
  GlP->Params[2] = ConeH; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlP = &GPC->NewPrimitive("DiskZ", sgloDisk);  // Z cone bottom
  GlP->SetProperties( GS.GetMaterial("DiskZ", GlM) );
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M1);  EB->SetCenter( M[2]/5 );
  GlP->Params[0] = 0;  GlP->Params[1] = ConeW;
  GlP->Params[2] = CQ; GlP->Params[3] = CQ;
  GlP->SetQuadricOrientation(GLU_INSIDE);

  GlP = &GPC->NewPrimitive("CylinderZ", sgloCylinder);  // Z axis
  GlP->SetProperties( GS.GetMaterial("CylinderZ", GlM) );
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M1);
  GlP->Params[0] = ConeW/2;  GlP->Params[1] = ConeW/2;
  GlP->Params[2] = M[2].Length()/5; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlM.SetIdentityDraw(true);
  GlM.SetTransparent(false);
  GlP = &GPC->NewPrimitive("Label", sgloText);  // labels
  GlP->SetProperties( GS.GetMaterial("Label", GlM) );
  GlP->SetFont( Parent.GetScene().DefFont() );
}
//..............................................................................
bool TDBasis::Orient(TGlPrimitive& P) {
  // extra zoom is very important for making pictures - it makes sure that the
  // object is translated to the right place!
  const double EZoom = Parent.GetExtraZoom()*Parent.GetViewZoom();
  if( P.GetType() == sgloText )  {
    olxstr Str('a');
    const double scale = 1./Parent.GetScale();
    vec3d Center( Basis.GetCenter() );
    Center[0] = Center[0]*EZoom;
    Center[1] = Center[1]*EZoom;
    Center *= Parent.GetScale();
    Center = Parent.GetBasis().GetMatrix() * Center;
    const TGlFont& fnt = *Parent.GetScene().DefFont();
    for( int i=0; i < 3; i++ )  {
      vec3d T = FAU->GetCellToCartesian()[i];
      T /= 5;
      T[i] += 0.8;
      T *= Basis.GetZoom();
      T += Center;
      T *= Parent.GetBasis().GetMatrix();
      T *= scale;
      T[2] = Parent.GetMaxRasterZ();
      Str[0] = (char)('a'+i);
      Parent.DrawTextSafe(T, Str, fnt);
    }
    return true;
  }
  vec3d T = Basis.GetCenter();
  T[0] = T[0]*EZoom;
  T[1] = T[1]*EZoom;
  T *= Parent.GetScale();
  T = Parent.GetBasis().GetMatrix() * T;
  T -= Parent.GetBasis().GetCenter();
  Parent.GlTranslate(T);
  Parent.GlScale( (float)Basis.GetZoom() );
  return false;
}
//..............................................................................
 
