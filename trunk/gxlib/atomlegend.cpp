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
void TAtomLegend::SetData(unsigned char *rgb,
  GLsizei width, GLsizei height, GLenum format)
{
  if (olx_is_valid_index(TextureId)) {
    TGlTexture* tex = Parent.GetTextureManager().FindTexture(TextureId);
    Parent.GetTextureManager().Replace2DTexture(
      *tex, 0, width, height, 0, format, rgb);
  }
  else {
    TextureId = Parent.GetTextureManager().Add2DTexture(
      GetCollectionName(), 0, width, height, 0,
      format, rgb);
    TGlTexture* tex = Parent.GetTextureManager().FindTexture(TextureId);
    tex->SetEnvMode(tpeDecal);
    tex->SetSCrdWrapping(tpCrdClamp);
    tex->SetTCrdWrapping(tpCrdClamp);

    tex->SetMagFilter(tpFilterNearest);
    tex->SetMinFilter(tpFilterLinear);
    tex->SetEnabled(true);
  }
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

  TGlPrimitive& GlP = GPC.NewPrimitive("Plane", sgloQuads);
  GlM.SetIdentityDraw(true);
  GlP.SetProperties(GlM);
  // texture coordinates
  GlP.TextureCrds.SetCount(4);
  GlP.Vertices.SetCount(4);
  GlP.TextureCrds[0].s = 0;  GlP.TextureCrds[0].t = 1;
  GlP.TextureCrds[1].s = 0;  GlP.TextureCrds[1].t = 0;
  GlP.TextureCrds[2].s = 1;  GlP.TextureCrds[2].t = 0;
  GlP.TextureCrds[3].s = 1;  GlP.TextureCrds[3].t = 1;

  TGlFont &glf = Parent.GetScene().GetFont(
    Parent.GetScene().FindFontIndexForType<TXAtom>(), true);
  TGlPrimitive& glpText = GPC.NewPrimitive("Text", sgloText);
  glpText.SetProperties(GS.GetMaterial("Text", glf.GetMaterial()));
  glpText.SetFont(&glf);
  glpText.Params[0] = -1;
  if (tex_data.is_valid()) {
    SetData(tex_data(), 32, text.Count()*32, GL_RGB);
  }
}
//.............................................................................
void TAtomLegend::Fit() {
  TGlFont &glf = Parent.GetScene().GetFont(
    Parent.GetScene().FindFontIndexForType<TXAtom>(), true);
  const uint16_t th = glf.TextHeight(EmptyString());
  const double LineSpacer = 0.05*th;
  Height = 0;
  for (size_t i = 0; i < text.Count(); i++) {
    const TTextRect tr = glf.GetTextRect(text[i]);
    Height -= (uint16_t)olx_round(tr.top);
    Height += (uint16_t)olx_round(olx_max(tr.height, glf.GetMaxHeight()));
  }
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
      T[1] -= tr.top*scale;
      Parent.DrawTextSafe(T, line, glf);
      T[1] += (olx_max(tr.height, glf.GetMaxHeight()) + LineSpacer)*scale;
    }
    return true;
  }
  else {
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
}
//.............................................................................
void TAtomLegend::Update() {
  TGXApp &app = TGXApp::GetInstance();
  olxset<const cm_Element * , TPointerComparator> elms;
  const TAsymmUnit &au = app.XFile().GetAsymmUnit();
  for (size_t i = 0; i < au.AtomCount(); i++) {
    TCAtom &a = au.GetAtom(i);
    if (a.IsDeleted()) {
      continue;
    }
    elms.Add(&a.GetType());
  }
  text.Clear();
  if (elms.IsEmpty()) {
    return;
  }
  olx_array_ptr<uint8_t> ld(elms.Count()*32*32*3);
  for (size_t i = 0; i < elms.Count(); i++) {
    text.Add(elms.Get(i)->symbol);
    uint32_t cl = elms.Get(i)->def_color;
    TGraphicsStyle *st = app.GetRenderer().GetStyles().FindStyle(
      elms.Get(i)->symbol);
    if (st != 0) {
      TGlMaterial *m = st->FindMaterial("Sphere");
      if (m != 0) {
        cl = m->AmbientF.GetRGB();
      }
    }
    const size_t off = 32 * 32 * 3 * i;
    for (size_t x = 0; x < 32; x++) {
      for (size_t y = 0; y < 32; y++) {
        const size_t off1 = off + (x*32 + y) * 3;
        ld()[off1 + 0] = OLX_GetRValue(cl);
        ld()[off1 + 1] = OLX_GetGValue(cl);
        ld()[off1 + 2] = OLX_GetBValue(cl);
      }
    }
  }
  SetData(ld(), 32, 32 * elms.Count(), GL_RGB);
  tex_data = ld;
  Fit();
}
//.............................................................................
