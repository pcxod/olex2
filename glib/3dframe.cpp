/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "3dframe.h"
#include "pers_util.h"
#include "library.h"

void TFaceCtrl::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )  SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;
  TGraphicsStyle& GS = GPC.GetStyle();
  TGlPrimitive& dummy = GPC.NewPrimitive("Button", sgloMacro);
  TGlMaterial& glm = GS.GetMaterial("button",
    TGlMaterial("85;1.000,0.000,0.000,0.500;1.000,1.000,0.000,0.100;1.000,"
      "1.000,1.000,1.000;1"));
  dummy.SetProperties(glm);
}
//.............................................................................
bool TFaceCtrl::Orient(TGlPrimitive&) {
  ParentCtrl.SetBasis();
  const vec3d cnt = (A+B+C+D)/4;
  olx_gl::translate(cnt + N*0.04);
  const vec3d v1 = (C-A)*0.1;
  const vec3d v2 = (D-B)*0.1;
  olx_gl::normal(N);
  olx_gl::begin(GL_QUADS);
  olx_gl::vertex(-v1);
  olx_gl::vertex(-v2);
  olx_gl::vertex(v1);
  olx_gl::vertex(v2);
  olx_gl::end();
  return true;
}
///////////////////////////////////////////////////////////////////////////////
T3DFrameCtrl::T3DFrameCtrl(TGlRenderer& prnt, const olxstr& cName)
  : AGlMouseHandlerImp(prnt, cName)
{
  zoom = 1;
  sphere = false;
  SetMoveable(true);
  SetRoteable(true);
  SetZoomable(true);

  edges[0] = vec3d(-0.5,-0.5,-0.5);
  edges[1] = vec3d(-0.5,0.5,-0.5);
  edges[2] = vec3d(0.5,0.5,-0.5);
  edges[3] = vec3d(0.5,-0.5,-0.5);
  edges[4] = vec3d(-0.5,-0.5,0.5);
  edges[5] = vec3d(-0.5,0.5,0.5);
  edges[6] = vec3d(0.5,0.5,0.5);
  edges[7] = vec3d(0.5,-0.5,0.5);

  Faces.Add(new TFaceCtrl(prnt, "facectrl", 0, edges[0], edges[1], edges[2],
    edges[3], norms[0], *this));
  Faces.Add(new TFaceCtrl(prnt, "facectrl", 1, edges[0], edges[4], edges[5],
    edges[1], norms[1], *this));
  Faces.Add(new TFaceCtrl(prnt, "facectrl", 2, edges[0], edges[3], edges[7],
    edges[4], norms[2], *this));
  Faces.Add(new TFaceCtrl(prnt, "facectrl", 3, edges[1], edges[5], edges[6],
    edges[2], norms[3], *this));
  Faces.Add(new TFaceCtrl(prnt, "facectrl", 4, edges[2], edges[6], edges[7],
    edges[3], norms[4], *this));
  Faces.Add(new TFaceCtrl(prnt, "facectrl", 5, edges[4], edges[7], edges[6],
    edges[5], norms[5], *this));
  UpdateEdges();
}
//.............................................................................
void T3DFrameCtrl::Create(const olxstr& cName) {
  if (!cName.IsEmpty())  SetCollectionName(cName);
  for (size_t i = 0; i < Faces.Count(); i++) {
    Faces[i].Create(EmptyString());
  }
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if (GPC.PrimitiveCount() != 0)  return;
  TGraphicsStyle& GS = GPC.GetStyle();
  TGlPrimitive& sph = GPC.NewPrimitive("Sphere", sgloSphere);
  sph.Params[0] = 1;
  sph.Params[1] = 25;
  sph.Params[2] = 25;
  TGlMaterial& glm = GS.GetMaterial("Sphere",
    TGlMaterial("1109;0.000,0.502,0.753,1.000;2768240640;0.180,0.180,0.180,"
      "1.000;5"));
  sph.SetProperties(glm);
  olxstr sp = GS.GetParam("spherical", FalseString(), true);
  if (sp.IsBool()) {
    bool s = sp.ToBool();
    if (s != sphere) {
      SetType(s ? ftSphere : ftBox);
    }
  }
}
//.............................................................................
bool T3DFrameCtrl::Orient(TGlPrimitive& p)  {
  SetBasis();
  if (sphere) {
    if (p.GetType() == sgloSphere) {
      olx_gl::translate(GetCenter());
      olx_gl::scale(zoom);
      return false;
    }
    return true;
  }
  else {
    olx_gl::begin(GL_QUADS);
    for (int i = 0; i < 6; i++)  {
      olx_gl::normal(Faces[i].GetN());
      olx_gl::vertex(Faces[i].GetA());
      olx_gl::vertex(Faces[i].GetB());
      olx_gl::vertex(Faces[i].GetC());
      olx_gl::vertex(Faces[i].GetD());
    }
    olx_gl::end();
  }
  return true;
}
//.............................................................................
void T3DFrameCtrl::SetVisible(bool v) {
  AGDrawObject::SetVisible(v);
  if (!v) {
    for (int i = 0; i < 6; i++) {
      Faces[i].SetVisible(v);
    }
  }
  else {
    for (int i = 0; i < 6; i++) {
      Faces[i].SetVisible(!IsSpherical());
    }
  }
}
//.............................................................................
bool T3DFrameCtrl::DoRotate(const vec3d& vec, double angle)  {
  mat3d m;
  olx_create_rotation_matrix(m, vec, cos(angle), sin(angle));
  vec3d cnt = GetCenter();
  for (int i = 0; i < 8; i++) {
    edges[i] = (edges[i] - cnt) * m + cnt;
  }
  UpdateEdges();
  return !sphere;
}
//.............................................................................
bool T3DFrameCtrl::DoZoom(double zoom_, bool inc)  {
  const double vol = GetVolume();
  double z;
  if (inc) {
    z = 1.0 + zoom_;
  }
  else {
    z = zoom_;
  }
  if (vol * z < 0.1 && z < 1.0) {
    return true;
  }
  vec3d cnt = GetCenter();
  for (int i = 0; i < 8; i++) {
    edges[i] = (edges[i] - cnt) * z + cnt;
  }
  zoom *= z;
  return true;
}
//.............................................................................
bool T3DFrameCtrl::OnTranslate(size_t sender, const vec3d& t)  {
  vec3d dir = (Faces[sender].GetCenter()-GetCenter()).NormaliseTo(
    t.Length());
  dir *= olx_sign(dir.DotProd(t));
  Faces[sender].GetA() += dir;
  Faces[sender].GetB() += dir;
  Faces[sender].GetC() += dir;
  Faces[sender].GetD() += dir;
  center = olx_mean(olx_as_list(edges, 8));
  return true;
}
//.............................................................................
void T3DFrameCtrl::UpdateEdges()  {
  for( int i=0; i < 6; i++ ) {
    norms[i] = (Faces[i].GetC()-Faces[i].GetB()).XProdVec(
      Faces[i].GetA()-Faces[i].GetB()).Normalise();
  }
  center = olx_mean(olx_as_list(edges, 8));
  vec3d sz = GetSize();
  if (sz[0] > sz[1]) {
    zoom = (sz[0] > sz[2] ? sz[0] : sz[2]);
  }
  else {
    zoom = (sz[1] > sz[2] ? sz[1] : sz[2]);
  }
  zoom /= 2;
}
//.............................................................................
void T3DFrameCtrl::ToDataItem(TDataItem &di) const {
  di.AddField("edges", PersUtil::VecArrayToStr(edges, 8));
  di.AddField("visible", IsVisible());
  di.AddField("sphere", IsSpherical());
}
//.............................................................................
void T3DFrameCtrl::FromDataItem(const TDataItem &di) {
  SetType(di.FindField("sphere", FalseString()).ToBool() ? ftSphere : ftBox);
  SetVisible(di.GetFieldByName("visible").ToBool());
  PersUtil::VecArrayFromStr(di.GetFieldByName("edges"), edges, 8);
  UpdateEdges();
}
//.............................................................................
void T3DFrameCtrl::SetType(int t) {
  if (t == ftSphere) {
    sphere = true;
  }
  else {
    sphere = false;
  }
  for (size_t i = 0; i < Faces.Count(); i++) {
    Faces[i].SetVisible(!sphere);
  }
}
//.............................................................................
void T3DFrameCtrl::LibType(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    E.SetRetVal<olxstr>(sphere ? "sphere" : "box");
  }
  else {
    if (Params[0].Equalsi("sphere")) {
      SetType(ftSphere);
    }
    else if (Params[0].Equalsi("box")) {
      SetType(ftBox);
    }
    else {
      E.ProcessingError(__OlxSrcInfo, "Sphere/Box is expected");
    }
    if (GetPrimitives().HasStyle()) {
      TGraphicsStyle &s = GetPrimitives().GetStyle();
      s.SetParam("spherical", sphere, true);
    }
  }
}
//.............................................................................
TLibrary* T3DFrameCtrl::ExportLibrary(const olxstr& name) {
  TLibrary* lib = new TLibrary((name.IsEmpty() ? olxstr("wbox") : name));
  lib->Register(
    new TFunction<T3DFrameCtrl>(this, &T3DFrameCtrl::LibType,
    "Type", fpNone|fpOne,
    "Sets the frame type - sphere/box"));
  AGDrawObject::ExportLibrary(*lib);
  return lib;
}
