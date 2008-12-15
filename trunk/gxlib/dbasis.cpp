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

TDBasis::TDBasis(const olxstr& collectionName, TGlRender *Render) : TGlMouseListener(collectionName, Render)  {
  Move2D(true);
  Moveable(true);
  Zoomable(true);
  Groupable(false);
}
void TDBasis::Create(const olxstr& cName, const CreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  ematd M, M1, M2;
  M.Assign(FAU->GetCellToCartesian(), 3, 3);
  evecd V, V1;
  TEBasis* EB;
  M1 = M;
  if( !M1[0].Length() )  M1[0][0] = 1;
    M1[0].Normalise();
  if( !M1[1].Length() )  M1[1][1] = 1;
    M1[1].Normalise();
  if( !M1[2].Length() )  M1[2][2] = 1;
    M1[2].Normalise();
  TGlMaterial GlM, GlM1;
  const TGlMaterial *SGlM;
  GlM.SetFlags(0);   GlM.ShininessF = 128;      GlM.SpecularF = 0x03030303;  GlM.SpecularF = 0x03030303;
  GlM1.SetFlags(0);  GlM1.ShininessF = 128;     GlM1.SpecularF = 0x03030303;  GlM1.SpecularB = 0x03030303;

  double ConeH = 0.8, ConeW = 0.2; // cylinder dimensions
  int CQ = 5; // cylinde quality
  TGPCollection* GPC = FParent->NewCollection( GetCollectionName() );
  GPC->AddObject(this);
  TGraphicsStyle* GS = GPC->Style();

  GlM.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|
                sglmAmbientB|sglmDiffuseB|sglmSpecularB);
  GlM1.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|  // non transluetn sphere
                sglmAmbientB|sglmDiffuseB|sglmSpecularB);

  TGlPrimitive* GlP = GPC->NewPrimitive("Sphere");  // a sphere at the basis of the object {0,0,0}
  GlM.AmbientF = 0x800f0f0f;
  GlP->SetProperties(&GlM1);
  GlP->Type(sgloSphere);
  GlP->Params()[0] = ConeW/1.5;  GlP->Params()[1] = 6;  GlP->Params()[2] = 6;

  GlM.AmbientF = 0x800000ff;
  M2 = M1;  M2.SwapRows(0, 1);  M2.SwapRows(1,2);
  GlP = GPC->NewPrimitive("ConeX");  // X cone
  SGlM = GS->Material("ConeX");
  if( !SGlM->Mark() )  GlP->SetProperties(SGlM);
  else                 GlP->SetProperties(&GlM);
  EB = new TEBasis;  GlP->Basis(EB);  EB->SetMatrix(M2);  EB->SetCenter( M[0]/5 );
  GlP->Type(sgloCylinder);
  GlP->Params()[0] = ConeW;  GlP->Params()[1] = 0;
  GlP->Params()[2] = ConeH; GlP->Params()[3] = CQ; GlP->Params()[4] = CQ;

  GlP = GPC->NewPrimitive("DiskX");  // X cone bottom
  SGlM = GS->Material("DiskX");
  if( !SGlM->Mark() )  GlP->SetProperties(SGlM);
  else                 GlP->SetProperties(&GlM);
  EB = new TEBasis;  GlP->Basis(EB);  EB->SetMatrix(M2);  EB->SetCenter( M[0]/5 );
  GlP->Type(sgloDisk);
  GlP->Params()[0] = 0;  GlP->Params()[1] = ConeW;
  GlP->Params()[2] = CQ; GlP->Params()[3] = CQ;
  GlP->QuadricOrientation(GLU_INSIDE);


  GlP = GPC->NewPrimitive("CylinderX");  // X axis
  SGlM = GS->Material("CylinderX");
  if( !SGlM->Mark() )  GlP->SetProperties(SGlM);
  else                 GlP->SetProperties(&GlM);
  GlP->SetProperties(&GlM);
  EB = new TEBasis;  GlP->Basis(EB);  EB->SetMatrix(M2);
  GlP->Type(sgloCylinder);
  GlP->Params()[0] = ConeW/2;  GlP->Params()[1] = ConeW/2;
  GlP->Params()[2] = M[0].Length()/5; GlP->Params()[3] = CQ; GlP->Params()[4] = CQ;


  GlM.AmbientF = 0x8000ff00;
  M2 = M1;  M2.SwapRows(0, 1);  M2.SwapRows(0, 2);
  GlP = GPC->NewPrimitive("ConeY");  // Y
  SGlM = GS->Material("ConeY");
  if( !SGlM->Mark() )  GlP->SetProperties(SGlM);
  else                 GlP->SetProperties(&GlM);
  GlP->SetProperties(&GlM);
  EB = new TEBasis;  GlP->Basis(EB);  EB->SetMatrix(M2);  EB->SetCenter( M[1]/5 );
  GlP->Type(sgloCylinder);
  GlP->Params()[0] = ConeW;  GlP->Params()[1] = 0;
  GlP->Params()[2] = ConeH; GlP->Params()[3] = CQ; GlP->Params()[4] = CQ;

  GlP = GPC->NewPrimitive("DiskY");  // Y cone bottom
  SGlM = GS->Material("DiskY");
  if( !SGlM->Mark() )  GlP->SetProperties(SGlM);
  else                 GlP->SetProperties(&GlM);
  EB = new TEBasis;  GlP->Basis(EB);  EB->SetMatrix(M2);  EB->SetCenter( M[1]/5 );
  GlP->Type(sgloDisk);
  GlP->Params()[0] = 0;  GlP->Params()[1] = ConeW;
  GlP->Params()[2] = CQ; GlP->Params()[3] = CQ;
  GlP->QuadricOrientation(GLU_INSIDE);

  GlP = GPC->NewPrimitive("CylinderY");  // y axis
  SGlM = GS->Material("CylinderY");
  if( !SGlM->Mark() )  GlP->SetProperties(SGlM);
  else                 GlP->SetProperties(&GlM);
  EB = new TEBasis;  GlP->Basis(EB);  EB->SetMatrix(M2);
  GlP->Type(sgloCylinder);
  GlP->Params()[0] = ConeW/2;  GlP->Params()[1] = ConeW/2;
  GlP->Params()[2] = M[1].Length()/5; GlP->Params()[3] = CQ; GlP->Params()[4] = CQ;

  GlM.AmbientF  = 0x80ff0000;
  GlP = GPC->NewPrimitive("ConeZ");  //Z cone
  SGlM = GS->Material("ConeZ");
  if( !SGlM->Mark() )  GlP->SetProperties(SGlM);
  else                 GlP->SetProperties(&GlM);
  EB = new TEBasis;  GlP->Basis(EB);  EB->SetMatrix(M1);  EB->SetCenter( M[2]/5 );
  GlP->Type(sgloCylinder);
  GlP->Params()[0] = ConeW;  GlP->Params()[1] = 0;
  GlP->Params()[2] = ConeH; GlP->Params()[3] = CQ; GlP->Params()[4] = CQ;

  GlP = GPC->NewPrimitive("DiskZ");  // Z cone bottom
  SGlM = GS->Material("DiskZ");
  if( !SGlM->Mark() )  GlP->SetProperties(SGlM);
  else                 GlP->SetProperties(&GlM);
  EB = new TEBasis;  GlP->Basis(EB);  EB->SetMatrix(M1);  EB->SetCenter( M[2]/5 );
  GlP->Type(sgloDisk);
  GlP->Params()[0] = 0;  GlP->Params()[1] = ConeW;
  GlP->Params()[2] = CQ; GlP->Params()[3] = CQ;
  GlP->QuadricOrientation(GLU_INSIDE);

  GlP = GPC->NewPrimitive("CylinderZ");  // Z axis
  SGlM = GS->Material("CylinderZ");
  if( !SGlM->Mark() )  GlP->SetProperties(SGlM);
  else                 GlP->SetProperties(&GlM);
  EB = new TEBasis;  GlP->Basis(EB);  EB->SetMatrix(M1);
  GlP->Type(sgloCylinder);
  GlP->Params()[0] = ConeW/2;  GlP->Params()[1] = ConeW/2;
  GlP->Params()[2] = M[2].Length()/5; GlP->Params()[3] = CQ; GlP->Params()[4] = CQ;

  GlP = GPC->NewPrimitive("Label");  // labels
  SGlM = GS->Material("Label");
  if( !SGlM->Mark() )  GlP->SetProperties(SGlM);
  else
  {
    GlM.SetIdentityDraw(true);
    GlM.SetTransparent(false);
    GlP->SetProperties(&GlM);
  }
  GlP->Type(sgloText);
  GlP->Font( Parent()->Scene()->DefFont() );
}
//..............................................................................
bool TDBasis::Orient(TGlPrimitive *P) {
  vec3d T;
  // extra zoom is very important for making pictures - it makes sure that the
  // object is translated to the right place!
  double EZoom = Parent()->GetExtraZoom();
  if( P->Type() == sgloText )  {
    olxstr Str;
    vec3d Center( Basis.GetCenter() );
    Center[0] = Center[0]*EZoom;
    Center[1] = Center[1]*EZoom;
    Center *= FParent->GetScale();
    Center = FParent->GetBasis().GetMatrix() * Center;
    //A
    T = FAU->GetCellToCartesian()[0];
    T /= 5;
    T[0] += 0.8;
    T *= Basis.GetZoom();
    T += Center;
    T *= FParent->GetBasis().GetMatrix();
    glRasterPos3d(T[0], T[1], T[2]+5);
    Str = "a";
    P->String(&Str);
    P->Draw();
    //B
    T = FAU->GetCellToCartesian()[1];
    T /= 5;
    T[1] += 0.8;
    T *= Basis.GetZoom();
    T += Center;
    T *= FParent->GetBasis().GetMatrix();
    glRasterPos3d(T[0], T[1], T[2]+5);
    Str = "b";
    P->String(&Str);
    P->Draw();
    //C
    T = FAU->GetCellToCartesian()[2];
    T /= 5;
    T[2] += 0.8;
    T *= Basis.GetZoom();
    T += Center;
    T *= FParent->GetBasis().GetMatrix();
    glRasterPos3d(T[0], T[1], T[2]+5);
    Str = "c";
    P->String(&Str);
    P->Draw();

    return true;
  }

  T = Basis.GetCenter();
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
 
