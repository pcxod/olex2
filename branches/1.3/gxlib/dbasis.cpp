/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "dbasis.h"
#include "gpcollection.h"
#include "glrender.h"
#include "styles.h"
#include "glmaterial.h"
#include "glprimitive.h"
#include "pers_util.h"

TDBasis::TDBasis(TGlRenderer& Render, const olxstr& collectionName) :
  AGlMouseHandlerImp(Render, collectionName),
  Zoom(1.0)
{
  SetMove2D(true);
  SetMoveable(true);
  SetZoomable(true);
  SetSelectable(false);
  olxstr label_cn("db_label");
  for (int i = 0; i < 3; i++) {
    (Labels[i] = new TXGlLabel(Parent, label_cn))->SetVisible(false);
    Labels[i]->SetTransformer(this);
  }
}
//..............................................................................
TDBasis::~TDBasis() {
  for (int i = 0; i < 3; i++) {
    delete Labels[i];
  }
}
//..............................................................................
void TDBasis::SetAsymmUnit(TAsymmUnit& au)  {
  AU = &au;
  const size_t FontIndex = Parent.GetScene().FindFontIndexForType<TDBasis>();
  for( int i=0; i < 3; i++ )  {
    //Labels[i]->SetCenter(FGlP->Vertices[i*2+1]);
    Labels[i]->SetFontIndex(FontIndex);
    Labels[i]->SetLabel(olxstr((char)('a'+i)));
  }
}
//..............................................................................
void TDBasis::Create(const olxstr& cName) {
  if (!cName.IsEmpty()) {
    SetCollectionName(cName);
  }
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if (GPC.PrimitiveCount() != 0) {
    GPC.AddObject(*this);
    for (int i = 0; i < 3; i++) {
      Labels[i]->Create();
    }
    return;
  }
  TGraphicsStyle& GS = GPC.GetStyle();
  mat3d m = AU->GetCellToCartesian();
  if (m[0].QLength() < 1.e-6) {
    m.I();
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
  const int CQ = 5, CQs = 1; // cylinder quality

  GlM.SetFlags(sglmAmbientF | sglmDiffuseF | sglmSpecularF |
    sglmAmbientB | sglmDiffuseB | sglmSpecularB);
  GlM1.SetFlags(sglmAmbientF | sglmDiffuseF | sglmSpecularF |  // non transluetn sphere
    sglmAmbientB | sglmDiffuseB | sglmSpecularB);

  TGlPrimitive* GlP = &GPC.NewPrimitive("Sphere", sgloSphere);  // a sphere at the basis of the object {0,0,0}
  GlM.AmbientF = 0x800f0f0f;
  GlP->SetProperties(GlM1);
  GlP->Params[0] = ConeW / 1.5;  GlP->Params[1] = 6;  GlP->Params[2] = 6;

  GlM.AmbientF = 0x800000ff;
  GlP = &GPC.NewPrimitive("ConeX", sgloCylinder);  // X cone
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  TEBasis* EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(TEBasis::CalcBasis<vec3d, mat3d>(m[0]));  EB->SetCenter(m[0] / 5);
  GlP->Params[0] = ConeW;  GlP->Params[1] = 0;
  GlP->Params[2] = ConeH; GlP->Params[3] = CQ; GlP->Params[4] = CQ;

  GlP = &GPC.NewPrimitive("DiskX", sgloDisk);  // X cone bottom
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  GlP->SetBasis(new TEBasis(*EB));
  GlP->Params[0] = 0;  GlP->Params[1] = ConeW;
  GlP->Params[2] = CQ; GlP->Params[3] = CQ;
  GlP->SetQuadricOrientation(GLU_INSIDE);

  GlP = &GPC.NewPrimitive("CylinderX", sgloCylinder);  // X axis
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  EB = new TEBasis(*EB);  GlP->SetBasis(EB);  EB->NullCenter();
  GlP->Params[0] = ConeW / 2;  GlP->Params[1] = ConeW / 2;
  GlP->Params[2] = m[0].Length() / 5; GlP->Params[3] = CQ; GlP->Params[4] = CQs;

  GlM.AmbientF = 0x8000ff00;
  GlP = &GPC.NewPrimitive("ConeY", sgloCylinder);  // Y
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(TEBasis::CalcBasis<vec3d, mat3d>(m[1]));  EB->SetCenter(m[1] / 5);
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
  GlP->Params[0] = ConeW / 2;  GlP->Params[1] = ConeW / 2;
  GlP->Params[2] = m[1].Length() / 5; GlP->Params[3] = CQ; GlP->Params[4] = CQs;

  GlM.AmbientF = 0x80ff0000;
  GlP = &GPC.NewPrimitive("ConeZ", sgloCylinder);  //Z cone
  GlP->SetProperties(GS.GetMaterial(GlP->GetName(), GlM));
  EB = new TEBasis;  GlP->SetBasis(EB);  EB->SetMatrix(TEBasis::CalcBasis<vec3d, mat3d>(m[2]));  EB->SetCenter(m[2] / 5);
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
  GlP->Params[0] = ConeW / 2;  GlP->Params[1] = ConeW / 2;
  GlP->Params[2] = m[2].Length() / 5; GlP->Params[3] = CQ; GlP->Params[4] = CQs;

  Compile();
  for (int i = 0; i < 3; i++) {
    Labels[i]->Create();
  }
}
//..............................................................................
const vec3d &TDBasis::GetAxis(const TXGlLabel &l) const {
  for (int i = 0; i < 3; i++) {
    if (&l == Labels[i]) {
      return AU->GetCellToCartesian()[i];
    }
  }
  throw TInvalidArgumentException(__OlxSourceInfo, "label");
}
//..............................................................................
vec3d TDBasis::ForRaster(const TXGlLabel& l) const {
  vec3d t = ForVector(l);
  t /= Parent.GetScale();
  t[2] = Parent.CalcRasterZ(0.001);
  return t;
}
//..............................................................................
vec3d TDBasis::ForVector(const TXGlLabel& l) const {
  const double Scale = Parent.GetScale()*Parent.GetExtraZoom()
    *Parent.GetViewZoom();

  vec3d lo = GetAxis(l);
  lo /= 5;
  lo += vec3d(lo).NormaliseTo(0.8);
  lo = lo*(GetZoom() / Scale);

  vec3d off = Parent.GetBasis().GetMatrix()*l.GetCenter() + lo;
  vec3d T = Parent.Project(GetCenter() + l.GetOffset() + off*Scale);
  T[2] = Parent.CalcRasterZ(0.001);
  return T;
}
//..............................................................................
bool TDBasis::DoTranslate(const vec3d& t) {
  _Center += Parent.GetBasis().GetMatrix()*
    (t*(Parent.GetScale()/Parent.GetBasis().GetZoom()));
  return true;
}
//..............................................................................
bool TDBasis::Orient(TGlPrimitive& P) {
  olx_gl::translate(GetCenter());
  olx_gl::scale(GetZoom());
  return false;
}
//..............................................................................
void TDBasis::ToDataItem(TDataItem& di) const {
  di.AddField("center", PersUtil::VecToStr(GetCenter()));
  di.AddField("zoom", GetZoom());
  TDataItem& labels = di.AddItem("Labels");
  for (int i = 0; i < 3; i++) {
    Labels[i]->ToDataItem(labels.AddItem(olxstr((olxch)('x' + i))));
  }
}
//..............................................................................
void TDBasis::FromDataItem(const TDataItem& di) {
  const olxstr& c = di.FindField("center");
  if (c.IsEmpty()) {
    TEBasis b;
    b.FromDataItem(di.GetItemByName("basis"));
    _Center = b.GetCenter();
    Zoom = b.GetZoom();
  }
  else {
    PersUtil::VecFromStr(c, _Center);
    Zoom = di.GetFieldByName("zoom").ToDouble();
  }
  const TDataItem* labels = di.FindItem("Labels");
  if (labels != NULL && labels->ItemCount() == 3) {
    for (int i = 0; i < 3; i++) {
      Labels[i]->FromDataItem(labels->GetItemByIndex(i));
    }
  }
}
//..............................................................................
void TDBasis::ListPrimitives(TStrList &List) const {}
//..............................................................................
void TDBasis::UpdatePrimitives(int32_t Mask) {}
//..............................................................................
void TDBasis::UpdateLabel() {
  for (int i = 0; i < 3; i++) {
    Labels[i]->Update();
  }
}
//..............................................................................
void TDBasis::SetVisible(bool v) {
  if (v && _Center.IsNull()) {
    _Center = -Parent.GetBasis().GetCenter();
  }
  AGDrawObject::SetVisible(v);
  for (int i = 0; i < 3; i++) {
    Labels[i]->SetVisible(v);
  }
}
//..............................................................................
