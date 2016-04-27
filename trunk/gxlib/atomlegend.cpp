#include "atomlegend.h"
//#include "glprimitive.h"
#include "gxapp.h"
#include "eset.h"

TAtomLegend::TAtomLegend(TGlRenderer& Render, const olxstr& collectionName)
  : AGlMouseHandlerImp(Render, collectionName)
{
  SetMove2D(true);
  SetMoveable(true);
  SetSelectable(false);
  SetVisible(false);
  Top = Left = 0;
  Width = 32;
  Height = 128;
  Z = 0;
  TextureId = ~0;
}
//.............................................................................
void TAtomLegend::Create(const olxstr& cName) {
  if (!cName.IsEmpty()) {
    SetCollectionName(cName);
  }
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if (GPC.PrimitiveCount() != 0) {
    return;
  }
  TGraphicsStyle& GS = GPC.GetStyle();
  Left = GS.GetParam("Left", Left, true).ToInt();
  Top = GS.GetParam("Top", Top, true).ToInt();
  Z = GS.GetParam("Z", Z).ToDouble();
  TGlMaterial glm("3077;0;0");
  TGlPrimitive& GlP = GPC.NewPrimitive("Plane", sgloQuads);
  GlP.SetProperties(GS.GetMaterial("Plane", glm));
  GlP.Vertices.SetCount(4);
  TGlFont &glf = Parent.GetScene().GetFont(
    Parent.GetScene().FindFontIndexForType<TXAtom>(), true);
  TGlPrimitive& glpText = GPC.NewPrimitive("Text", sgloText);
  glpText.SetProperties(GS.GetMaterial("Text", glf.GetMaterial()));
  glpText.SetFont(&glf);
  glpText.Params[0] = -1;

  TGlPrimitive& sp = GPC.NewPrimitive("Sphere", sgloSphere);
  sp.Params[0] = 1; sp.Params[1] = sp.Params[2] = 10;
  sp.SetProperties(glm);
}
//.............................................................................
void TAtomLegend::Fit() {
  TGlFont &glf = Parent.GetScene().GetFont(
    Parent.GetScene().FindFontIndexForType<TXAtom>(), true);
  const uint16_t th = glf.TextHeight(EmptyString());
  const double LineSpacer = 0.05*th;
  Height = glf.GetMaxHeight() * text.Count();
  Height += (uint16_t)olx_round(LineSpacer*(text.Count() - 1));
}
//.............................................................................
bool TAtomLegend::OnMouseUp(const IOlxObject *Sender,
  const TMouseData& Data)
{
  Left = olx_round(Left + GetCenter()[0]);
  Top = olx_round(Top - GetCenter()[1]);
  Center.Null();
  GetPrimitives().GetStyle().SetParam("Top", Top, true);
  GetPrimitives().GetStyle().SetParam("Left", Left, true);
  return AGlMouseHandlerImp::OnMouseUp(Sender, Data);
}
//.............................................................................
bool TAtomLegend::Orient(TGlPrimitive& P) {
  if (Width == 0 || Height == 0 || text.IsEmpty()) {
    return true;
  }
  olx_gl::normal(0, 0, 1);
  const double es = Parent.GetExtraZoom()*Parent.GetViewZoom();
  if (P.GetType() == sgloText) {
    TGlFont &glf = Parent.GetScene().GetFont(
      Parent.GetScene().FindFontIndexForType<TXAtom>(), true);
    const uint16_t th = glf.TextHeight(EmptyString());
    const double hw = Parent.GetWidth() / 2;
    const double hh = Parent.GetHeight() / 2;
    const double GlLeft = ((Left + Width + GetCenter()[0])*es - hw) + 5;
    const double scale = Parent.GetViewZoom() == 1.0 ? 1.0 : 1. / Parent.GetExtraZoom();
    const double GlTop = (hh - (Top - GetCenter()[1])*es - Height*scale) + 0.1;
    const double LineSpacer = 0.05*th;
    vec3d T(GlLeft, GlTop, Z);
    for (size_t i = 0; i < text.Count(); i++) {
      const size_t ii = text.Count() - i - 1;
      olxstr line = text[ii].SubStringTo(
        glf.LengthForWidth(text[ii], Parent.GetWidth()));
      const TTextRect tr = glf.GetTextRect(line);
      //T[1] -= tr.top*scale;
      Parent.DrawTextSafe(T, line, glf);
      //T[1] += (olx_max(tr.height, glf.GetMaxHeight()) + LineSpacer)*scale;
      T[1] += (glf.GetMaxHeight() + LineSpacer)*scale;
    }
    return true;
  }
  else if (P.GetType() == sgloQuads) {
    P.SetTextureId(TextureId);
    double Scale = Parent.GetScale()*es;
    const double hw = Parent.GetWidth() / (2 * es);
    const double hh = Parent.GetHeight() / (2 * es);
    double xx = GetCenter()[0], xy = -GetCenter()[1];
    const double z = Z - 0.01;
    double w = Width,
      h = Height / Parent.GetExtraZoom();
    P.Vertices[0] = vec3d((Left + w + xx - hw)*Scale, -(Top + h + xy - hh)*Scale, z);
    P.Vertices[1] = vec3d(P.Vertices[0][0], -(Top + xy - hh)*Scale, z);
    P.Vertices[2] = vec3d((Left + xx - hw)*Scale, -(Top + xy - hh)*Scale, z);
    P.Vertices[3] = vec3d(P.Vertices[2][0], -(Top + h + xy - hh)*Scale, z);
    return false;
  }
  else if (P.GetType() == sgloSphere) {
    TGlFont &glf = Parent.GetScene().GetFont(
      Parent.GetScene().FindFontIndexForType<TXAtom>(), true);
    const uint16_t th = glf.TextHeight(EmptyString());
    const double LineSpacer = 0.05*th;
    double Scale = Parent.GetScale();
    olx_gl::scale(Scale);
    const double hw = Parent.GetWidth() / (2 * es);
    const double hh = Parent.GetHeight() / (2 * es);
    double xx = GetCenter()[0], xy = -GetCenter()[1];
    double scale = Parent.GetViewZoom() == 1.0 ? 1.0 : 1. / Parent.GetExtraZoom();
    const double z = Z - 0.01;
    double w = Width,
      h = Height / Parent.GetExtraZoom();
    vec3d t((Left + w/2 + xx - hw)*es,
      -(Top + xy - hh)*es - scale*glf.GetMaxHeight() / 2, z);
    double sph_scale = scale * glf.GetMaxHeight() / 2;
    scale /= sph_scale; // glf.GetMaxHeight() / 2
    olx_gl::translate(t);
    olx_gl::scale(sph_scale);
    for (size_t i = 0; i < materials.Count(); i++) {
      materials[i].Init(false);
      P.Draw();
      olx_gl::translate(0.0, -(glf.GetMaxHeight()+LineSpacer)*scale, 0.0);
    }
    return true;
  }
  return false;
}
//.............................................................................
void TAtomLegend::Update() {
  TGXApp &app = TGXApp::GetInstance();
  olxset<const cm_Element * , TPointerComparator> elm_set;
  const TAsymmUnit &au = app.XFile().GetAsymmUnit();
  TCAtom *q1 = 0;
  for (size_t i = 0; i < au.AtomCount(); i++) {
    TCAtom &a = au.GetAtom(i);
    if (a.IsDeleted()) {
      continue;
    }
    if (q1 == 0 && a.GetType() == iQPeakZ) {
      q1 = &a;
    }
    elm_set.Add(&a.GetType());
  }
  text.Clear();
  materials.Clear();
  if (elm_set.IsEmpty()) {
    return;
  }
  ElementPList elms(elm_set);
  QuickSorter::Sort(elms, ElementSymbolSorter());
  size_t idx_h = InvalidIndex, idx_c = InvalidIndex;
  for (size_t i = 0; i < elms.Count(); i++) {
    if (elms[i]->z == iCarbonZ) {
      idx_c = i;
    }
    else if (elms[i]->z == iHydrogenZ) {
      idx_h = i;
    }
  }
  if (idx_h != InvalidIndex && idx_c != InvalidIndex) {
    if (idx_c != 0) {
      elms.Move(idx_c, 0);
    }
    if (idx_h != 1) {
      elms.Move(idx_h, 1);
    }
  }
  olx_array_ptr<uint8_t> ld(elms.Count() * 32 * 32 * 3);
  for (size_t i = 0; i < elms.Count(); i++) {
    uint32_t cl = elms[i]->def_color;
    bool set = false;
    if (elms[i]->z == iQPeakZ && q1 != 0) {
      if (!app.AreQPeaksVisible()) {
        continue;
      }
      TXAtom::GetDefSphereMaterial(*q1, materials.Add(TGlMaterial()),
        app.GetRenderer());
      set = true;
    }
    else {
      if (elms[i]->z == iHydrogenZ) {
        if (!app.AreHydrogensVisible()) {
          continue;
        }
      }
      TGraphicsStyle *st = app.GetRenderer().GetStyles().FindStyle(
        elms[i]->symbol);
      if (st != 0) {
        TGlMaterial *m = st->FindMaterial("Sphere");
        if (m != 0) {
          cl = m->AmbientF.GetRGB();
          materials.Add(*m);
          set = true;
        }
      }
    }
    if (!set) {
      TGlMaterial &m = materials.Add(TGlMaterial());
      m.SetFlags(sglmAmbientF);
      m.AmbientF = cl;
    }
    text.Add(elms[i]->symbol);
  }
  Fit();
}
//.............................................................................
