/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "gltextbox.h"
#include "styles.h"
#include "glrender.h"
#include "gpcollection.h"
#include "glprimitive.h"
#include "glfont.h"

TGlTextBox::TGlTextBox(TGlRenderer& Render, const olxstr& collectionName):
  AGlMouseHandlerImp(Render, collectionName)
{
  SetMove2D(true);
  SetMoveable(true);
  SetRoteable(false);
  SetZoomable(false);

  LineSpacing = 1;
  Left = Top = 0;
  Width = Height = 0;
  MaxStringLength = 0;
  SetSelectable(false);
  FontIndex = ~0;
  ScrollDirectionUp = true;
  Z = 0;
}
//..............................................................................
TGlTextBox::~TGlTextBox()  {  Clear();  }
//..............................................................................
void TGlTextBox::Create(const olxstr& cName) {
  if (!cName.IsEmpty())
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if (GPC.PrimitiveCount() != 0) {
    return;
  }

  TGraphicsStyle& GS = GPC.GetStyle();
  Left = GS.GetParam("Left", Left, true).ToInt();
  Top = GS.GetParam("Top", Top, true).ToInt();
  TGlMaterial GlM;
  GlM.SetFlags(0);
  GlM.ShininessF = 128;
  GlM.SetFlags(sglmAmbientF | sglmDiffuseF | sglmIdentityDraw | sglmTransparent);
  GlM.AmbientF = 0xff0f0f0f;
  GlM.DiffuseF = 0x000f0f0f;

  TGlPrimitive& glpPlane = GPC.NewPrimitive("Plane", sgloQuads);
  glpPlane.SetProperties(GS.GetMaterial("Plane", GlM));
  glpPlane.Vertices.SetCount(4);

  TGlPrimitive& glpText = GPC.NewPrimitive("Text", sgloText);
  glpText.SetProperties(GS.GetMaterial("Text", GetFont().GetMaterial()));
  glpText.Params[0] = -1;  //bitmap; TTF by default
}
//..............................................................................
bool TGlTextBox::Orient(TGlPrimitive& P) {
  olx_gl::normal(0, 0, 1);
  TGlFont& glf = GetFont();
  const double es = Parent.GetExtraZoom()*Parent.GetViewZoom();
  if (P.GetType() == sgloText) {
    const uint16_t th = glf.TextHeight(EmptyString());
    double GlLeft = ((Left + GetCenter()[0])*es - Parent.GetWidth() / 2.0) + 0.1;
    double scale = Parent.GetViewZoom() == 1.0 ? 1.0 : 1. / Parent.GetExtraZoom();
    double GlTop = (Parent.GetHeight() / 2.0 - (Top - GetCenter()[1])*es - Height * scale) + 0.1;
    double LineSpacer = (0.05 + LineSpacing - 1)*th;
    bool mat_changed = false;
    double max_h = glf.GetMaxHeight();
    if (glf.IsVectorFont()) {
      double sc = Parent.GetScale();
      max_h *= sc;
      GlLeft *= sc;
      GlTop *= sc;
      LineSpacer *= sc;
    }
    vec3d T(GlLeft, GlTop, Z);
    for (size_t i = 0; i < FBuffer.Count(); i++) {
      const size_t ii = FBuffer.Count() - i - 1;
      TGlMaterial* GlM = FBuffer.GetObject(ii);
      if (GlM != 0) {
        GlM->Init(Parent.ForcePlain());
        mat_changed = true;
      }
      olxstr line = FBuffer[ii].SubStringTo(
        glf.LengthForWidth(FBuffer[ii], Parent.GetWidth()));
      const TTextRect tr = glf.GetTextRect(line);
      if (glf.IsVectorFont()) {
        T[1] -= tr.top;
        glf.DrawVectorText(T, line, 1);
      }
      else {
        T[1] -= tr.top*scale;
        Parent.DrawTextSafe(T, line, glf);
      }
      T[1] += (olx_max(tr.height, max_h) + LineSpacer)*scale;
      if (mat_changed) {
        P.GetProperties().Init(Parent.ForcePlain());
        mat_changed = false;
      }
    }
    return true;
  }
  else {
    double Scale = Parent.GetScale()*es;
    double hw = Parent.GetWidth() / (2 * es), w = Width;
    if (FBuffer.Count() == 1) {
      w /= Parent.GetExtraZoom();
    }
    const double hh = Parent.GetHeight() / (2 * es), h = Height / Parent.GetExtraZoom();
    double xx = GetCenter()[0], xy = -GetCenter()[1];
    const double z = Z - 0.01;
    P.Vertices[0] = vec3d((Left + w + xx - hw)*Scale, -(Top + h + xy - hh)*Scale, z);
    P.Vertices[1] = vec3d(P.Vertices[0][0], -(Top + xy - hh)*Scale, z);
    P.Vertices[2] = vec3d((Left + xx - hw)*Scale, -(Top + xy - hh)*Scale, z);
    P.Vertices[3] = vec3d(P.Vertices[2][0], -(Top + h + xy - hh)*Scale, z);
    return false;
  }
}
//..............................................................................
void TGlTextBox::Clear() {
  for (size_t i = 0; i < FBuffer.Count(); i++) {
    if (FBuffer.GetObject(i) != 0) {
      delete FBuffer.GetObject(i);
    }
  }
  FBuffer.Clear();
  Width = Height = 0;
}
//..............................................................................
void TGlTextBox::PostText(const olxstr& S, TGlMaterial* M) {
  if (S.IndexOf('\n') != InvalidIndex) {
    TStrList toks(S, '\n');
    PostText(toks, M);
    return;
  }
  if (MaxStringLength && (S.Length() > MaxStringLength)) {
    TStrList Txt;
    Txt.Hyphenate(S, MaxStringLength, true);
    PostText(Txt, M);
    return;
  }
  FBuffer.Add(S, M != 0 ? new TGlMaterial(*M) : 0);
  const size_t width = GetFont().TextWidth(S);
  if (width > Width) {
    Width = (uint16_t)(width + 3);
  }
}
//..............................................................................
void TGlTextBox::PostText(const TStrList &SL, TGlMaterial *M) {
  size_t position = FBuffer.Count();
  for (size_t i = 0; i < SL.Count(); i++) {
    PostText(SL[i], 0);
  }
  if (M != 0) {
    FBuffer.GetObject(position) = new TGlMaterial(*M);
  }
}
//..............................................................................
void TGlTextBox::Fit() {
  const TGlFont& glf = GetFont();
  double scale = 1;
  if (glf.IsVectorFont()) {
    scale = 1. / Parent.GetScale();
  }
  if (FBuffer.Count() > 1) {
    const uint16_t th = glf.TextHeight(EmptyString());
    const double LineSpacer = (0.05 + LineSpacing - 1) * th;
    Height = 0;
    Width = 0;
    double h = 0;
    for (size_t i = 0; i < FBuffer.Count(); i++) {
      TTextRect tr = glf.GetTextRect(FBuffer[FBuffer.Count()-i-1]);
      if (glf.IsVectorFont()) {
        tr.height *= scale;
        tr.width *= scale;
      }
      h -= tr.top * scale;
      h += olx_max(tr.height, glf.GetMaxHeight());
      if (tr.width > Width) {
        Width = olx_round_t<double, uint16_t>(tr.width);
      }
    }
    h += LineSpacer * (FBuffer.Count() - 1);
    Height = static_cast<uint16_t>(h);
  }
  else if (FBuffer.Count() == 1) {
    TTextRect tr = glf.GetTextRect(FBuffer[0]);
    if (glf.IsVectorFont()) {
      tr.height *= scale;
      tr.width *= scale;
    }
    Height = (uint16_t)olx_round(tr.height);
    Width = (uint16_t)olx_round(tr.width);
  }
  else {
    Height = Width = 0;
  }
}
//..............................................................................
void TGlTextBox::SetLeft(int l) {
  Left = l;
  GetPrimitives().GetStyle().SetParam("Left", Left, true);
}
//..............................................................................
void TGlTextBox::SetTop(int t) {
  Top = t;
  GetPrimitives().GetStyle().SetParam("Top", Top, true);
}
//..............................................................................
bool TGlTextBox::OnMouseUp(const IOlxObject *Sender, const TMouseData& Data)  {
  SetLeft((int)(Left + GetCenter()[0]));
  SetTop((int)(Top - GetCenter()[1]));
  Center.Null();
  return AGlMouseHandlerImp::OnMouseUp(Sender, Data);
}
//..............................................................................
TGlFont& TGlTextBox::GetFont() const {
  return Parent.GetScene().GetFont(FontIndex, true);
}
//..............................................................................
//..............................................................................
//..............................................................................
void TGlTextBox::LibReset(const TStrObjList& Params, TMacroData& E) {
  TStrList toks(Params[0], ',');
  if (toks.Count() == 2 && olx_list_and(toks, &olxstr::IsInt)) {
    SetPosition(toks[0].ToInt(), toks[1].ToInt());
  }
  else {
    int margin = 0;
    bool bottom = Params[0].Contains("b"),
      right = Params[0].Contains("r");
    if (!toks.IsEmpty() && toks.GetLastString().IsInt()) {
      margin = toks.GetLastString().ToInt();
    }
    int t = margin, l = margin;
    if (right) {
      l = Parent.GetWidth() - GetWidth() - margin;
    }
    if (bottom) {
      t = Parent.GetHeight() - GetHeight() - margin;
    }
    SetPosition(l, t);
  }
  Update();
}
//..............................................................................
void TGlTextBox::LibFit(const TStrObjList& Params, TMacroData& E) {
  Fit();
}
//..............................................................................
void TGlTextBox::LibPostText(const TStrObjList& Params, TMacroData& E) {
  if (Params.Count() == 2) {
    TGlMaterial m(Params[1]);
    PostText(Params[0], &m);
  }
  else {
    PostText(Params[0]);
  }
}
//..............................................................................
void TGlTextBox::LibClear(const TStrObjList& Params, TMacroData& E) {
  Clear();
}
//..............................................................................
void TGlTextBox::ExportLibrary(TLibrary& lib) {
  AGDrawObject::ExportLibrary(lib);

  lib.Register(new TFunction<TGlTextBox>(this, &TGlTextBox::LibReset,
    "Reset", fpOne, "Resets the text box position"));
  lib.Register(new TFunction<TGlTextBox>(this, &TGlTextBox::LibFit,
    "Fit", fpNone, "Fits box size to the content"));
  lib.Register(new TFunction<TGlTextBox>(this, &TGlTextBox::LibPostText,
    "PostText", fpOne|fpTwo, "Posts a text line into the box with optional material"));
  lib.Register(new TFunction<TGlTextBox>(this, &TGlTextBox::LibClear ,
    "Clear", fpNone, "Clears the text"));
}
//..............................................................................
