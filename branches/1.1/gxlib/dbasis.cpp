//----------------------------------------------------------------------------//
// TDBasis - a drawing object for basis
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#include "dbasis.h"
#include "gpcollection.h"
#include "glrender.h"
#include "styles.h"
#include "glmaterial.h"
#include "glprimitive.h"
#include "pers_util.h"

TDBasis::TDBasis(TGlRenderer& Render, const olxstr& collectionName) : 
TGlMouseListener(Render, collectionName)  
{
  SetMove2D(true);
  SetMoveable(true);
  SetZoomable(true);
  SetSelectable(false);
}
//..............................................................................
TDBasis::~TDBasis()  {}
//..............................................................................
void TDBasis::SetAsymmUnit(TAsymmUnit& au)  {  AU = &au;  }
//..............................................................................
void TDBasis::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;
  TGraphicsStyle& GS = GPC.GetStyle();
  olxstr& str_mask = GS.GetParam(GetPrimitiveMaskName(), "3", true);
  int PMask = str_mask.ToInt();
  if( PMask == 0 )  {
    PMask = 3;
    str_mask = '3';
  }

  mat3d m = AU->GetCellToCartesian();
  if( m[0].QLength() < 1.e-6 )
    m.I();
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

  GlM.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|
                sglmAmbientB|sglmDiffuseB|sglmSpecularB);
  GlM1.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|  // non transluetn sphere
                sglmAmbientB|sglmDiffuseB|sglmSpecularB);

  TGlPrimitive* GlP = &GPC.NewPrimitive("Sphere", sgloSphere);  // a sphere at the basis of the object {0,0,0}
  GlM.AmbientF = 0x800f0f0f;
  GlP->SetProperties(GlM1);
  GlP->Params[0] = ConeW/1.5;  GlP->Params[1] = 6;  GlP->Params[2] = 6;

  GlM.AmbientF = 0x800000ff;
  GlP = &GPC.NewPrimitive("ConeX", sgloCylinder);  // X cone
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  TEBasis* EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(TEBasis::CalcBasis<vec3d,mat3d>(m[0]));  EB->SetCenter(m[0]/5);
  GlP->Params[0] = ConeW;  GlP->Params[1] = 0;
  GlP->Params[2] = ConeH; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlP = &GPC.NewPrimitive("DiskX", sgloDisk);  // X cone bottom
  GlP->SetProperties( GS.GetMaterial(GlP->GetName(), GlM) );
  GlP->SetBasis(new TEBasis(*EB));
  GlP->Params[0] = 0;  GlP->Params[1] = ConeW;
  GlP->Params[2] = CQ; GlP->Params[3] = CQ;
  GlP->SetQuadricOrientation(GLU_INSIDE);

  GlP = &GPC.NewPrimitive("CylinderX",sgloCylinder);  // X axis
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  EB = new TEBasis(*EB);  GlP->SetBasis(EB);  EB->NullCenter();
  GlP->Params[0] = ConeW/2;  GlP->Params[1] = ConeW/2;
  GlP->Params[2] = m[0].Length()/5; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlM.AmbientF = 0x8000ff00;
  GlP = &GPC.NewPrimitive("ConeY", sgloCylinder);  // Y
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(TEBasis::CalcBasis<vec3d,mat3d>(m[1]));  EB->SetCenter(m[1]/5);
  GlP->Params[0] = ConeW;  GlP->Params[1] = 0;
  GlP->Params[2] = ConeH; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlP = &GPC.NewPrimitive("DiskY", sgloDisk);  // Y cone bottom
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  GlP->SetBasis(new TEBasis(*EB));
  GlP->Params[0] = 0;  GlP->Params[1] = ConeW;
  GlP->Params[2] = CQ; GlP->Params[3] = CQ;
  GlP->SetQuadricOrientation(GLU_INSIDE);

  GlP = &GPC.NewPrimitive("CylinderY", sgloCylinder);  // y axis
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  EB = new TEBasis(*EB);  GlP->SetBasis(EB);  EB->NullCenter();
  GlP->Params[0] = ConeW/2;  GlP->Params[1] = ConeW/2;
  GlP->Params[2] = m[1].Length()/5; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlM.AmbientF  = 0x80ff0000;
  GlP = &GPC.NewPrimitive("ConeZ", sgloCylinder);  //Z cone
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(TEBasis::CalcBasis<vec3d,mat3d>(m[2]));  EB->SetCenter(m[2]/5);
  GlP->Params[0] = ConeW;  GlP->Params[1] = 0;
  GlP->Params[2] = ConeH; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlP = &GPC.NewPrimitive("DiskZ", sgloDisk);  // Z cone bottom
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  GlP->SetBasis(new TEBasis(*EB));
  GlP->Params[0] = 0;  GlP->Params[1] = ConeW;
  GlP->Params[2] = CQ; GlP->Params[3] = CQ;
  GlP->SetQuadricOrientation(GLU_INSIDE);

  GlP = &GPC.NewPrimitive("CylinderZ", sgloCylinder);  // Z axis
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  EB = new TEBasis(*EB);  GlP->SetBasis(EB);  EB->NullCenter();
  GlP->Params[0] = ConeW/2;  GlP->Params[1] = ConeW/2;
  GlP->Params[2] = m[2].Length()/5; GlP->Params[3] = CQ; GlP->Params[4] = CQ;
  
  if( (PMask & 0x2) != 0 )  {
    GlM.SetIdentityDraw(true);
    GlM.SetTransparent(false);
    GlP = &GPC.NewPrimitive("Label", sgloText);  // labels
    GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
    GlP->SetFont(Parent.GetScene().DefFont());
  }
  Compile();
}
//..............................................................................
bool TDBasis::Orient(TGlPrimitive& P) {
  // extra zoom is very important for making pictures - it makes sure that the
  // object is translated to the right place!
  const double EZoom = Parent.GetExtraZoom()*Parent.GetViewZoom();
  //const double zoom = olx_sqr(Parent.GetBasis().GetZoom());
  if( P.GetType() == sgloText )  {
    olxstr Str('a');
    const double scale = 1./Parent.GetScale();
    vec3d Center(Basis.GetCenter());
    Center[0] = Center[0]*EZoom;
    Center[1] = Center[1]*EZoom;
    Center *= Parent.GetScale();
    Center = Parent.GetBasis().GetMatrix() * Center;
    Center /= Parent.GetBasis().GetZoom();
    const TGlFont& fnt = *Parent.GetScene().DefFont();
    for( int i=0; i < 3; i++ )  {
      vec3d T = AU->GetCellToCartesian()[i];
      T /= 5;
      T += vec3d(T).NormaliseTo(0.8);
      T *= Basis.GetZoom();
      T += Center;
      T *= Parent.GetBasis().GetMatrix();
      T *= Parent.GetBasis().GetZoom();
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
  T /= Parent.GetBasis().GetZoom();
  T -= Parent.GetBasis().GetCenter();
  olx_gl::translate(T);
  olx_gl::scale(Basis.GetZoom());
  return false;
}
//..............................................................................
void TDBasis::ToDataItem(TDataItem& di) const {
  Basis.ToDataItem(di.AddItem("basis"));
}
//..............................................................................
void TDBasis::FromDataItem(const TDataItem& di)  {
  Basis.FromDataItem(di.FindRequiredItem("basis"));
}
//..............................................................................
void TDBasis::ListPrimitives(TStrList &List) const {
  List.Add("Axis");
  List.Add("Labels");
}
//..............................................................................
void TDBasis::UpdatePrimitives(int32_t Mask, const ACreationParams* cpar) {
  SetBit(true, Mask, 1);
  AGDrawObject::UpdatePrimitives(Mask, cpar);
}
//..............................................................................
 
