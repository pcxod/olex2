/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "atomlegend.h"
//#include "glprimitive.h"
#include "gxapp.h"
#include "xcartoon.h"
#include "eset.h"

namespace {
  /* a swatch drawn the way the ribbon is: the material from the cartoon
  itself, colour material on, the colour through glColor. Reproduced rather
  than re-derived - working out what the lights should do to a colour and
  building a material to match is how the ribbon ended up white
  */
  TGlMaterial SwatchMaterial(uint32_t cl) {
    TGlMaterial m = gxlib::cartoon_colour::RibbonMaterial(cl);
    m.SetColorMaterial(true);
    return m;
  }
  void EmitColour(uint32_t cl) {
    olx_gl::color((float)OLX_GetRValue(cl) / 255,
      (float)OLX_GetGValue(cl) / 255,
      (float)OLX_GetBValue(cl) / 255,
      (float)OLX_GetAValue(cl) / 255);
  }
  /* a short length of ribbon with one twist, in the unit box the sphere swatch
  occupies. The twist is what makes it read as a ribbon rather than a bar at
  this size, and it shows both faces
  */
  void DrawRibbonSwatch(uint32_t cl) {
    const int n = 14;
    const double half_length = 1.15, half_width = 0.40, twist = 0.85;
    const double two_pi = 6.283185307179586;
    olx_gl::colorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    EmitColour(cl);
    olx_gl::begin(GL_QUAD_STRIP);
    for (int i = 0; i <= n; i++) {
      const double t = (double)i / n;
      const double x = -half_length + 2 * half_length*t;
      const double a = twist*sin(two_pi*t);
      const double ca = cos(a), sa = sin(a);
      olx_gl::normal(0.0, -sa, ca);
      olx_gl::vertex(x, half_width*ca, half_width*sa);
      olx_gl::vertex(x, -half_width*ca, -half_width*sa);
    }
    olx_gl::end();
  }
}

