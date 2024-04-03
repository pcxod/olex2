/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xangle.h"
#include "styles.h"
#include "glprimitive.h"
#include "pers_util.h"

//..............................................................................
TXAngle::TXAngle(TGlRenderer& R, const olxstr& collectionName,
  const vec3d& cnt, const vec3d& from, const vec3d& to)
  : AGDrawObject(R, collectionName),
  material("85;0;4286611584;4290822336;64"),
  center(cnt), from(from), to(to),
  settings(0)
{
  Label = new TXGlLabel(GetParent(), PLabelsCollectionName());
  Label->SetOffset((from + to)/ 2);
}
//...........................................................................
TXAngle::TXAngle(TGlRenderer& R, const TDataItem& di)
  : AGDrawObject(R, EmptyString()),
  settings(0)
{
  Label = new TXGlLabel(GetParent(), PLabelsCollectionName());
  FromDataItem(di);
}
//...........................................................................
TXAngle::~TXAngle() {
  delete Label;
}
//...........................................................................
void TXAngle::Create(const olxstr& cName) {
  if (!cName.IsEmpty()) {
    SetCollectionName(cName);
  }
  Label->SetFontIndex(Parent.GetScene().FindFontIndexForType<TXAngle>());
  Label->Create();
  Label->SetVisible(true);
  olxstr NewL;
  TGPCollection* GPC = Parent.FindCollectionX(GetCollectionName(), NewL);
  if (GPC == 0) {
    GPC = &Parent.NewCollection(NewL);
  }
  GPC->AddObject(*this);
  if (GPC->PrimitiveCount() != 0) {
    return;
  }
  uint32_t pmask = GetPrimitiveMask();
  if (pmask == 0) {
    return;
  }
  TStrList pnames;
  ListPrimitives(pnames);

  TGraphicsStyle& GS = GPC->GetStyle();
  //GS.SetSaveable(false);
  for (size_t i = 0; i < pnames.Count(); i++) {
    if ((pmask & (1 << i)) != 0) {
      TGlPrimitive& GlP = GPC->NewPrimitive(pnames[i], sgloCommandList);
      GlP.SetProperties(GS.GetMaterial(pnames[i], material));
      GlP.SetOwnerId(i);
    }
  }
}
//...........................................................................
bool TXAngle::Orient(TGlPrimitive& glp) {
  vec3d::Circumcenter(from, center, to);
  
  vec3d intersect = center.NormalIntersection(from, to);
  vec3d shift = intersect - (from + to) / 2;
  vec3d ba = from - to;
  vec3d new_cnt = center -
    ba.NormaliseTo((shift).Length()) * olx_sign(ba.DotProd(shift));
  vec3d a = from - new_cnt;
  vec3d b = to - new_cnt;
  olx_gl::translate(new_cnt);
  vec3d normal = a.XProdVec(b).Normalise();

  int sections = GetSettings().GetSections();
  double ang = acos(a.CAngle(b));
  double ca = cos(ang/sections/2);
  mat3d rm;
  if (glp.GetOwnerId() == 0) {
    olx_gl::begin(GL_LINES);
    olx_create_rotation_matrix(rm, normal, ca);
    for (int i = 0; i < sections; i++) {
      olx_gl::vertex(a);
      a *= rm;
      olx_gl::vertex(a);
      a *= rm;
    }
    olx_gl::end();
  }
  if (glp.GetOwnerId() == 1) {
    const TStringToList<olxstr, TGlPrimitive*>& primtives =
      GetSettings().GetPrimitives(true);
    ca = cos(ang / sections);
    olx_create_rotation_matrix(rm, normal, ca);
    a = from - new_cnt;
    for (int i = 0; i < sections; i++) {
      olx_gl::translate(a);
      primtives.GetObject(0)->Draw();
      olx_gl::translate(-a);
      a *= rm;
    }
  }
  return true;
}
//...........................................................................
void TXAngle::Update() {
}
//...........................................................................
void TXAngle::SetVisible(bool v) {
  AGDrawObject::SetVisible(v);
  Label->SetVisible(v);
}
//...........................................................................
void TXAngle::ToDataItem(TDataItem& i) const {
  i.SetValue(GetCollectionName());
  i.AddField("from", PersUtil::VecToStr(from))
    .AddField("to", PersUtil::VecToStr(to))
    .AddField("center", PersUtil::VecToStr(center))
    ;
  Label->ToDataItem(i.AddItem("label"));
}
//...........................................................................
void TXAngle::FromDataItem(const TDataItem& i) {
  SetCollectionName(i.GetValue());
  PersUtil::VecFromStr(i.GetFieldByName("from"), from);
  PersUtil::VecFromStr(i.GetFieldByName("to"), to);
  PersUtil::VecFromStr(i.GetFieldByName("center"), center);
  Label->FromDataItem(i.GetItemByName("label"));
}
//...........................................................................
void TXAngle::ListDrawingStyles(TStrList& List) {
}
//...........................................................................
void TXAngle::ListParams(TStrList& List, TGlPrimitive* Primitive) {
}
//..............................................................................
void TXAngle::ListParams(TStrList& List) {
}
//..............................................................................
void TXAngle::UpdatePrimitiveParams(TGlPrimitive* Primitive) {
}
//..............................................................................
void TXAngle::ListPrimitives(TStrList& List) const {
  List.Add("Lines");
  List.Add("Balls");
}
uint32_t TXAngle::GetPrimitiveMask() const {
  return GetPrimitives().GetStyle().GetNumParam(GetPrimitiveMaskName(),
    GetSettings().GetMask(), IsMaskSaveable());
}
//...........................................................................
//...........................................................................
//...........................................................................
void TXAngle::Settings::CreatePrimitives() {
  ClearPrimitives();
  TGlPrimitive& sph = parent.NewPrimitive(sgloSphere);
  sph.Params[0] = 0.04;
  sph.Params[1] = 5;
  sph.Params[2] = 5;
  sph.Compile();
  primitives.Add("Sphere", &sph);
}
//...........................................................................
