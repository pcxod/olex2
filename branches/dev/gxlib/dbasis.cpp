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

TDBasis::TDBasis(const olxstr& collectionName, TGlRenderer *Render) : TGlMouseListener(collectionName, Render)  {
  Move2D(true);
  Moveable(true);
  Zoomable(true);
  Groupable(false);
}
void TDBasis::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  olxstr NewL;
  TGPCollection* GPC = FParent->CollectionX( GetCollectionName(), NewL);
  if( GPC == NULL )
    GPC = FParent->NewCollection(NewL);
  GPC->AddObject(this);
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
  const TGlMaterial* SGlM;
  GlM.SetFlags(0);   GlM.ShininessF = 128;      GlM.SpecularF = 0x03030303;  GlM.SpecularF = 0x03030303;
  GlM1.SetFlags(0);  GlM1.ShininessF = 128;     GlM1.SpecularF = 0x03030303;  GlM1.SpecularB = 0x03030303;

  const double ConeH = 0.8, ConeW = 0.2; // cylinder dimensions
  const int CQ = 5; // cylinder quality
  TGraphicsStyle* GS = GPC->Style();

  GlM.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|
                sglmAmbientB|sglmDiffuseB|sglmSpecularB);
  GlM1.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|  // non transluetn sphere
                sglmAmbientB|sglmDiffuseB|sglmSpecularB);

  TGlPrimitive* GlP = GPC->NewPrimitive("Sphere", sgloSphere);  // a sphere at the basis of the object {0,0,0}
  GlM.AmbientF = 0x800f0f0f;
  GlP->SetProperties(&GlM1);
  GlP->Params[0] = ConeW/1.5;  GlP->Params[1] = 6;  GlP->Params[2] = 6;

  GlM.AmbientF = 0x800000ff;
  M2 = M1;  M2.SwapRows(0, 1);  M2.SwapRows(1, 2);
  GlP = GPC->NewPrimitive("ConeX", sgloCylinder);  // X cone
  SGlM = GS->Material("ConeX");
  GlP->SetProperties(SGlM->HasMark() ? &GlM : SGlM);
  TEBasis* EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M2);  EB->SetCenter( M[0]/5 );
  GlP->Params[0] = ConeW;  GlP->Params[1] = 0;
  GlP->Params[2] = ConeH; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlP = GPC->NewPrimitive("DiskX", sgloDisk);  // X cone bottom
  SGlM = GS->Material("DiskX");
  GlP->SetProperties(SGlM->HasMark() ? &GlM : SGlM);
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M2);  EB->SetCenter( M[0]/5 );
  GlP->Params[0] = 0;  GlP->Params[1] = ConeW;
  GlP->Params[2] = CQ; GlP->Params[3] = CQ;
  GlP->SetQuadricOrientation(GLU_INSIDE);

  GlP = GPC->NewPrimitive("CylinderX",sgloCylinder);  // X axis
  SGlM = GS->Material("CylinderX");
  GlP->SetProperties(SGlM->HasMark() ? &GlM : SGlM);
  GlP->SetProperties(&GlM);
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M2);
  GlP->Params[0] = ConeW/2;  GlP->Params[1] = ConeW/2;
  GlP->Params[2] = M[0].Length()/5; GlP->Params[3] = CQ; GlP->Params[4] = CQ;


  GlM.AmbientF = 0x8000ff00;
  M2 = M1;  M2.SwapRows(0, 1);  M2.SwapRows(0, 2);
  GlP = GPC->NewPrimitive("ConeY", sgloCylinder);  // Y
  SGlM = GS->Material("ConeY");
  GlP->SetProperties(SGlM->HasMark() ? &GlM : SGlM);
  GlP->SetProperties(&GlM);
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M2);  EB->SetCenter( M[1]/5 );
  GlP->Params[0] = ConeW;  GlP->Params[1] = 0;
  GlP->Params[2] = ConeH; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlP = GPC->NewPrimitive("DiskY", sgloDisk);  // Y cone bottom
  SGlM = GS->Material("DiskY");
  GlP->SetProperties(SGlM->HasMark() ? &GlM : SGlM);
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M2);  EB->SetCenter( M[1]/5 );
  GlP->Params[0] = 0;  GlP->Params[1] = ConeW;
  GlP->Params[2] = CQ; GlP->Params[3] = CQ;
  GlP->SetQuadricOrientation(GLU_INSIDE);

  GlP = GPC->NewPrimitive("CylinderY", sgloCylinder);  // y axis
  SGlM = GS->Material("CylinderY");
  GlP->SetProperties(SGlM->HasMark() ? &GlM : SGlM);
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M2);
  GlP->Params[0] = ConeW/2;  GlP->Params[1] = ConeW/2;
  GlP->Params[2] = M[1].Length()/5; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlM.AmbientF  = 0x80ff0000;
  GlP = GPC->NewPrimitive("ConeZ", sgloCylinder);  //Z cone
  SGlM = GS->Material("ConeZ");
  GlP->SetProperties(SGlM->HasMark() ? &GlM : SGlM);
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M1);  EB->SetCenter( M[2]/5 );
  GlP->Params[0] = ConeW;  GlP->Params[1] = 0;
  GlP->Params[2] = ConeH; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlP = GPC->NewPrimitive("DiskZ", sgloDisk);  // Z cone bottom
  SGlM = GS->Material("DiskZ");
  GlP->SetProperties(SGlM->HasMark() ? &GlM : SGlM);
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M1);  EB->SetCenter( M[2]/5 );
  GlP->Params[0] = 0;  GlP->Params[1] = ConeW;
  GlP->Params[2] = CQ; GlP->Params[3] = CQ;
  GlP->SetQuadricOrientation(GLU_INSIDE);

  GlP = GPC->NewPrimitive("CylinderZ", sgloCylinder);  // Z axis
  SGlM = GS->Material("CylinderZ");
  GlP->SetProperties(SGlM->HasMark() ? &GlM : SGlM);
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(M1);
  GlP->Params[0] = ConeW/2;  GlP->Params[1] = ConeW/2;
  GlP->Params[2] = M[2].Length()/5; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlP = GPC->NewPrimitive("Label", sgloText);  // labels
  SGlM = GS->Material("Label");
  if( !SGlM->HasMark() )  GlP->SetProperties(SGlM);
  else  {
    GlM.SetIdentityDraw(true);
    GlM.SetTransparent(false);
    GlP->SetProperties(&GlM);
  }
  GlP->SetFont( Parent()->Scene()->DefFont() );
}
//..............................................................................
bool TDBasis::Orient(TGlPrimitive *P) {
  // extra zoom is very important for making pictures - it makes sure that the
  // object is translated to the right place!
  const double EZoom = FParent->GetExtraZoom()*FParent->GetViewZoom();
  if( P->GetType() == sgloText )  {
    olxstr Str('a');
    const double scale = 1./FParent->GetScale();
    vec3d Center( Basis.GetCenter() );
    Center[0] = Center[0]*EZoom;
    Center[1] = Center[1]*EZoom;
    Center *= FParent->GetScale();
    Center = FParent->GetBasis().GetMatrix() * Center;
    const TGlFont& fnt = *Parent()->Scene()->DefFont();
    for( int i=0; i < 3; i++ )  {
      vec3d T = FAU->GetCellToCartesian()[i];
      T /= 5;
      T[i] += 0.8;
      T *= Basis.GetZoom();
      T += Center;
      T *= FParent->GetBasis().GetMatrix();
      T *= scale;
      T[2] = FParent->GetMaxRasterZ();
      Str[0] = (char)('a'+i);
      FParent->DrawTextSafe(T, Str, fnt);
    }
    return true;
  }
  vec3d T = Basis.GetCenter();
  T[0] = T[0]*EZoom;
  T[1] = T[1]*EZoom;
  T *= Parent()->GetScale();
  T = Parent()->GetBasis().GetMatrix() * T;
  T -= Parent()->GetBasis().GetCenter();
  Parent()->GlTranslate(T);
  Parent()->GlScale( (float)Basis.GetZoom() );
  return false;
}
//..............................................................................
 
