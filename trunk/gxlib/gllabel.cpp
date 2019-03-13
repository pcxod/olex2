/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "gllabel.h"
#include "gpcollection.h"
#include "styles.h"
#include "glrender.h"
#include "glprimitive.h"
#include "pers_util.h"
#include "povdraw.h"
#include "wrldraw.h"
#include "gxapp.h"

TXGlLabel::TXGlLabel(TGlRenderer& R, const olxstr& collectionName) :
  AGlMouseHandlerImp(R, collectionName), Transformer(0)
{
  SetMove2DZ(true);
  SetMoveable(true);
  SetZoomable(false);
  SetGroupable(true);
  FontIndex = ~0;
};
//..............................................................................
void TXGlLabel::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  text_rect = GetFont().GetTextRect(FLabel);
  if( GPC.PrimitiveCount() != 0 )  {
    TGlPrimitive* glpText = GPC.FindPrimitiveByName("Text");
    if (glpText != 0) {
      glpText->SetFont(&GetFont());
    }
    return;
  }

  TGraphicsStyle& GS = GPC.GetStyle();
  GS.SetPersistent(true);
  TGlPrimitive& glpPlane = GPC.NewPrimitive("Plane", sgloQuads);
  glpPlane.SetProperties(GS.GetMaterial("Plane", TGlMaterial("3077;2131693327;427259767")));
  glpPlane.Vertices.SetCount(4);

  TGlPrimitive& glpText = GPC.NewPrimitive("Text", sgloText);
  glpText.SetProperties(GS.GetMaterial("Text", TGlMaterial("2049;0.000,0.000,0.000,1.000")));
  glpText.SetFont(&GetFont());
  glpText.Params[0] = -1;  //bitmap; TTF by default
}
//..............................................................................
void TXGlLabel::SetLabel(const olxstr& L)  {
  FLabel = L;
  text_rect = GetFont().GetTextRect(FLabel);
  double inc = text_rect.height *0.5;
  text_rect.top -= inc / 2;
  text_rect.height += inc;
}
//..............................................................................
vec3d TXGlLabel::GetRasterPosition() const {
  if (Transformer != 0) {
    return Transformer->ForRaster(*this);
  }
  vec3d t = GetVectorPosition();
  t /= Parent.GetScale();
  t[2] = Parent.CalcRasterZ(0.001);
  return t;
}
//..............................................................................
vec3d TXGlLabel::GetVectorPosition() const {
  if (Transformer != 0) {
    return Transformer->ForVector(*this);
  }
  vec3d off = Parent.GetBasis().GetMatrix()*GetCenter();
  const double Scale = Parent.GetScale()*Parent.GetExtraZoom()
    *Parent.GetViewZoom();
  vec3d T = Parent.Project(GetOffset() + off*Scale);
  T[2] = Parent.CalcRasterZ(0.001);
  return T;
}
//..............................................................................
bool TXGlLabel::Orient(TGlPrimitive& P) {
  const double Scale = Parent.GetScale();
  TGlFont& glf = GetFont();
  if (P.GetType() == sgloText) {
    if (!glf.IsVectorFont()) {
      vec3d T = GetRasterPosition();
      Parent.DrawTextSafe(T, FLabel, glf);
    }
    else {
      vec3d T = GetVectorPosition();
      //float glw;
      //glGetFloatv(GL_LINE_WIDTH, &glw);
      //glLineWidth((float)(1./Scale)/50);
      glf.DrawVectorText(T, FLabel, Parent.GetBasis().GetZoom() / Parent.CalcZoom());
      //glLineWidth(glw);
    }
    return true;
  }
  else {
    vec3d T = GetVectorPosition();
    T[2] -= 0.0005;
    olx_gl::translate(T);
    if (!glf.IsVectorFont()) {
      double s = Scale;
      if (Parent.GetViewZoom() != 1)
        s /= Parent.GetExtraZoom();
      olx_gl::scale(s, s, 1.0);
    }
    else {
      const double scale = Parent.GetBasis().GetZoom() / Parent.CalcZoom();
      olx_gl::scale(scale, scale, 1.0);
    }
    P.Vertices[0] = vec3d(text_rect.left, text_rect.top, 0);
    P.Vertices[1] = vec3d(text_rect.left + text_rect.width, text_rect.top, 0);
    P.Vertices[2] = vec3d(text_rect.left + text_rect.width,
      text_rect.top + text_rect.height, 0);
    P.Vertices[3] = vec3d(text_rect.left, text_rect.top + text_rect.height, 0);
  }
  return false;
}
//..............................................................................
TGlFont& TXGlLabel::GetFont() const {  return Parent.GetScene().GetFont(FontIndex, true);  }
//..............................................................................
void TXGlLabel::ToDataItem(TDataItem& item) const {
  item.AddField("text", FLabel);
  item.AddField("visible", IsVisible());
  item.AddField("font_id", FontIndex);
  item.AddField("offset", PersUtil::VecToStr(GetOffset()));
  item.AddField("center", PersUtil::VecToStr(GetCenter()));
}
//..............................................................................
void TXGlLabel::FromDataItem(const TDataItem& item) {
  SetVisible(item.GetFieldByName("visible").ToBool());
  FontIndex = item.GetFieldByName("font_id").ToInt();
  SetLabel(item.GetFieldByName("text"));
  TDataItem* basis = item.FindItem("Basis");
  if (basis != 0) {
    PersUtil::VecFromStr(item.GetFieldByName("center"), Offset);
    TEBasis b;
    b.FromDataItem(*basis);
    _Center = b.GetCenter();
  }
  else {
    PersUtil::VecFromStr(item.GetFieldByName("offset"), Offset);
    PersUtil::VecFromStr(item.GetFieldByName("center"), _Center);
  }
}
//..............................................................................
const_strlist TXGlLabel::ToPov(olx_cdict<TGlMaterial, olxstr> &materials) const
{
  TStrList out;
  TGlPrimitive *glp = GetPrimitives().FindPrimitiveByName("Text");
  if (glp == 0) {
    return out;
  }
  const TGPCollection &gpc = GetPrimitives();
  vec3d off = GetOffset() + (GetCenter() +
    vec3d(-text_rect.width/2,text_rect.height/2,0))*Parent.GetScale();
  pov::CrdTransformer crdc(Parent.GetBasis());
  vec3d T = crdc.crd(off);
  olxstr p_mat = pov::get_mat_name(glp->GetProperties(), materials);
  out.Add("  text { ttf \"timrom.ttf\" ").quote('"') <<
    exparse::parser_util::escape(FLabel) << ' ' << "0.25" <<
      " 0 texture {" <<      p_mat << "}";
  out.Add("   scale ") << 0.5;
  out.Add("   translate ") << pov::to_str(T);
  out.Add("  }");
  return out;
}
//..............................................................................
const_strlist TXGlLabel::ToWrl(olx_cdict<TGlMaterial, olxstr> &materials) const
{
  TStrList out;
  TGlPrimitive *glp = GetPrimitives().FindPrimitiveByName("Text");
  if (glp == 0) {
    return out;
  }
  const TGPCollection &gpc = GetPrimitives();
  vec3d off = GetOffset() + (GetCenter() +
    vec3d(-text_rect.width/2,text_rect.height/2,0))*Parent.GetScale();
  wrl::CrdTransformer crdc(Parent.GetBasis());
  vec3d T = crdc.crd(off);
  olxstr p_mat = wrl::get_mat_str(glp->GetProperties(), materials);
  out.Add("  Transform{ translation").stream(' ') << wrl::to_str(T) <<
    "children Shape{ appearance" << p_mat <<
    "geometry Text { fontStyle FontStyle {size 0.5} string[\"";
  out.GetLastString() << exparse::parser_util::escape(FLabel) << "\"]}}}";
  return out;
}
//..............................................................................
bool TXGlLabel::DoTranslate(const vec3d& t_) {
  vec3d t = TGXApp::GetConstrainedDirection(t_);
  _Center += t;
  return true;
}
//..............................................................................