TAtomLegend::TAtomLegend(TGlRenderer& Render, const olxstr& collectionName)
  : AGlMouseHandlerImp(Render, collectionName)
{
  SetMove2D(true);
  SetMoveable(true);
  SetSelectable(false);
  Top = Left = 0;
  first_ribbon_row = InvalidIndex;
  FirstSpacer = 5;
  ColWidth = 16;
  Width = ColWidth*2;
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
  Left = GS.FindNumParam("Left", Left);
  Top = GS.FindNumParam("Top", Top);
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
  sp.Params[0] = 1; sp.Params[1] = sp.Params[2] = 16;
  sp.SetProperties(glm);
  AGDrawObject::SetVisible(GS.GetParam("visible", TrueString(), true).ToBool());
}
//.............................................................................
void TAtomLegend::Fit() {
  TGlFont &glf = Parent.GetScene().GetFont(
    Parent.GetScene().FindFontIndexForType<TXAtom>(), true);
  const uint16_t th = glf.TextHeight(EmptyString());
  const double LineSpacer = 0.05*th;
  ColWidth = glf.GetMaxHeight();
  int max_w = 0;
  for (size_t i = 0; i < text.Count(); i++) {
    TTextRect r = glf.GetTextRect(text[i]);
    int w = olx_round(r.left + r.width);
    if (w > max_w) {
      max_w = w;
    }
  }
  Width = ColWidth + max_w + FirstSpacer;
  Height = static_cast<int>(glf.GetMaxHeight() * text.Count());
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
  TGlFont &glf = Parent.GetScene().GetFont(
    Parent.GetScene().FindFontIndexForType<TXAtom>(), true);
  if (P.GetType() == sgloText) {
    uint16_t th = glf.TextHeight(EmptyString());
    const double hw = Parent.GetWidth() / 2;
    const double hh = Parent.GetHeight() / 2;
    double scale = glf.IsVectorFont() ? es
      : (Parent.GetViewZoom() == 1.0 ? 1.0 : 1. / Parent.GetExtraZoom());
    const double GlLeft = ((Left + GetCenter()[0])*es + ColWidth*scale - hw) + FirstSpacer;
    const double GlTop = (hh - (Top - GetCenter()[1])*es - Height*scale) +
      (glf.IsVectorFont() ? glf.GetPointSize()*scale/2.5 : 0.1);
    const double LineSpacer = 0.05*th;
    vec3d T(GlLeft, GlTop, Z);
    for (size_t i = 0; i < text.Count(); i++) {
      const size_t ii = text.Count() - i - 1;
      olxstr line = text[ii].SubStringTo(
        glf.LengthForWidth(text[ii], Parent.GetWidth()));
      const TTextRect tr = glf.GetTextRect(line);
      if (glf.IsVectorFont()) {
        glf.DrawVectorText(T * Parent.GetScale(), line, 1);
      }
      else {
        Parent.DrawTextSafe(T, line, glf);
      }
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
    double w = ColWidth, h = Height;
    if (!glf.IsVectorFont()) {
      w /= Parent.GetExtraZoom();
      h /= Parent.GetExtraZoom();
    }
    P.Vertices[0] = vec3d((Left + w + xx - hw)*Scale, -(Top + h + xy - hh)*Scale, z);
    P.Vertices[1] = vec3d(P.Vertices[0][0], -(Top + xy - hh)*Scale, z);
    P.Vertices[2] = vec3d((Left + xx - hw)*Scale, -(Top + xy - hh)*Scale, z);
    P.Vertices[3] = vec3d(P.Vertices[2][0], -(Top + h + xy - hh)*Scale, z);
    return false;
  }
  else if (P.GetType() == sgloSphere) {
    uint16_t th = glf.TextHeight(EmptyString());
    const double LineSpacer = 0.05*th;
    double Scale = Parent.GetScale();
    olx_gl::scale(Scale);
    const double hw = Parent.GetWidth() / (2 * es);
    const double hh = Parent.GetHeight() / (2 * es);
    double xx = GetCenter()[0], xy = -GetCenter()[1];
    double scale = glf.IsVectorFont() ? es
      : (Parent.GetViewZoom() == 1.0 ? 1.0 : 1. / Parent.GetExtraZoom());
    const double z = Z - 0.01;
    double w = ColWidth, h = Height;
    if (!glf.IsVectorFont()) {
      w /= Parent.GetExtraZoom();
      h /= Parent.GetExtraZoom();
    }
    vec3d t((Left + w/2 + xx - hw)*es,
      -(Top + xy - hh)*es - scale*glf.GetMaxHeight() / 2, z);
    double sph_scale = scale * glf.GetMaxHeight() / 2;
    scale /= sph_scale; // glf.GetMaxHeight() / 2
    olx_gl::translate(t);
    olx_gl::scale(sph_scale);
    for (size_t i = 0; i < materials.Count(); i++) {
      materials[i].Init(Parent.ForcePlain());
      if (i < first_ribbon_row) {
        P.Draw();
      }
      else {
        DrawRibbonSwatch(row_colours[i - first_ribbon_row]);
      }
      olx_gl::translate(0.0, -(glf.GetMaxHeight()+LineSpacer)*scale, 0.0);
    }
    P.GetProperties().Init(Parent.ForcePlain());
    return true;
  }
  return false;
}
//.............................................................................
void TAtomLegend::Update() {
  TGXApp &app = TGXApp::GetInstance();
  olxset<const cm_Element * , TPointerComparator> elm_set;
  const TAsymmUnit &au = app.XFile().GetAsymmUnit();
  TXAtom *q1 = 0;
  TGXApp::AtomIterator ai = app.GetAtoms();
  while (ai.HasNext()) {
    TXAtom &a = ai.Next();
    if (!a.IsVisible()) {
      continue;
    }
    if (q1 == 0 && a.GetType() == iQPeakZ) {
      q1 = &a;
    }
    elm_set.Add(&a.GetType());
  }
  text.Clear();
  materials.Clear();
  row_colours.Clear();
  // no ribbon rows unless AddCartoonKey says otherwise, so every row is a ball
  first_ribbon_row = InvalidIndex;
  /* not "no elements, no legend": with the cartoon hiding everything it traced
  there may be no visible atom at all, and returning here left the cartoon key
  unbuilt. The element block is skipped, the cartoon key is not
  */
  if (elm_set.IsEmpty()) {
    AddCartoonKey();
    Fit();
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
      TXAtom::GetDefSphereMaterial(q1->CAtom(), materials.Add(TGlMaterial()),
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
  AddCartoonKey();
  Fit();
}
//.............................................................................
void TAtomLegend::AddCartoonKey() {
  TGXApp &app = TGXApp::GetInstance();
  if (!app.AreCartoonsVisible() || app.GetCartoons().IsEmpty()) {
    return;
  }
  // everything added from here on is a ribbon rather than a ball
  first_ribbon_row = materials.Count();
  using namespace xlib::protein;
  const short mode = app.GetCartoonColourMode();

  /* appended to the lists the element key uses, so it inherits the dragging,
  the saved position and the reset. Rows are drawn top-down in the order added,
  one swatch and one label each
  */
  if (mode == ccChain) {
    TArrayList<olx_pair_t<olxch, uint32_t> > chains;
    app.GetCartoonChainKey(chains);
    for (size_t i = 0; i < chains.Count(); i++) {
      materials.Add(SwatchMaterial(chains[i].b));
      text.Add(olxstr("Chain ") << chains[i].a);
    }
  }
  else if (mode == ccSecondary) {
    const short kinds[] = { ss_helix, ss_strand, ss_coil };
    const char *names[] = { "Helix", "Strand", "Coil" };
    for (size_t i = 0; i < 3; i++) {
      materials.Add(SwatchMaterial(cartoon_colour::Secondary(kinds[i])));
      text.Add(names[i]);
    }
  }
  else if (mode == ccPolarity) {
    const short kinds[] = { rpLipophilic, rpHydrophilic };
    const char *names[] = { "Lipophilic", "Hydrophilic" };
    for (size_t i = 0; i < 2; i++) {
      materials.Add(SwatchMaterial(cartoon_colour::Polarity(kinds[i])));
      text.Add(names[i]);
    }
  }
  else if (mode == ccCharge) {
    const short kinds[] = { rcAcidic, rcBasic, rcNeutral };
    const char *names[] = { "Acidic", "Basic", "Neutral" };
    for (size_t i = 0; i < 3; i++) {
      materials.Add(SwatchMaterial(cartoon_colour::Charge(kinds[i])));
      text.Add(names[i]);
    }
  }
  else if (mode == ccResidue) {
    TArrayList<olx_pair_t<size_t, uint32_t> > resi;
    app.GetCartoonResidueKey(resi);
    for (size_t i = 0; i < resi.Count(); i++) {
      materials.Add(SwatchMaterial(resi[i].b));
      text.Add(resi[i].a == InvalidIndex
        ? olxstr("Other") : StandardResidueName(resi[i].a));
    }
  }
  else if (mode == ccIndex || mode == ccUeq) {
    /* a colour bar as a column of swatches down the legend's own rows rather
    than a separate gradient object: discrete, but at this many steps it reads
    as a bar and costs no new primitive or draggable. The scale runs high at
    the top, as colour bars are drawn, so the ramp is walked backwards
    */
    const size_t steps = 9;
    double u_min = 0, u_max = 0;
    const bool have_u = (mode == ccUeq) &&
      app.GetCartoonUeqRange(u_min, u_max);
    for (size_t i = 0; i < steps; i++) {
      const double t = double(steps - 1 - i)/(steps - 1);
      materials.Add(SwatchMaterial(gxlib::cartoon::RainbowColour(t)));
      /* labelled at the ends and the middle only - every step is unreadable at
      this row height, and the rest still line up, text and swatches coming
      from the same row index
      */
      if (mode == ccUeq) {
        if (!have_u) {
          text.Add(i == 0 ? olxstr("Ueq high")
            : (i + 1 == steps ? olxstr("Ueq low") : olxstr()));
        }
        else if (i == 0 || i + 1 == steps || i == steps/2) {
          text.Add(olxstr::FormatFloat(3, u_min + (u_max - u_min)*t));
        }
        else {
          text.Add(EmptyString());
        }
      }
      else {
        text.Add(i == 0 ? olxstr("C term")
          : (i + 1 == steps ? olxstr("N term") : olxstr()));
      }
    }
  }
  /* taken back off the materials rather than threaded through every branch
  above, the diffuse term being where SwatchMaterial put it. Alpha forced
  opaque: GetRGB drops it, and zero would draw nothing
  */
  for (size_t i = first_ribbon_row; i < materials.Count(); i++) {
    row_colours.Add(materials[i].DiffuseF.GetRGB() | 0xff000000);
  }
}
//.............................................................................
void TAtomLegend::SetVisible(bool v) {
  AGDrawObject::SetVisible(v);
  if (GetPrimitives().HasStyle()) {
    GetPrimitives().GetStyle().SetParam("visible",
      v ? TrueString() : FalseString(), true);
  }
}
//.............................................................................
void TAtomLegend::SetPosition(int left, int top) {
  GetPrimitives().GetStyle().SetParam("Top", Top = top, true);
  GetPrimitives().GetStyle().SetParam("Left", Left = left, true);
  Center.Null();
}
//.............................................................................
void TAtomLegend::ResetPosition(bool right, bool bottom, int margin) {
  int top = margin, left = margin;
  if (right) {
    left = Parent.GetWidth() - GetWidth() - margin;
  }
  if (bottom) {
    top = Parent.GetHeight() - GetHeight() - margin;
  }
  SetPosition(left, top);
}
//.............................................................................
