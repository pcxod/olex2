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
#include "gxapp.h"

//..............................................................................
TXAngle::TXAngle(TGlRenderer& R, const olxstr& collectionName,
  const vec3d& cnt, const vec3d& from, const vec3d& to)
  : AGlMouseHandlerImp(R, collectionName),
  center(cnt), from(from), to(to),
  thickness(1), radius(1),
  settings(0)
{
  Label = new TXGlLabel(GetParent(), PLabelsCollectionName());
  Label->SetOffset((from + to)/ 2);
  Init();
}
//...........................................................................
TXAngle::TXAngle(TGlRenderer& R, const TDataItem& di)
  : AGlMouseHandlerImp(R, EmptyString()),
  thickness(1), radius(1),
  settings(0)
{
  Label = new TXGlLabel(GetParent(), PLabelsCollectionName());
  FromDataItem(di);
  Init();
}
//...........................................................................
TXAngle::~TXAngle() {
  delete Label;
}
//...........................................................................
void TXAngle::Init() {
  SetMoveable(true);
  SetZoomable(true);

  vec3d intersect = center.NormalIntersection(from, to);
  vec3d ab_center = (from + to) / 2;
  vec3d shift = intersect - ab_center;
  vec3d ba = from - to;
  draw_center = center -
    ba.NormaliseTo((shift).Length()) * olx_sign(ba.DotProd(shift));
  if (radius != 1) {
    draw_center = ab_center + (draw_center - ab_center) * radius;
  }
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
  TGlMaterial def_m("255;4294967040;4294967040;4286611584;4286611584;4290822336;4290822336;16;16");
  TGraphicsStyle& GS = GPC->GetStyle();
  for (size_t i = 0; i < pnames.Count(); i++) {
    if ((pmask & (1 << i)) != 0) {
      TGlPrimitive& GlP = GPC->NewPrimitive(pnames[i], sgloCommandList);
      GlP.SetProperties(GS.GetMaterial(pnames[i], def_m));
      GlP.SetOwnerId(i);
    }
  }
}
//...........................................................................
bool TXAngle::Orient(TGlPrimitive& glp) {
  vec3d a = (from - draw_center) * radius;
  vec3d b = (to - draw_center) * radius;
  olx_gl::translate(draw_center);
  vec3d normal = a.XProdVec(b).Normalise();

  int sections = GetSettings().GetSections();
  double ang = acos(a.CAngle(b));
  mat3d rm;
  if (glp.GetOwnerId() == 0) {
    if (thickness != 1) {
      olx_gl::lineWidth(thickness);
    }
    double ca = cos(ang / ((double)sections-0.5) / 2);
    olx_gl::begin(GL_LINES);
    olx_create_rotation_matrix(rm, normal, ca);
    for (int i = 0; i < sections; i++) {
      olx_gl::vertex(a);
      a *= rm;
      olx_gl::vertex(a);
      a *= rm;
    }
    olx_gl::end();
    if (thickness != 1) {
      olx_gl::lineWidth(1./thickness);
    }
  }
  else if (glp.GetOwnerId() == 1) {
    const TStringToList<olxstr, TGlPrimitive*>& primtives =
      GetSettings().GetPrimitives(true);
    double ca = cos(ang / sections);
    olx_create_rotation_matrix(rm, normal, ca);
    for (int i = 0; i < sections; i++) {
      vec3d b = a * rm;
      vec3d t = a + (b-a) / 2;
      olx_gl::translate(t);
      if (thickness != 1) {
        olx_gl::scale(thickness);
      }
      primtives.GetObject(0)->Draw();
      if (thickness != 1) {
        olx_gl::scale(1./thickness);
      }
      olx_gl::translate(-t);
      a = b;
    }
  }
  else if (glp.GetOwnerId() == 2) {
    const TStringToList<olxstr, TGlPrimitive*>& primtives =
      GetSettings().GetPrimitives(true);
    double ca = cos(ang / sections);
    olx_create_rotation_matrix(rm, normal, ca);
    for (int i = 0; i < sections; i++) {
      vec3d b = a * rm;
      vec3d t = a + (b - a) / 2;
      vec3d d = (b - a).Normalise();
      double rang = acos(d[2]) * 180 / M_PI;
      olx_gl::translate(t);
      if (thickness != 1) {
        olx_gl::scale(thickness);
      }
      olx_gl::rotate(rang, -d[1], d[0], 0.0);
      primtives.GetObject(glp.GetOwnerId() - 1)->Draw();
      olx_gl::rotate(-rang, -d[1], d[0], 0.0);
      if (thickness != 1) {
        olx_gl::scale(1. / thickness);
      }
      olx_gl::translate(-t);
      a = b;
    }
  }
  if (glp.GetOwnerId() == 3) {
    double ca = cos(ang / sections / 2);
    olx_gl::begin(GL_TRIANGLE_FAN);
    olx_gl::normal(normal);
    olx_gl::vertex(vec3d());
    olx_create_rotation_matrix(rm, normal, ca);
    for (int i = 0; i < sections; i++) {
      olx_gl::vertex(a);
      a *= rm;
      olx_gl::vertex(a);
      a *= rm;
    }
    olx_gl::vertex(a);
    olx_gl::end();
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
    .AddField("thickness", thickness)
    .AddField("radius", radius)
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
  thickness = i.FindField("thickness", "1").ToDouble();
  radius = i.FindField("radius", "1").ToDouble();
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
  List.Add("Cylinders");
  List.Add("Segment");
}
uint32_t TXAngle::GetPrimitiveMask() const {
  return GetPrimitives().GetStyle().GetNumParam(GetPrimitiveMaskName(),
    GetSettings().GetMask(), IsMaskSaveable());
}
//...........................................................................
bool TXAngle::DoTranslate(const vec3d& t_) {
  vec3d t = TGXApp::GetConstrainedDirection(t_);
  radius += t.Length() * olx_sign(t[1]);
  if (radius < 0.3) {
    radius = 0.3;
  }
  else if (radius > 1) {
    radius = 1;
  }
  return true;
}
//...........................................................................
bool TXAngle::DoZoom(double zoom, bool inc) {
  thickness += zoom;
  if (thickness < 1) {
    thickness = 1;
  }
  else if (thickness > 5) {
    thickness = 5;
  }
  return true;
}
//...........................................................................
//...........................................................................
void TXAngle::Settings::CreatePrimitives() {
  ClearPrimitives();
  {
    TGlPrimitive& sph = parent.NewPrimitive(sgloSphere);
    sph.Params[0] = 0.04;
    sph.Params[1] = 8;
    sph.Params[2] = 8;
    sph.Compile();
    primitives.Add("Sphere", &sph);
  }
  {
    TGlPrimitive& glp = parent.NewPrimitive(sgloCommandList);

    TGlPrimitive& tmp_c = parent.NewPrimitive(sgloCylinder);
    tmp_c.Params[0] = 0.02;  tmp_c.Params[1] = 0.02;  tmp_c.Params[2] = 0.1;
    tmp_c.Params[3] = 5;    tmp_c.Params[4] = 1;
    tmp_c.Compile();

    TGlPrimitive& tmp_d = parent.NewPrimitive(sgloDisk);
    tmp_d.Params[0] = 0;  tmp_d.Params[1] = 0.02;  tmp_d.Params[2] = 5;
    tmp_d.Params[3] = 1;
    tmp_d.Compile();

    glp.StartList();
    olx_gl::translate(0.0, 0.0, -0.05);
    glp.CallList(&tmp_c);
    glp.CallList(&tmp_d);
    olx_gl::translate(0.0, 0.0, 0.05);
    glp.CallList(&tmp_d);
    glp.EndList();
    primitives.Add("Cylinder", &glp);
  }
}
//...........................................................................
