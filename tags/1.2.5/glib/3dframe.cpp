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
bool TFaceCtrl::Orient(TGlPrimitive&)  {
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
void T3DFrameCtrl::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )  SetCollectionName(cName);
  for( size_t i=0; i < Faces.Count(); i++ )
    Faces[i].Create(EmptyString());
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;
  TGraphicsStyle& GS = GPC.GetStyle();
  TGlPrimitive& dummy = GPC.NewPrimitive("Box", sgloMacro);
  TGlMaterial& glm = GS.GetMaterial("Box",
    TGlMaterial("1109;0.000,0.502,0.753,1.000;2768240640;0.180,0.180,0.180,"
      "1.000;5"));
  dummy.SetProperties(glm);
}
//.............................................................................
bool T3DFrameCtrl::DoRotate(const vec3d& vec, double angle)  {
  mat3d m;  
  olx_create_rotation_matrix(m, vec, cos(angle), sin(angle));
  vec3d cnt = GetCenter();
  for( int i=0; i < 8; i++ )
    edges[i] = (edges[i]-cnt)*m+cnt;
  UpdateEdges();
  return true;
}
//.............................................................................
bool T3DFrameCtrl::DoZoom(double zoom, bool inc)  {
  const double vol = GetVolume();
  double z;
  if( inc )
    z = 1.0 + zoom;
  else
    z = zoom;
  if( vol*z < 0.1 && z < 1.0 )  return true;
  vec3d cnt = GetCenter();
  for( int i=0; i < 8; i++ )
    edges[i] = (edges[i]-cnt)*z+cnt;
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
  return true;
}
//.............................................................................
void T3DFrameCtrl::UpdateEdges()  {
  for( int i=0; i < 6; i++ ) {
    norms[i] = (Faces[i].GetC()-Faces[i].GetB()).XProdVec(
      Faces[i].GetA()-Faces[i].GetB()).Normalise();
  }
}
//.............................................................................
void T3DFrameCtrl::ToDataItem(TDataItem &di) const {
  di.AddField("edges", PersUtil::VecArrayToStr(edges, 8));
  di.AddField("visible", IsVisible());
}
//.............................................................................
void T3DFrameCtrl::FromDataItem(const TDataItem &di) {
  SetVisible(di.GetFieldByName("visible").ToBool());
  PersUtil::VecArrayFromStr(di.GetFieldByName("edges"), edges, 8);
  UpdateEdges();
}
//.............................................................................
