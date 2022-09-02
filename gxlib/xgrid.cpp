/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xgrid.h"
#include "gpcollection.h"
#include "styles.h"
#include "glmaterial.h"
#include "glrender.h"
#include "efile.h"
#include "gxapp.h"
#include "library.h"
#include "conrec.h"
#include "pers_util.h"
#include "maputil.h"
#include "ememstream.h"
#include "povdraw.h"

#ifdef _PYTHON
  #include "pyext.h"
#endif

/* heatmap colours:
http://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients
*/

vec3i CalculateColour(float v, size_t colour_count, const vec3i* colours, bool reverse) {
  if (reverse) {
    v = -v;
  }
  v = ((v + 1) / 2) * (colour_count - 1);
  // should not happen as v should be normalised to +/-1, but
  if (v < 0) {
    v = 0;
  }
  size_t idx1 = olx_floor(v),
    idx2 = idx1 + 1;
  if (idx2 >= colour_count) {
    idx2 = colour_count - 1;
  }
  v = v - idx1;
  vec3i rv;
  for (int ci = 0; ci < 3; ci++) {
    float x = ((colours[idx2][ci] - colours[idx1][ci])*v + colours[idx1][ci]);
    rv[ci] = (char) x;
  }
  return rv;
}

const vec3i* Get7Colours() {
  const int NUM_COLORS = 7;
  static const vec3i colours[NUM_COLORS] = {
    vec3i(255,255,255), vec3i(255,0,0), vec3i(255,255,0),
    vec3i(0,255,0), vec3i(0,255,255), vec3i(0,0,255), vec3i(0,0,0)
  };
  return colours;
}

const vec3i* Get5Colours() {
  const int NUM_COLORS = 5;
  static vec3i colours[NUM_COLORS] = {
    vec3i(255,0,0), vec3i(255,255,0),
    vec3i(0,255,0), vec3i(0,255,255), vec3i(0,0,255)
  };
  return colours;
}

TXGrid::TLegend::TLegend(TGlRenderer& Render, const olxstr& collectionName)
  : AGlMouseHandlerImp(Render, collectionName)
{
  SetMove2D(true);
  SetMoveable(true);
  SetSelectable(false);
  Top = Left = 0;
  Width = 64;
  Height = 128;
  Z = 0;
  TextureId = ~0;
}
//.............................................................................
void TXGrid::TLegend::SetData(unsigned char *rgb,
  GLsizei width, GLsizei height, GLenum format)
{
  if (olx_is_valid_index(TextureId)) {
    TGlTexture* tex = Parent.GetTextureManager().FindTexture(TextureId);
    Parent.GetTextureManager().Replace2DTexture(
      *tex, 0, width, height, 0, format, rgb);
  }
  else {
    TextureId = Parent.GetTextureManager().Add2DTexture(
      GetCollectionName(), 0, Width, Height, 0,
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
void TXGrid::TLegend::Create(const olxstr& cName) {
  if (!cName.IsEmpty()) {
    SetCollectionName(cName);
  }
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if (GPC.PrimitiveCount() != 0) {
    return;
  }
  TGlFont& glf = Parent.GetScene().GetFont(~0, true);
  TGraphicsStyle& GS = GPC.GetStyle();
  Left = GS.FindNumParam("Left", Left);
  // offest to by height to avoid overlap with text
  Top = GS.FindNumParam("Top", (int)glf.GetMaxHeight());
  Z = GS.FindNumParam("Z", Z);
  {
    TGlPrimitive& GlP = GPC.NewPrimitive("Plane", sgloQuads);
    GlP.SetTextureId(TextureId);
    GlM.SetIdentityDraw(true);
    GlP.SetProperties(GlM);
    // texture coordinates
    GlP.TextureCrds.SetCount(4);
    GlP.Vertices.SetCount(4);
    GlP.TextureCrds[0].s = 0;  GlP.TextureCrds[0].t = 1;
    GlP.TextureCrds[1].s = 0;  GlP.TextureCrds[1].t = 0;
    GlP.TextureCrds[2].s = 1;  GlP.TextureCrds[2].t = 0;
    GlP.TextureCrds[3].s = 1;  GlP.TextureCrds[3].t = 1;
  }
  {
    TGlPrimitive& GlP = GPC.NewPrimitive("Plane-text", sgloQuads);
    TGlMaterial* gm = GS.FindMaterial(GlP.GetName(), &GlM);
    GlP.SetProperties(*gm);
    GlP.Vertices.SetCount(4);
  }

  TGlPrimitive& glpText = GPC.NewPrimitive("Text", sgloText);
  glpText.SetProperties(GS.GetMaterial("Text", glf.GetMaterial()));
  glpText.SetFont(&glf);
  glpText.Params[0] = -1;
}
//.............................................................................
void TXGrid::TLegend::Fit() {
  TGlFont &glf = Parent.GetScene().GetFont(~0, true);
  const uint16_t th = glf.TextHeight(EmptyString());
  const double LineSpacer = 0.05*th;
  Height = 0;
  size_t trim_l = 10;
  for (size_t i = 0; i < text.Count(); i++) {
    {
      size_t x = text[i].Length();
      while (--x != InvalidIndex) {
        if (text[i][x] != '0') {
          if (text[i][x] == '.') {
            x--;
          }
          break;
        }
      }
      x = text[i].Length() - x - 1;
      if (x < trim_l) {
        trim_l = x;
      }
    }
    const TTextRect tr = glf.GetTextRect(text[i]);
    Height -= (uint16_t)olx_round(tr.top);
    Height += (uint16_t)olx_round(olx_max(tr.height, glf.GetMaxHeight()));
  }
  if (trim_l != 0) {
    for (size_t i = 0; i < text.Count(); i++) {
      if (text[i].Length() > trim_l) {
        text[i].SetLength(text[i].Length() - trim_l);
      }
    }
  }
  Height += (uint16_t)olx_round(LineSpacer*(text.Count() - 1));
}
//.............................................................................
bool TXGrid::TLegend::OnMouseUp(const IOlxObject *Sender,
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
bool TXGrid::TLegend::Orient(TGlPrimitive& P) {
  if (Width == 0 || Height == 0 || text.IsEmpty()) {
    return true;
  }
  olx_gl::normal(0, 0, 1);
  Z = Parent.CalcRasterZ(0.1);
  const double es = Parent.GetExtraZoom()*Parent.GetViewZoom();
  if (P.GetType() == sgloText) {
    TGlFont &glf = Parent.GetScene().GetFont(~0, true);
    const uint16_t th = glf.TextHeight(EmptyString());
    const double hw = Parent.GetWidth() / 2;
    const double hh = Parent.GetHeight() / 2;
    const double GlLeft = ((Left + Width + GetCenter()[0])*es - hw) + 0.1;
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
  else if (P.GetName() == "Plane") {
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
    P.Vertices[2] = vec3d((Left + xx - hw)*Scale, P.Vertices[1][1], z);
    P.Vertices[3] = vec3d(P.Vertices[2][0], P.Vertices[0][1], z);
    return false;
  }
  else {
    TGlFont &glf = Parent.GetScene().GetFont(~0, true);
    double Scale = Parent.GetScale();
    double tw = glf.TextWidth(text[0]) * Scale *
     (Parent.GetViewZoom() == 1.0 ? 1.0 : 1. / Parent.GetExtraZoom());
    Scale *= es;
    const double hw = Parent.GetWidth() / (2 * es);
    const double hh = Parent.GetHeight() / (2 * es);
    double xx = GetCenter()[0], xy = -GetCenter()[1];
    const double z = Z - 0.01;
    double w = Width,
      h = Height / Parent.GetExtraZoom();
    P.Vertices[0] = vec3d((Left + xx - hw + w)*Scale, -(Top + h + xy - hh)*Scale, z);
    P.Vertices[1] = vec3d(P.Vertices[0][0] + tw, P.Vertices[0][1], z);
    P.Vertices[2] = vec3d(P.Vertices[1][0], -(Top + xy - hh)*Scale, z);
    P.Vertices[3] = vec3d(P.Vertices[0][0], P.Vertices[2][1], z);
    return false;
  }
}
//.............................................................................
//.............................................................................
//.............................................................................
TXGrid::TContextClear::TContextClear(TGlRenderer& r)  {
  r.OnClear.Add(this);
}
//.............................................................................
bool TXGrid::TContextClear::Enter(const IOlxObject *Sender, const IOlxObject *Data,
  TActionQueue *)
{
  TXGrid::_ResetLists();
  return true;
}
//.............................................................................
bool TXGrid::TContextClear::Exit(const IOlxObject *Sender, const IOlxObject *Data,
  TActionQueue *)
{
  return true;
}
//.............................................................................
//.............................................................................
//.............................................................................
TXGrid* TXGrid::Instance = NULL;
//----------------------------------------------------------------------------//
TXGrid::TXGrid(const olxstr& collectionName, TGXApp* xapp)
: AGDrawObject(xapp->GetRenderer(), collectionName),
  ExtMin(-1,-1,-1),
  ExtMax(1,1,1)
{
  if (Instance != 0) {
    throw TFunctionFailedException(__OlxSourceInfo, "singleton");
  }
  AGDrawObject::SetSelectable(false);
  new TContextClear(Parent);
  Mask = 0;
  Instance = this;
  Loading_ = Boxed = Extended = false;
  RenderMode = planeRenderModeFill;
  XApp = xapp;
  Depth = 0;
  ED = 0;
  ColorData = 0;
  MouseDown = false;
  Size = 10;
  // texture related data
  TextIndex = ~0;
  TextData = 0;
  // contour related data
  ContourCrds[0] = ContourCrds[1] = 0;
  ContourLevels = 0;
  ContourLevelCount = 14;
  Scale = 1;
  //for textures, 2^n+2 (for border)...
  //MaxDim = 128;//olx_max( olx_max(MaxX,MaxY), MaxZ);
  MaxDim = 128;
  Info = new TGlTextBox(Parent, "XGrid_Label");
  Legend = new TLegend(Parent, "XGrid_Legend");
  LegendData = new char[32*32*3];
  MaxX = MaxY = MaxZ = 0;
  MaxVal = MinVal = 0;
  MinHole = MaxHole = 0;
  box_min = box_step = 0;
  PListId = NListId = ~0;
  glpC = glpN = glpP = 0;
  ReverseColors = false;
}
//.............................................................................
TXGrid::~TXGrid() {
  Clear();
  DeleteObjects();
  delete Info;
  delete Legend;
  delete[] LegendData;
  Instance = 0;
}
//.............................................................................
void TXGrid::Clear() {
  DeleteObjects();
  _ResetLists();
}
//.............................................................................
void TXGrid::Create(const olxstr& cName) {
  if (!cName.IsEmpty()) {
    SetCollectionName(cName);
  }
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  TGraphicsStyle& GS = GPC.GetStyle();
  TGlMaterial GlM;
  GlM.SetFlags(0);
  GlM.ShininessF = 128;
  GlM.SetFlags(sglmAmbientF | sglmDiffuseF | sglmTransparent);
  GlM.AmbientF = 0xD80f0f0f;
  GlM.DiffuseF = 0xD80f0f0f;
  Info->Create();
  Legend->SetMaterial(GS.GetMaterial("eMap", GlM));
  Legend->Create();
  if (GPC.PrimitiveCount() != 0) {
    return;
  }

  TGlPrimitive& GlP = GPC.NewPrimitive("eMap", sgloQuads);
  GlP.SetProperties(GS.GetMaterial(GlP.GetName(), GlM));

  TextIndex = ~0;
  GlP.SetTextureId(~0);

  GlP.Vertices.SetCount(4);
  GlP.TextureCrds.SetCount(4);
  // texture coordinates
  GlP.TextureCrds[0].s = 0;  GlP.TextureCrds[0].t = 0;
  GlP.TextureCrds[1].s = 1;  GlP.TextureCrds[1].t = 0;
  GlP.TextureCrds[2].s = 1;  GlP.TextureCrds[2].t = 1;
  GlP.TextureCrds[3].s = 0;  GlP.TextureCrds[3].t = 1;
  // create dummy primitives
  glpP = &GPC.NewPrimitive("+Surface", sgloQuads);
  glpP->SetProperties(GS.GetMaterial("+Surface",
    TGlMaterial("85;0.000,1.000,0.000,0.850;3632300160;1.000,1.000,1.000,0.500;36")));
  glpN = &GPC.NewPrimitive("-Surface", sgloQuads);
  glpN->SetProperties(GS.GetMaterial("-Surface",
    TGlMaterial("85;1.000,0.000,0.000,0.850;3632300160;1.000,1.000,1.000,0.500;36")));

  glpC = &GPC.NewPrimitive("Contour plane", sgloQuads);
  glpC->SetProperties(GS.GetMaterial("Contour plane",
    TGlMaterial("1029;3628944717;645955712")));
  glpC->Vertices.SetCount(4);
  if (!olx_is_valid_index(PListId)) {
    RescaleSurface(false);
  }
}
//.............................................................................
void TXGrid::TPlaneCalculationTask::Run(size_t ind) {
  for (size_t j = 0; j < max_dim; j++) {  // (i,j,Depth)
    vec3f p((float)(ind - hh) / size, (float)(j - hh) / size, depth);
    p = proj_m*p;
    p -= center;
    p *= c2c;
    float val = map_getter.Get(p);
    if (val < minVal) {
      minVal = val;
    }
    if (val > maxVal) {
      maxVal = val;
    }
    data[ind][j] = val;
  }
}
//.............................................................................
bool TXGrid::Orient(TGlPrimitive& GlP) {
  if (ED == 0) {
    return true;
  }
  if (Is3D()) {
    olx_gl::FlagManager fm;
    if (ColorData != 0) {
      fm.enable(GL_COLOR_MATERIAL);
    }
    if (&GlP == glpN) { // draw once only
      olx_gl::callList(PListId);
    }
    else if (&GlP == glpP) { // draw once only
      olx_gl::callList(NListId);
    }
    return true;
  }
  if (&GlP == glpP || &GlP == glpN) {
    return true;
  }
  if (&GlP == glpC) {
    if ((RenderMode & planeRenderModePlane) != 0) {
      return true;
    }
  }
  else {
    if ((RenderMode&planeRenderModeContour) != 0 &&
      (RenderMode&planeRenderModePlane) == 0)
    {
      return true;
    }
  }

  const mat3f bm(Parent.GetBasis().GetMatrix());
  const mat3f c2c(XApp->XFile().GetAsymmUnit().GetCartesianToCell());
  const float hh = (float)MaxDim / 2;
  const vec3f center(Parent.GetBasis().GetCenter());
  const vec3s dim(MaxX, MaxY, MaxZ);
  float Z;
  if ((RenderMode&planeRenderModeContour) != 0) {
    Z = Depth - 0.001f;
  }
  else { // no adjustment is required
    Z = Depth;
  }
  GlP.Vertices[0] = bm*vec3f(-hh / Size, -hh / Size, Z) - center;
  GlP.Vertices[1] = bm*vec3f(hh / Size, -hh / Size, Z) - center;
  GlP.Vertices[2] = bm*vec3f(hh / Size, hh / Size, Z) - center;
  GlP.Vertices[3] = bm*vec3f(-hh / Size, hh / Size, Z) - center;
  if (Parent.IsSelecting()) {
    GlP.SetTextureId(~0);
    return false;
  }
  TPlaneCalculationTask calc_task(*this, ED->Data, ContourData, TextData,
    MaxDim, Size, Depth, bm, c2c, center, RenderMode);
  TListIteratorManager<TPlaneCalculationTask> tasks(calc_task, MaxDim,
    tLinearTask, MaxDim > 64);
  float minVal = 1000, maxVal = -1000;
  if (!olx_feq(box_step, 0.0f)) {
    if (box_step < 0) {
      minVal = box_min + box_step*(ContourLevelCount-1);
      maxVal = box_min - box_step;
    }
    else {
      minVal = box_min;
      maxVal = box_min + box_step*ContourLevelCount;
    }
  }
  else {
    for (size_t i = 0; i < tasks.Count(); i++) {
      if (tasks[i].minVal < minVal) {
        minVal = tasks[i].minVal;
      }
      if (tasks[i].maxVal > maxVal) {
        maxVal = tasks[i].maxVal;
      }
    }
  }
  const TGlOption& start_p =
    GetPrimitives().FindPrimitiveByName("-Surface")->GetProperties().AmbientF;
  const TGlOption& end_p =
    GetPrimitives().FindPrimitiveByName("+Surface")->GetProperties().AmbientF;
  vec3i colours2[] = {
    vec3i(int(start_p[0] * 255), int(start_p[1] * 255), int(start_p[2] * 255)),
    vec3i(int(end_p[0] * 255), int(end_p[1] * 255), int(end_p[2] * 255))
  };
  vec3i colours3[] = { colours2[0], vec3i(255,255,255), colours2[1]};
  const vec3i *colours;
  size_t colour_count;
  short render_m = (RenderMode & 0xff00) >> 8;
  switch (render_m) {
    case 2: {
      colours = &colours2[0];
      colour_count = 2;
      break;
    }
    case 3: {
      colours = &colours3[0];
      colour_count = 3;
      break;
    }
    case 5: {
      colours = Get5Colours();
      colour_count = 5;
      break;
    }
    case 7: {
      colours = Get7Colours();
      colour_count = 7;
      break;
    }
    default: {
      colours = &colours3[0];
      colour_count = 3;
    }
  }
  olx_gl::normal(bm[0][2], bm[1][2], bm[2][2]);
  const float max_v = olx_max(olx_abs(minVal), olx_abs(maxVal));
  if ((RenderMode&planeRenderModePlane) != 0) {
    for (size_t i = 0; i < MaxDim; i++) {
      for (size_t j = 0; j < MaxDim; j++) {
        const size_t off = (i + j*MaxDim) * 3;
        vec3i cl;
        if (ContourData[i][j] > max_v) {
          cl = CalculateColour(1.0, colour_count, colours, ReverseColors);
        }
        else {
          cl = CalculateColour(ContourData[i][j] / max_v, colour_count, colours, ReverseColors);
        }
        for (int ci = 0; ci < 3; ci++) {
          TextData[off + ci] = cl[ci];
        }
      }
    }
    if (!olx_is_valid_index(TextIndex)) {
      TextIndex = Parent.GetTextureManager().Add2DTexture("Plane", 0,
        (GLsizei)MaxDim, (GLsizei)MaxDim, 0, GL_RGB, TextData);
      TGlTexture* tex = Parent.GetTextureManager().FindTexture(TextIndex);
      tex->SetEnvMode(tpeDecal);
      tex->SetSCrdWrapping(tpCrdClamp);
      tex->SetTCrdWrapping(tpCrdClamp);

      tex->SetMinFilter(tpFilterLinear);
      tex->SetMagFilter(tpFilterLinear);
      tex->SetEnabled(true);
    }
    else {
      Parent.GetTextureManager().
        Replace2DTexture(*Parent.GetTextureManager().
          FindTexture(TextIndex), 0, (GLsizei)MaxDim, (GLsizei)MaxDim, 0, GL_RGB,
          TextData);
    }
    GlP.SetTextureId(TextIndex);
  }
  if ((RenderMode&planeRenderModeContour) != 0) {
    Contour<float> cm;
    Contour<float>::MemberFeedback<TXGrid> mf(*this, &TXGrid::GlLine);
    float contour_step = (maxVal - minVal) / ContourLevelCount;
    ContourLevels[0] = minVal;
    for (size_t i = 1; i < ContourLevelCount; i++) {
      ContourLevels[i] = ContourLevels[i - 1] + contour_step;
    }
    TGlOption c_cl = 0;
    TGlPrimitive* mp = GetPrimitives().FindPrimitiveByName("eMap");
    if (mp != 0) {
      c_cl = mp->GetProperties().AmbientF;
    }
    GlP.PrepareColorRendering(GL_LINES);
    olx_gl::color(c_cl.Data());
    cm.DoContour(ContourData.data, 0, (int)MaxDim - 1, 0, (int)MaxDim - 1,
      ContourCrds[0], ContourCrds[1],
      ContourLevelCount, ContourLevels, mf);
    GlP.EndColorRendering();
  }
  if ((RenderMode&planeRenderModePlane) != 0) {
    float legend_step = (maxVal - minVal) / 32;
    for (int i = 0; i < 32; i++) {
      float val = minVal + legend_step*i;
      vec3i cl = CalculateColour(val / max_v, colour_count, colours, ReverseColors);
      size_t off = i * 32 * 3;
      for (int j = 0; j < 32 * 3; j += 3) {
        for (int ci = 0; ci < 3; ci++) {
          LegendData[off + j + ci] = cl[ci];
        }
      }
    }
    Legend->SetData((unsigned char*)LegendData, 32, 32, GL_RGB);
    legend_step = (maxVal - minVal) / ContourLevelCount;
    Legend->text.Clear();
    for (int i = 0; i < (int)ContourLevelCount; i++) {
      double v = minVal + legend_step*i;
      int av = (int)olx_abs(v);
      int positions = 3;
      while (av >= 10) {
        if(--positions == 1) {
          break;
        }
        av /= 10;
      }
      Legend->text << olxstr::FormatFloat(-positions, v);
    }
    Legend->Fit();
    UpdateInfo();
  }
  return false;
}
//.............................................................................
void TXGrid::GlLine(float x1, float y1, float x2, float y2, float z) {
  vec3d p1(x1 / Size, y1 / Size, Depth), p2(x2 / Size, y2 / Size, Depth);
  p1 = Parent.GetBasis().GetMatrix()*p1 - Parent.GetBasis().GetCenter();
  p2 = Parent.GetBasis().GetMatrix()*p2 - Parent.GetBasis().GetCenter();
  if (z < 0)  // render just a half of the segment
    p2 = (p1 + p2)*0.5;

  olx_gl::vertex(p1);
  olx_gl::vertex(p2);
}
//.............................................................................
bool TXGrid::GetDimensions(vec3d &Max, vec3d &Min)  {
//  Min = FCenter;
//  Max = FCenter;
  return false;
};
//.............................................................................
void TXGrid::InitGrid(size_t maxX, size_t maxY, size_t maxZ, bool use_colors) {
  DeleteObjects();
  _ResetLists();
  MaxX = maxX;
  MaxY = maxY;
  MaxZ = maxZ;
  MaxVal = MinVal = 0;
  olx_del_obj(ED);
  olx_del_obj(ColorData);
  ED = new TArray3D<float>(0, MaxX, 0, MaxY, 0, MaxZ);
  if (use_colors) {
    ColorData = new TArray3D<int>(0, MaxX, 0, MaxY, 0, MaxZ);
  }
  else {
    ColorData = 0;
  }
  TextData = new char[MaxDim*MaxDim * 3 + 1];
  ContourData.resize(MaxDim, MaxDim, false);
  ContourCrds[0] = new float[MaxDim];
  ContourCrds[1] = new float[MaxDim];
  float step = (MaxDim + 1)/(float)MaxDim;
  for (int i = 0; i < (int)MaxDim; i++) {
    ContourCrds[0][i] = ContourCrds[1][i] = step*i - (float)MaxDim / 2;
  }
  ContourLevels = new float[ContourLevelCount];
  MaxHole = MinHole = 0;
}
//.............................................................................
void TXGrid::DeleteObjects() {
  if (ED != 0) {
    delete ED;
    MaxX = MaxY = MaxZ = 0;
    ED = 0;
  }
  if (ColorData != 0) {
    delete ColorData;
    ColorData = 0;
  }
  if (TextData != 0) {
    delete TextData;
    TextData = 0;
  }
  triangles.Clear();
  normals.Clear();
  vertices.Clear();
  colors.Clear();
  if (Mask != 0) {
    delete Mask;
    Mask = 0;
  }
  ContourData.clear();
  if (ContourLevels != 0) {
    delete[] ContourCrds[0];
    delete[] ContourCrds[1];
    delete[] ContourLevels;
    ContourLevels = 0;
  }
}
//.............................................................................
bool TXGrid::LoadFromFile(const olxstr& GridFile) {
  TEFile::CheckFileExists(__OlxSourceInfo, GridFile);
  TCStrList SL = TEFile::ReadCLines(GridFile),
    toks;
  toks.Strtok(SL[0], ' ');

  int vc = 3;
  InitGrid(toks[0].ToSizeT(), toks[1].ToSizeT(), toks[2].ToSizeT());
  for (size_t i = 0; i < MaxX; i++) {
    for (size_t j = 0; j < MaxY; j++) {
      for (size_t k = 0; k < MaxZ; k++) {
        const float val = toks[vc].ToFloat();
        if (val > MaxVal) {
          MaxVal = val;
        }
        if (val < MinVal) {
          MinVal = val;
        }
        ED->Data[i][j][k] = val;
        vc++;
      }
    }
  }

  // set default depth to center of the asymmetric unit
  vec3d v(XApp->XFile().GetAsymmUnit().GetOCenter(true, true));
  v = XApp->XFile().GetAsymmUnit().GetCellToCartesian() * v;
  SetDepth(v);
  return true;
}
//.............................................................................
void TXGrid::SetScale(float v) {
  const bool _3d = Is3D();
  if (!Loading_) {
    Boxed = false;
  }
  if (_3d && MinHole != MaxHole) {
    if (v >= MinHole && v <= MaxHole) {
      Info->Clear();
      Info->PostText("Locked");
      Info->Fit();
      return;
    }
  }
  Scale = v;
  UpdateInfo();
  if (_3d && ED != 0) {
    triangles.Clear();
    normals.Clear();
    vertices.Clear();
    colors.Clear();
    if (XApp->Get3DFrame().IsVisible() || Boxed) {
      double SZ = olx_round(
        (double)MaxX / XApp->XFile().GetAsymmUnit().GetAxes()[0]);
      const vec3i isz = (XApp->Get3DFrame().GetSize()*SZ).Round<int>();
      TArray3D<float>& points = *(new TArray3D<float>(vec3i(0, 0, 0), isz));
      const mat3f rm(XApp->Get3DFrame().GetNormals() / SZ);
      const vec3f tr = XApp->Get3DFrame().GetEdge(0);
      const smatdd g2c(XApp->Get3DFrame().GetNormals() / SZ,
        XApp->Get3DFrame().GetEdge(0));
      const mat3d c2c = XApp->XFile().GetAsymmUnit().GetCartesianToCell();
      MapUtil::Cell2Cart(MapUtil::MapGetter<float, 2>(ED->Data),
        points.Data, g2c, c2c);
      CIsoSurface IS(points.Data, ColorData != 0 ? &ColorData->Data : 0);
      IS.GenerateSurface(Scale);
      vertices.Add(0).TakeOver(IS.VertexList());
      normals.Add(0).TakeOver(IS.NormalList());
      triangles.Add(0).TakeOver(IS.TriangleList());
      if (ColorData != 0) {
        colors.Add(0).TakeOver(IS.GetVertexData());
      }
      else if (Scale < 0) {
        IS.GenerateSurface(-Scale);
        vertices.Add(0).TakeOver(IS.VertexList());
        normals.Add(0).TakeOver(IS.NormalList());
        triangles.Add(0).TakeOver(IS.TriangleList());
      }
      for (size_t li = 0; li < vertices.Count(); li++) {
        for (size_t i = 0; i < vertices[li].Count(); i++) {
          vertices[li][i] = vertices[li][i] * rm + tr;
        }
        for (size_t i = 0; i < normals[li].Count(); i++) {
          normals[li][i] = normals[li][i] * XApp->Get3DFrame().GetNormals();
        }
      }
      delete &points;
      Boxed = true;
    }
    else {
      CIsoSurface IS(ED->Data, ColorData != 0 ? &ColorData->Data : 0);
      IS.GenerateSurface(Scale);
      vertices.Add(0).TakeOver(IS.VertexList());
      normals.Add(0).TakeOver(IS.NormalList());
      triangles.Add(0).TakeOver(IS.TriangleList());
      if (ColorData != 0) {
        colors.Add(0).TakeOver(IS.GetVertexData());
      }
      else if (Scale < 0) {
        IS.GenerateSurface(-Scale);
        vertices.Add(0).TakeOver(IS.VertexList());
        normals.Add(0).TakeOver(IS.NormalList());
        triangles.Add(0).TakeOver(IS.TriangleList());
      }
    }
    RescaleSurface(false);
  }
}
//.............................................................................
void TXGrid::SetExtended(bool v) {
  if (Extended == v) {
    return;
  }
  Extended = v;
  SetScale(Scale);
}
//.............................................................................
void TXGrid::SetDepth(float v) {
  Depth = v;
  const float max_z = (float)Parent.MaxDim().Length();
  const float min_z = (float)Parent.MinDim().Length();
  if (Depth < -min_z) {
    Depth = -min_z;
  }
  if (Depth > max_z) {
    Depth = max_z;
  }
  UpdateInfo();
}
//.............................................................................
void TXGrid::SetDepth(const vec3d& v) {
  vec3d p = (v + Parent.GetBasis().GetCenter())*Parent.GetBasis().GetMatrix();
  SetDepth((float)p[2]);
}
//.............................................................................
void TXGrid::SetPlaneSize(size_t _v) {
  size_t v = _v;
  while ((v & 1) == 0) {
    v = v >> 1;
  }
  if (v != 1) {
    throw TInvalidArgumentException(__OlxSrcInfo,
      "PlaneSize must be a power of 2");
  }
  if (_v < 64 || _v > 1024 || _v == MaxDim) {
    return;
  }
  if (TextData != 0) {
    delete TextData;
    TextData = new char[_v*_v * 3 + 1];
  }
  ContourData.resize(_v, _v, false);
  if (ContourLevels != 0) {
    delete[] ContourCrds[0];
    delete[] ContourCrds[1];
    ContourCrds[0] = new float[_v];
    ContourCrds[1] = new float[_v];
    const float hh = (float)_v / 2;
    for (size_t i = 0; i < _v; i++) {
      ContourCrds[0][i] = ContourCrds[1][i] = (float)i - hh;
    }
  }
  MaxDim = _v;
  Parent.Draw();
}
//.............................................................................
void TXGrid::SetContourLevelCount(size_t v) {
  if (v <= 2 || v > 30) {
    return;
  }
  if (ContourLevels != 0) {
    delete[] ContourLevels;
  }
  ContourLevelCount = v;
  ContourLevels = new float[ContourLevelCount];
}
//.............................................................................
bool TXGrid::OnMouseDown(const IOlxObject *Sender, const TMouseData& Data) {
  if ((Data.Shift & sssCtrl) == 0 && (Data.Shift & sssShift) == 0) {
    return false;
  }
  LastMouseX = Data.DownX;
  LastMouseY = Data.DownY;
  MouseDown = true;
  return true;
}
//.............................................................................
bool TXGrid::OnMouseUp(const IOlxObject *Sender, const TMouseData& Data) {
  MouseDown = false;
  return !((Data.Shift & sssCtrl) == 0 && (Data.Shift & sssShift) == 0);
}
//.............................................................................
bool TXGrid::OnMouseMove(const IOlxObject *Sender, const TMouseData& Data) {
  if (!MouseDown) {
    return false;
  }
  if ((Data.Button & smbLeft) != 0) {
    SetDepth(Depth + (float)((LastMouseX - Data.X) + (LastMouseY - Data.Y)) / 15);
  }
  else {
    if ((Data.Shift & sssShift) != 0) {
      if (RenderMode == planeRenderModeContour) {
        const int v = -(LastMouseX - Data.X) + (LastMouseY - Data.Y);
        SetContourLevelCount(GetContourLevelCount() + v / 2);
      }
      else {
        const float step = (MaxVal - MinVal) / 250.0f;
        Scale -= step*(LastMouseX - Data.X);
        Scale += step*(LastMouseY - Data.Y);
        if (olx_abs(Scale) > olx_max(MaxVal, MinVal))
          Scale = olx_sign(Scale)*olx_max(MaxVal, MinVal);
      }
    }
    else {
      Size += (float)(LastMouseX - Data.X) / 15;
      Size += (float)(LastMouseY - Data.Y) / 15;
      if (Size < 1)  Size = 1;
      if (Size > 20)  Size = 20;
    }
  }
  SetScale(Scale);
  UpdateInfo();
  LastMouseX = Data.X;
  LastMouseY = Data.Y;
  return true;
}
//.............................................................................
void TXGrid::UpdateInfo()  {
  Info->Clear();
  if (Is3D()) {
    Info->PostText(olxstr("Current level: ") <<
      olx_abs(olx_round(Scale, 1000)));
  }
  else {
    const vec3f center(Parent.GetBasis().GetCenter());
    vec3d p(0, 0, GetDepth());
    p = TXApp::GetInstance().XFile().GetAsymmUnit().Fractionalise(
      Parent.GetBasis().GetMatrix()*p - center);
    olxstr Tmp =  "Plane center: ";
    Tmp << olxstr::FormatFloat(3, p[0]) << "*a, " <<
      olxstr::FormatFloat(3, p[1]) << "*b, " <<
      olxstr::FormatFloat(3, p[2]) << "*c";
    Info->PostText(Tmp);
  }
  Info->Fit();
}
//.............................................................................
const_strlist TXGrid::ToPov(olx_cdict<TGlMaterial, olxstr> &materials) const {
  TGraphicsStyle &style = GetPrimitives().GetStyle();
  const olxstr p_mat_name = pov::get_mat_name("-Surface", style, materials),
    n_mat_name = pov::get_mat_name("+Surface", style, materials);
  TStrList out;
  pov::CrdTransformer crdc(Parent.GetBasis());
  out.Add(" object {");
  out.Add("  union {");
  const TAsymmUnit& au = XApp->XFile().GetAsymmUnit();
  if (Boxed) {
    for (size_t li = 0; li < triangles.Count(); li++) {
      const TArrayList<IsoTriangle>& trians = triangles[li];
      if (trians.IsEmpty()) {
        continue;
      }
      out.Add("  object { mesh {");
      const TArrayList<vec3f>& verts = vertices[li];
      const TArrayList<vec3f>& norms = normals[li];
      for (size_t i = 0; i < trians.Count(); i++) {
        out.Add("    smooth_triangle {");
        for (int j = 0; j < 3; j++) {
          out.Add("    ") << pov::to_str(crdc.crd(verts[trians[i][j]]))
            << pov::to_str(crdc.normal(norms[trians[i][j]]));
        }
        out.Add("    }");
      }
      out.Add("  }");
      out.Add("  texture {") << (li == 0 ? p_mat_name : n_mat_name) << "}}";
    }
  }
  else if (Mask != 0) {
    vec3d pts[3];
    for (size_t li = 0; li < triangles.Count(); li++) {
      const TArrayList<IsoTriangle>& trians = triangles[li];
      if (trians.IsEmpty()) {
        continue;
      }
      out.Add("  object { mesh {");
      const TArrayList<vec3f>& verts = vertices[li];
      const TArrayList<vec3f>& norms = normals[li];
      for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
          for (int z = -1; z <= 1; z++) {
            for (size_t i = 0; i < trians.Count(); i++) {
              bool draw = true;
              for (int j = 0; j < 3; j++) {
                pts[j] = verts[trians[i][j]];
                pts[j][0] = pts[j][0] / MaxX + x;
                pts[j][1] = pts[j][1] / MaxY + y;
                pts[j][2] = pts[j][2] / MaxZ + z;
                if (!Mask->Get(pts[j])) {
                  draw = false;
                  break;
                }
                au.CellToCartesian(pts[j]);
              }
              if (!draw)  continue;
              out.Add("    smooth_triangle {");
              for (int j = 0; j < 3; j++) {
                out.Add("    ") << pov::to_str(crdc.crd(pts[j]))
                  << pov::to_str(crdc.normal(norms[trians[i][j]]));
              }
              out.Add("    }");
            }
          }
        }
      }
      out.Add("  }");
      out.Add("  texture {") << (li == 0 ? p_mat_name : n_mat_name) << "}}";
    }
  }
  else {
    if (Extended) {
      vec3d pts[3]; // ext drawing
      for (size_t li = 0; li < triangles.Count(); li++) {
        const TArrayList<IsoTriangle>& trians = triangles[li];
        if (trians.IsEmpty()) {
          continue;
        }
        out.Add("  object { mesh {");
        const TArrayList<vec3f>& verts = vertices[li];
        const TArrayList<vec3f>& norms = normals[li];
        for (float x = ExtMin[0]; x < ExtMax[0]; x++) {
          for (float y = ExtMin[1]; y < ExtMax[1]; y++) {
            for (float z = ExtMin[2]; z < ExtMax[2]; z++) {
              for (size_t i = 0; i < trians.Count(); i++) {
                bool draw = true;
                for (int j = 0; j < 3; j++) {
                  pts[j] = verts[trians[i].pointID[j]];
                  pts[j][0] = pts[j][0] / MaxX + x;
                  pts[j][1] = pts[j][1] / MaxY + y;
                  pts[j][2] = pts[j][2] / MaxZ + z;
                  if (pts[j][0] > ExtMax[0] || pts[j][1] > ExtMax[1] ||
                    pts[j][2] > ExtMax[2])
                  {
                    draw = false;
                    break;
                  }
                  au.CellToCartesian(pts[j]);
                }
                if (!draw) {
                  continue;
                }
                out.Add("    smooth_triangle {");
                for (int j = 0; j < 3; j++) {
                  out.Add("    ") << pov::to_str(crdc.crd(pts[j]))
                    << pov::to_str(crdc.normal(norms[trians[i].pointID[j]]));
                }
                out.Add("    }");
              }
            }
          }
        }
        out.Add("  }");
        out.Add("  texture {") << (li == 0 ? p_mat_name : n_mat_name) << "}}";
      }
    }
    else {
      for (size_t li = 0; li < triangles.Count(); li++) {
        const TArrayList<IsoTriangle>& trians = triangles[li];
        if (trians.IsEmpty()) {
          continue;
        }
        out.Add("  object { mesh {");
        const TArrayList<vec3f>& verts = vertices[li];
        const TArrayList<vec3f>& norms = normals[li];
        for (size_t i = 0; i < verts.Count(); i++) {
          verts[i][0] /= MaxX;  verts[i][1] /= MaxY;  verts[i][2] /= MaxZ;
          au.CellToCartesian(verts[i]);
        }
        for (size_t i = 0; i < trians.Count(); i++) {
          out.Add("    smooth_triangle {");
          for (int j = 0; j < 3; j++) {
            out.Add("    ") << pov::to_str(crdc.crd(verts[trians[i].pointID[j]]))
              << pov::to_str(crdc.normal(norms[trians[i].pointID[j]]));
          }
          out.Add("    }");
        }
        out.Add("  }");
        out.Add("  texture {") << (li == 0 ? p_mat_name : n_mat_name) << "}}";
      }
    }
  }
  out.Add(" }}");
  return out;
}
//.............................................................................
void TXGrid::RescaleSurface(bool collect_only) {
  if (collect_only) {
    if (!cp_vertices.ok() || !cn_vertices.ok()) {
      return;
    }
  }
  const TAsymmUnit& au = XApp->XFile().GetAsymmUnit();
  if (!collect_only && !olx_is_valid_index(PListId)) {
    PListId = Parent.NewListId();
    NListId = Parent.NewListId();
  }
  if (Boxed) {
    bool sphere = XApp->Get3DFrame().IsVisible() &&
      XApp->Get3DFrame().IsSpherical();
    float qr = (float)olx_sqr(XApp->Get3DFrame().GetZoom());
    vec3f center = XApp->Get3DFrame().GetCenter();
    for (size_t li = 0; li < triangles.Count(); li++) {
      const TArrayList<vec3f>& verts = vertices[li];
      const TArrayList<vec3f>& norms = normals[li];
      const TArrayList<IsoTriangle>& trians = triangles[li];
      TTypeList<vec3f> *va = (li == 0 ? &cp_vertices : &cn_vertices);
      if (!collect_only) {
        olx_gl::newList(li == 0 ? PListId : NListId, GL_COMPILE);
        olx_gl::polygonMode(GL_FRONT_AND_BACK, GetPolygonMode());
        olx_gl::begin(GL_TRIANGLES);
      }
      if (sphere) {
        for (size_t i = 0; i < trians.Count(); i++) {
          bool draw = true;
          for (int j = 0; j < 3; j++) {
            if ((verts[trians[i][j]] - center).QLength() > qr) {
              draw = false;
              break;
            }
          }
          if (!draw) {
            continue;
          }
          if (collect_only) {
            for (int j = 0; j < 3; j++) {
              va->AddCopy(verts[trians[i][j]]);
            }
          }
          else {
            for (int j = 0; j < 3; j++) {
              olx_gl::normal(norms[trians[i][j]]);
              olx_gl::vertex(verts[trians[i][j]]);
              if (ColorData != 0) {
                olx_gl::color(colors[li][trians[i][j]]);
              }
            }
          }
        }
      }
      else {
        for (size_t i = 0; i < trians.Count(); i++) {
          for (int j = 0; j < 3; j++) {
            olx_gl::normal(norms[trians[i][j]]);
            olx_gl::vertex(verts[trians[i][j]]);
            if (ColorData != 0) {
              olx_gl::color(colors[li][trians[i][j]]);
            }
          }
        }
      }
      if (!collect_only) {
        olx_gl::end();
        olx_gl::polygonMode(GL_FRONT_AND_BACK, GL_FILL);
        olx_gl::endList();
      }
    }
  }
  else if (Mask != 0) {
    vec3d pts[3];
    for (size_t li = 0; li < triangles.Count(); li++) {
      const TArrayList<vec3f>& verts = vertices[li];
      const TArrayList<vec3f>& norms = normals[li];
      const TArrayList<IsoTriangle>& trians = triangles[li];
      TTypeList<vec3f> *va = (li == 0 ? &cp_vertices : &cn_vertices);
      if (!collect_only) {
        olx_gl::newList(li == 0 ? PListId : NListId, GL_COMPILE);
        olx_gl::polygonMode(GL_FRONT_AND_BACK, GetPolygonMode());
        olx_gl::begin(GL_TRIANGLES);
      }
      for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
          for (int z = -1; z <= 1; z++) {
            for (size_t i = 0; i < trians.Count(); i++) {
              bool draw = true;
              for (int j = 0; j < 3; j++) {
                pts[j] = verts[trians[i][j]];
                pts[j][0] = pts[j][0] / MaxX + x;
                pts[j][1] = pts[j][1] / MaxY + y;
                pts[j][2] = pts[j][2] / MaxZ + z;
                if (!Mask->Get(pts[j])) {
                  draw = false;
                  break;
                }
                au.CellToCartesian(pts[j]);
              }
              if (!draw) {
                continue;
              }
              if (collect_only) {
                for (int j = 0; j < 3; j++) {
                  va->AddCopy(pts[j]);
                }
              }
              else {
                for (int j = 0; j < 3; j++) {
                  olx_gl::normal(norms[trians[i][j]]);
                  olx_gl::vertex(pts[j]);
                  if (ColorData != 0) {
                    olx_gl::color(colors[li][trians[i][j]]);
                  }
                }
              }
            }
          }
        }
      }
      if (!collect_only) {
        olx_gl::end();
        olx_gl::polygonMode(GL_FRONT_AND_BACK, GL_FILL);
        olx_gl::endList();
      }
    }
  }
  else {
    if (Extended) {
      vec3d pts[3]; // ext drawing
      for (size_t li = 0; li < triangles.Count(); li++) {
        const TArrayList<vec3f>& verts = vertices[li];
        const TArrayList<vec3f>& norms = normals[li];
        const TArrayList<IsoTriangle>& trians = triangles[li];
        TTypeList<vec3f> *va = (li == 0 ? &cp_vertices : &cn_vertices);
        if (!collect_only) {
          olx_gl::newList(li == 0 ? PListId : NListId, GL_COMPILE);
          olx_gl::polygonMode(GL_FRONT_AND_BACK, GetPolygonMode());
          olx_gl::begin(GL_TRIANGLES);
        }
        for (float x = ExtMin[0]; x < ExtMax[0]; x++) {
          for (float y = ExtMin[1]; y < ExtMax[1]; y++) {
            for (float z = ExtMin[2]; z < ExtMax[2]; z++) {
              for (size_t i = 0; i < trians.Count(); i++) {
                bool draw = true;
                for (int j = 0; j < 3; j++) {
                  pts[j] = verts[trians[i][j]];
                  pts[j][0] = pts[j][0] / MaxX + x;
                  pts[j][1] = pts[j][1] / MaxY + y;
                  pts[j][2] = pts[j][2] / MaxZ + z;
                  if (pts[j][0] > ExtMax[0] || pts[j][1] > ExtMax[1] ||
                    pts[j][2] > ExtMax[2])
                  {
                    draw = false;
                    break;
                  }
                  au.CellToCartesian(pts[j]);
                }
                if (!draw) {
                  continue;
                }
                if (collect_only) {
                  for (int j = 0; j < 3; j++) {
                    va->AddCopy(pts[j]);
                  }
                }
                else {
                  for (int j = 0; j < 3; j++) {
                    olx_gl::normal(norms[trians[i].pointID[j]]);
                    olx_gl::vertex(pts[j]);
                    if (ColorData != 0) {
                      olx_gl::color(colors[li][trians[i][j]]);
                    }
                  }
                }
              }
            }
          }
        }
        if (!collect_only) {
          olx_gl::end();
          olx_gl::polygonMode(GL_FRONT_AND_BACK, GL_FILL);
          olx_gl::endList();
        }
      }
    }
    else {
      vec3d pts[3];
      for (size_t li = 0; li < triangles.Count(); li++) {
        const TArrayList<vec3f>& verts = vertices[li];
        const TArrayList<vec3f>& norms = normals[li];
        const TArrayList<IsoTriangle>& trians = triangles[li];
        TTypeList<vec3f> *va = (li == 0 ? &cp_vertices : &cn_vertices);
        if (!collect_only) {
          olx_gl::newList(li == 0 ? PListId : NListId, GL_COMPILE);
          olx_gl::polygonMode(GL_FRONT_AND_BACK, GetPolygonMode());
          olx_gl::begin(GL_TRIANGLES);
        }
        for (size_t i = 0; i < trians.Count(); i++) {
          for (int j = 0; j < 3; j++) {
            pts[j] = verts[trians[i][j]];
            pts[j][0] = pts[j][0] / MaxX;
            pts[j][1] = pts[j][1] / MaxY;
            pts[j][2] = pts[j][2] / MaxZ;
            au.CellToCartesian(pts[j]);
          }
          if (collect_only) {
            for (int j = 0; j < 3; j++) {
              va->AddCopy(pts[j]);
            }
          }
          else {
            for (int j = 0; j < 3; j++) {
              olx_gl::normal(norms[trians[i][j]]);
              olx_gl::vertex(pts[j]);  // cell drawing
              if (ColorData != 0) {
                olx_gl::color(colors[li][trians[i][j]]);
              }
            }
          }
        }
        if (!collect_only) {
          olx_gl::end();
          olx_gl::polygonMode(GL_FRONT_AND_BACK, GL_FILL);
          olx_gl::endList();
        }
      }
    }
  }
}
//.............................................................................
void TXGrid::AdjustMap() {
  if (ED == 0) {
    return;
  }
  for (size_t i = 0; i < MaxX; i++) {
    for (size_t j = 0; j < MaxY; j++) {
      ED->Data[i][j][MaxZ] = ED->Data[i][j][0];
    }
  }
  for (size_t i = 0; i < MaxX; i++) {
    for (size_t j = 0; j < MaxZ; j++) {
      ED->Data[i][MaxY][j] = ED->Data[i][0][j];
    }
  }
  for (size_t i = 0; i < MaxY; i++) {
    for (size_t j = 0; j < MaxZ; j++) {
      ED->Data[MaxX][i][j] = ED->Data[0][i][j];
    }
  }

  for (size_t i = 0; i < MaxX; i++) {
    ED->Data[i][MaxY][MaxZ] = ED->Data[i][0][0];
  }
  for (size_t i = 0; i < MaxY; i++) {
    ED->Data[MaxX][i][MaxZ] = ED->Data[0][i][0];
  }
  for (size_t i = 0; i < MaxZ; i++) {
    ED->Data[MaxX][MaxY][i] = ED->Data[0][0][i];
  }
  ED->Data[MaxX][MaxY][MaxZ] = ED->Data[0][0][0];
}
//.............................................................................
void TXGrid::InitIso() {
  if (Is3D() && ED != 0) {
    SetScale(Scale);
    Legend->text.Clear();
  }
}
//.............................................................................
TPtrList<TXBlob>::const_list_type TXGrid::CreateBlobs(int flags) {
  TPtrList<TXBlob> rv;
  if (!Is3D()) {
    return rv;
  }
  size_t t_cnt = 0;
  for (size_t li = 0; li < triangles.Count(); li++) {
    t_cnt += triangles[li].Count();
  }
  if (t_cnt < 25) {
    return rv;
  }
  cp_vertices = new TTypeList<vec3f>();
  cn_vertices = new TTypeList<vec3f>();
  RescaleSurface(true);
  // compact the data
  for (int li = 0; li <= 1; li++) {
    TTypeList<vec3f> &va = (li == 0 ? cp_vertices : cn_vertices);
    olxset<vec3f, TComparableComparator> vset;
    vset.AddAll(va);
    TTypeList<IsoTriangle> triangles;
    triangles.SetCapacity(va.Count() / 3);
    for (size_t i = 0; i < va.Count(); i+=3) {
      IsoTriangle &t = triangles.AddNew();
      t[0] = vset.IndexOf(va[i]);
      t[1] = vset.IndexOf(va[i+1]);
      t[2] = vset.IndexOf(va[i+2]);
    }
    TArrayList<size_t> verts(vset.Count(),
      olx_list_init::value(InvalidIndex));
    TEBitArray used_triags(triangles.Count());
    // map vertices to triangles
    TArrayList<olx_pset<size_t> > v2t(vset.Count());
    for (size_t i = 0; i < triangles.Count(); i++) {
      for (size_t j = 0; j < 3; j++) {
        v2t[triangles[i][j]].Add(i);
      }
    }
    TArrayList<size_t> new_ids(vset.Count());
    for (size_t i = 0; i < verts.Count(); i++) {
      if (verts[i] == InvalidIndex) {
        verts[i] = rv.Count();
        TXBlob &blob = *rv.Add(new TXBlob(Parent, li ? "blob+" : "blob-"));
        TArrayList<size_t> bt;
        size_t cnt = 1;
        olx_pset<size_t> st = v2t[i];
        while (!st.IsEmpty()) {
          size_t ti = st[st.Count() - 1];
          st.Delete(st.Count() - 1);
          bt.Add(ti);
          const IsoTriangle &t = triangles[ti];
          if (used_triags[ti]) {
            continue;
          }
          used_triags.SetTrue(ti);
          for (size_t j = 0; j < 3; j++) {
            if (verts[t[j]] == InvalidIndex) {
              cnt++;
              verts[t[j]] = verts[i];
              for (size_t k = 0; k < v2t[t[j]].Count(); k++) {
                if (!used_triags[v2t[t[j]][k]]) {
                  st.Add(v2t[t[j]][k]);
                }
              }
            }
          }
        }
        if (cnt < 25) {
          delete rv.GetLast();
          rv[rv.Count() - 1] = 0;
          continue;
        }
        blob.vertices.SetCapacity(cnt);
        blob.normals.SetCapacity(cnt);
        blob.SetSelectable(true);
        for (size_t j = i; j < verts.Count(); j++) {
          if (verts[j] == verts[i]) {
            new_ids[j] = blob.vertices.Count();
            blob.vertices.AddCopy(vset[j]);
            vec3f n;
            for (size_t k = 0; k < v2t[j].Count(); k++) {
              IsoTriangle& t = triangles[v2t[j][k]];
              n += (vset[t[1]] - vset[t[0]]).XProdVec(vset[t[2]] - vset[t[0]]);
            }
            float ql = n.QLength();
            if ( ql > 0) {
              blob.normals.AddCopy(n / sqrt(ql));
            }
            else {
              blob.normals.AddNew()[2] = 1;
            }
          }
          else {
            new_ids[j] = ~0;
          }
        }
        blob.triangles.SetCapacity(bt.Count());
        for (size_t i = 0; i < bt.Count(); i++) {
          IsoTriangle& t = blob.triangles.Add(new IsoTriangle(triangles[bt[i]]));
          for (size_t j = 0; j < 3; j++) {
            t[j] = (int)new_ids[t[j]];
          }
        }
      }
    }
  }
  cp_vertices = 0;
  cn_vertices = 0;
  rv.Pack();
  return rv;
}
//.............................................................................
//.............................................................................
//.............................................................................
void TXGrid::LibExtended(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    E.SetRetVal(Extended);
  }
  else if (Params.Count() == 1) {
    SetExtended(Params[0].ToBool());
  }
  else {
    ExtMin = vec3d(Params[0].ToDouble(), Params[1].ToDouble(), Params[2].ToDouble());
    ExtMax = vec3d(Params[3].ToDouble(), Params[4].ToDouble(), Params[5].ToDouble());
    SetExtended(true);
  }
}
//.............................................................................
void TXGrid::LibScale(const TStrObjList& Params, TMacroData& E)  {
  if (Params.IsEmpty()) {
    E.SetRetVal(Scale);
  }
  else {
    SetScale(Params[0].ToFloat());
  }
}
//.............................................................................
void TXGrid::LibSize(const TStrObjList& Params, TMacroData& E)  {
  if (Params.IsEmpty()) {
    E.SetRetVal(Size);
  }
  else {
    Size = Params[0].ToFloat();
  }
}
//.............................................................................
void TXGrid::LibPlaneSize(const TStrObjList& Params, TMacroData& E)  {
  if (Params.IsEmpty()) {
    E.SetRetVal(MaxDim);
  }
  else {
    SetPlaneSize(Params[0].ToSizeT());
  }
}
//.............................................................................
void TXGrid::LibDepth(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty())
    E.SetRetVal(Depth);
  else {
    if (Params.Count() == 1)
      Depth = Params[0].ToFloat();
    else if (Params.Count() == 3) {
      vec3d v(
        Params[0].ToDouble(),
        Params[1].ToDouble(),
        Params[2].ToDouble());
      const TAsymmUnit &au = TXApp::GetInstance().XFile().GetAsymmUnit();
      SetDepth(au.Orthogonalise(v));
    }
  }
}
//.............................................................................
void TXGrid::LibMaxDepth(const TStrObjList& Params, TMacroData& E) {
  E.SetRetVal(Parent.MaxDim().DistanceTo(Parent.MinDim()) / 2);
}
//.............................................................................
void TXGrid::LibContours(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    E.SetRetVal(ContourLevelCount);
  }
  else {
    SetContourLevelCount(Params[0].ToSizeT());
    XApp->Draw();
  }
}
//.............................................................................
void TXGrid::LibIsvalid(const TStrObjList& Params, TMacroData& E) {
  E.SetRetVal(ED != 0);
}
//.............................................................................
void TXGrid::LibGetMin(const TStrObjList& Params, TMacroData& E) {
  E.SetRetVal(MinVal);
}
//.............................................................................
void TXGrid::LibGetMax(const TStrObjList& Params, TMacroData& E) {
  E.SetRetVal(MaxVal);
}
//.............................................................................
void TXGrid::LibRenderMode(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    short rm = RenderMode & 0xFF;
    if (rm == planeRenderModeFill) {
      E.SetRetVal<olxstr>("fill");
    }
    else if (rm == planeRenderModePoint) {
      E.SetRetVal<olxstr>("point");
    }
    else if (rm == planeRenderModeLine) {
      E.SetRetVal<olxstr>("line");
    }
    else if (rm == planeRenderModePlane) {
      E.SetRetVal<olxstr>("plane");
    }
    else if (rm == planeRenderModeContour) {
      E.SetRetVal<olxstr>("contour");
    }
    else if (rm == (planeRenderModeContour | planeRenderModePlane)) {
      E.SetRetVal<olxstr>("contour+plane");
    }
    return;
  }
  if (Params[0] == "fill") {
    RenderMode = planeRenderModeFill;
  }
  else if (Params[0] == "point") {
    RenderMode = planeRenderModePoint;
  }
  else if (Params[0] == "line") {
    RenderMode = planeRenderModeLine;
  }
  else if (Params[0] == "plane") {
    RenderMode = planeRenderModePlane | (3 << 8);
    if (IsVisible()) {
      this->Legend->SetVisible(true);
    }
    if (Params.Count() == 2) {
      RenderMode = (RenderMode & 0xff) | (short)(Params[1].ToInt() << 8);
    }
  }
  else if (Params[0] == "contour") {
    RenderMode = planeRenderModeContour;
    if (IsVisible()) {
      this->Legend->SetVisible(false);
    }
  }
  else if (Params[0] == "contour+plane") {
    RenderMode = planeRenderModeContour | planeRenderModePlane | (3 << 8);
    if (IsVisible()) {
      this->Legend->SetVisible(true);
    }
    if (Params.Count() == 2) {
      RenderMode = (RenderMode&0xff)|(short)(Params[1].ToInt() << 8);
    }
  }
  else {
    throw TInvalidArgumentException(__OlxSourceInfo,
      olxstr("incorrect mode value: '") << Params[0] << '\'');
  }
  InitIso();
}
//.............................................................................
void TXGrid::ToDataItem(TDataItem& item, IOutputStream& zos) const {
  item.AddField("empty", IsEmpty());
  if (!IsEmpty()) {
    //item.AddField("visible", Visible());
    item.AddField("draw_mode", RenderMode);
    item.AddField("max_val", MaxVal);
    item.AddField("min_val", MinVal);
    item.AddField("depth", Depth);
    item.AddField("size", Size);
    item.AddField("extended", Extended);
    item.AddField("boxed", Boxed);
    item.AddField("ext_min", PersUtil::VecToStr(ExtMin));
    item.AddField("ext_max", PersUtil::VecToStr(ExtMin));
    item.AddField("scale", Scale);
    item.AddField("max_x", MaxX);
    item.AddField("max_y", MaxY);
    item.AddField("max_z", MaxZ);
    item.AddField("box_min", box_min);
    item.AddField("box_step", box_step);
    item.AddField("reverse_colors", ReverseColors);
    for (size_t x = 0; x < MaxX; x++) {
      for (size_t y = 0; y < MaxY; y++) {
        zos.Write(ED->Data[x][y], sizeof(float)*MaxZ);
      }
    }
    if (Mask != 0 && Mask->GetMask() != 0) {
      Mask->ToDataItem(item.AddItem("mask"), zos);
    }
  }
}
//.............................................................................
void TXGrid::FromDataItem(const TDataItem& item, IInputStream& zis) {
  Clear();
  Loading_ = true;
  bool empty = item.GetFieldByName("empty").ToBool();
  if (empty)  return;
  //Visible( item.GetFieldByName("visible").ToBool() );
  SetVisible(true);
  RenderMode = item.GetFieldByName("draw_mode").ToInt();
  Size = item.GetFieldByName("size").ToFloat();
  Extended = item.FindField("extended", FalseString()).ToBool();
  Boxed = item.FindField("boxed", FalseString()).ToBool();
  ExtMin = vec3f(-1, -1, -1);
  ExtMax = vec3f(1, 1, 1);
  const size_t ed_i = item.FieldIndex("ext_max");
  if (ed_i != InvalidIndex) {
    PersUtil::VecFromStr(item.GetFieldByIndex(ed_i), ExtMin);
    PersUtil::VecFromStr(item.GetFieldByName("ext_min"), ExtMax);
  }
  Scale = item.GetFieldByName("scale").ToFloat();
  InitGrid(item.GetFieldByName("max_x").ToInt(),
    item.GetFieldByName("max_y").ToInt(),
    item.GetFieldByName("max_z").ToInt());
  MaxVal = item.GetFieldByName("max_val").ToFloat();
  MinVal = item.GetFieldByName("min_val").ToFloat();
  Depth = item.GetFieldByName("depth").ToFloat();
  box_min = item.FindField("box_min", "0").ToFloat();
  box_step = item.FindField("box_step", "0").ToFloat();
  ReverseColors = item.FindField("reverse_colors", FalseString()).ToBool();
  for (size_t x = 0; x < MaxX; x++) {
    for (size_t y = 0; y < MaxY; y++) {
      zis.Read(ED->Data[x][y], sizeof(float)*MaxZ);
    }
  }
  TDataItem* maski = item.FindItem("mask");
  if (maski != 0) {
    Mask = new FractMask;
    Mask->FromDataItem(*maski, zis);
  }
  AdjustMap();
  InitIso();
  Loading_ = false;
}
//.............................................................................
void TXGrid::LibFix(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    E.SetRetVal<olxstr>(olxstr(box_min) << " at " << box_step);
  }
  else {
    box_min = Params[0].ToFloat();
    box_step = Params[1].ToFloat();
    XApp->Draw();
  }
}
//.............................................................................
void TXGrid::LibSplit(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TPtrList<TXBlob> blobs = CreateBlobs(2);
  TGXApp &gxapp = TGXApp::GetInstance();
  for (size_t i = 0; i < blobs.Count(); i++) {
    gxapp.AddObjectToCreate(blobs[i]);
    blobs[i]->Create();
  }
}
//.............................................................................
struct FloatVC {
  static int& eps() {
    static int eps_ = 75;
    return eps_;
  }
  FloatVC() {
  }
  int Compare(const vec3f& a, const vec3f& b) const {
    int e = eps();
    for (int i = 0; i < 3; i++) {
      int r = olx_cmp((int)(a[i] * e), (int)(b[i] * e));
      if (r != 0) {
        return r;
      }
    }
    return 0;
  }
};

void TXGrid::LibProcess(TStrObjList& Cmds, const TParamList& Options,
  TMacroData& Error)
{
  TGXApp& gxapp = TGXApp::GetInstance();
  const TAsymmUnit& au = gxapp.XFile().GetAsymmUnit();
  if (Cmds[0] == "smooth") {
    float ratio = Options.FindValue("r", "0.8").ToFloat();
    size_t N = Options.FindValue("n", "10").ToSizeT();
    int eps = Options.FindValue("m", "0").ToInt();
    for (size_t li = 0; li < triangles.Count(); li++) {
      if (eps > 0) {
        FloatVC::eps() = eps;
        olxset<vec3f, FloatVC> vset;
        vset.AddAll(vertices[li]);
        if (vset.Count() != vertices[li].Count()) {
          for (size_t i = 0; i < triangles[li].Count(); i++) {
            for (size_t j = 0; j < 3; j++) {
              triangles[li][i][j] = vset.IndexOf(vertices[li][triangles[li][i][j]]);
            }
          }
          vertices[li] = vset;
        }
      }
      TArrayList<vec3f>& norms = normals[li];
      TTypeList<IsoTriangle> triags = triangles[li];
      TTypeList<vec3f> verts(vertices[li].Count());
      if (!Boxed) {
        for (size_t i = 0; i < verts.Count(); i++) {
          verts[i] = vertices[li][i];
          verts[i][0] /= MaxX;
          verts[i][1] /= MaxY;
          verts[i][2] /= MaxZ;
          au.CellToCartesian(verts[i]);
        }
      }
      else {
        verts = vertices[li];
      }
      olx_grid_util::smoother sm(verts, triags);
      for (size_t i = 0; i < N; i++) {
        sm.smooth(ratio);
      }

      for (size_t i = 0; i < norms.Count(); i++) {
        norms[i].Null();
      }
      for (size_t i = 0; i < triags.Count(); i++) {
        const IsoTriangle& t = triags[i];
        vec3f n = (verts[t[1]] - verts[t[0]]).XProdVec(verts[t[2]] - verts[t[0]]);
        float ql = n.QLength();
        for (size_t j = 0; j < 3; j++) {
          if (ql > 0) {
            norms[t[j]] += (n / sqrt(ql));
          }
        }
      }
      for (size_t i = 0; i < norms.Count(); i++) {
        if (!norms[i].IsNull()) {
          norms[i].Normalise();
        }
        else {
          norms[i][2] = 1;
        }
      }
      if (!Boxed) {
        for (size_t i = 0; i < verts.Count(); i++) {
          vertices[li][i] = au.Fractionalise(verts[i]);
          vertices[li][i][0] *= MaxX;
          vertices[li][i][1] *= MaxY;
          vertices[li][i][2] *= MaxZ;
        }
      }
      else {
        vertices[li] = verts;
      }
    }
    RescaleSurface(false);
  }
}
//.............................................................................
void TXGrid::LibReverseColors(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    E.SetRetVal(ReverseColors);
  }
  else {
    ReverseColors = Params[0].ToBool();
  }
}
//.............................................................................
TLibrary* TXGrid::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("xgrid") : name);
  lib->Register(new TFunction<TXGrid>(this,
    &TXGrid::LibGetMin, "GetMin",
    fpNone, "Returns minimum value of the map"));
  lib->Register(new TFunction<TXGrid>(this,
    &TXGrid::LibGetMax, "GetMax",
    fpNone, "Returns maximum value of the map"));
  lib->Register(new TFunction<TXGrid>(this,
    &TXGrid::LibExtended, "Extended",
    fpNone | fpOne | fpSix, "Returns/sets extended size of the grid"));
  lib->Register(new TFunction<TXGrid>(this,
    &TXGrid::LibScale, "Scale",
    fpNone | fpOne, "Returns/sets current scale"));
  lib->Register(new TFunction<TXGrid>(this,
    &TXGrid::LibFix, "Fix",
    fpNone | fpTwo, "Sets the start and step for contours. Set step=0 to cancel."));
  lib->Register(new TFunction<TXGrid>(this,
    &TXGrid::LibSize, "Size",
    fpNone | fpOne, "Returns/sets current size"));
  lib->Register(new TFunction<TXGrid>(this,
    &TXGrid::LibPlaneSize, "PlaneSize",
    fpNone | fpOne, "Returns/sets current size"));
  lib->Register(new TFunction<TXGrid>(this,
    &TXGrid::LibDepth, "Depth",
    fpNone | fpOne | fpThree, "Returns/sets current depth"));
  lib->Register(new TFunction<TXGrid>(this,
    &TXGrid::LibDepth, "MaxDepth",
    fpNone, "Returns maximum available depth"));
  lib->Register(new TFunction<TXGrid>(this,
    &TXGrid::LibContours, "Contours",
    fpNone | fpOne, "Returns/sets number of contour levels"));
  lib->Register(new TFunction<TXGrid>(this,
    &TXGrid::LibIsvalid, "IsValid",
    fpNone | fpOne, "Returns true if grid data is initialised"));
  lib->Register(new TFunction<TXGrid>(this,
    &TXGrid::LibRenderMode, "RenderMode",
    fpNone | fpOne | fpTwo,
    "Returns/sets grid rendering mode. Supported values: point, line, fill, "
    "plane, contour. Second argument may specify the number of colours in the "
    "plane gradient (2,3,5,7)"));
  lib->Register(new TMacro<TXGrid>(this,
    &TXGrid::LibSplit, "Split",
    EmptyString(),
    fpNone | fpOne,
    "Split current grid view into individual blobs"));
  lib->Register(new TMacro<TXGrid>(this,
    &TXGrid::LibProcess, "Process",
    "r-ratio for smoothing [0.8]&;"
    "n-number of cycles [10]&;"
    "m-merge close vertices",
    fpAny ^ fpNone,
    "Executes a grid process. Currently only 'smooth' is available"));
  lib->Register(new TFunction<TXGrid>(this,
    &TXGrid::LibReverseColors, "ReverseColors",
    fpNone | fpOne, "Returns/sets reverse colors for plane rendering"));

  AGDrawObject::ExportLibrary(*lib);
  Info->ExportLibrary(*lib->AddLibrary("label"));
  Legend->ExportLibrary(*lib->AddLibrary("legend"));
  return lib;
}
//.............................................................................
//.............................................................................
//.............................................................................
#ifdef _PYTHON
PyObject* pyImport(PyObject* self, PyObject* args) {
  char* data;
  int dim1, dim2, dim3, focus1, focus2, focus3;
  int type;
  int len;
  olxcstr format = PythonExt::UpdateBinaryFormat("(iii)(iii)s#i");
  if (!PyArg_ParseTuple(args, format.c_str(),
    &dim1, &dim2, &dim3,
    &focus1, &focus2, &focus3, &data, &len, &type))
  {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo,
      format.c_str());
  }
  const size_t sz = dim1 * dim2*dim3;
  if ((type == 0 && sz * sizeof(double) != (size_t)len) ||
    (type == 1 && sz * sizeof(int) != (size_t)len))
  {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "array size");
  }
  TEMemoryInputStream ms(data, len);
  TXGrid& g = *TXGrid::GetInstance();
  g.InitGrid(focus1, focus2, focus3);
  for (int d1 = 0; d1 < focus1; d1++) {
    for (int d2 = 0; d2 < focus2; d2++)
      for (int d3 = 0; d3 < focus3; d3++) {
        float v;
        if (type == 0) {
          double _v;
          ms.SetPosition(((d1*dim2 + d2)*dim3 + d3) * sizeof(double));
          ms >> _v;
          v = (float)_v;
        }
        else if (type == 1) {
          int _v;
          ms.SetPosition(((d1*dim2 + d2)*dim3 + d3) * sizeof(int));
          ms >> _v;
          v = (float)_v;
        }
        g.SetValue(d1, d2, d3, v);
      }
  }
  return PythonExt::PyNone();
}
//.............................................................................
PyObject* pySetValue(PyObject* self, PyObject* args) {
  int i, j, k, cl;
  float val;
  if (!PyArg_ParseTuple(args, "iiif|i", &i, &j, &k, &val, &cl)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "iiif|i");
  }
  TXGrid::GetInstance()->SetValue(i, j, k, val, (uint32_t)cl);
  return PythonExt::PyNone();
}
//.............................................................................
PyObject *pyGetValue(PyObject *self, PyObject* args) {
  int i, j, k;
  if (!PyArg_ParseTuple(args, "iii", &i, &j, &k)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "iii");
  }
  TXGrid *g = TXGrid::GetInstance();
  if (g->Data() == 0 || !g->Data()->IsInRange(i, j, k)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo,
      "index - out of range");
  }
  return Py_BuildValue("f", g->GetValue(i, j, k));
}
//.............................................................................
PyObject* pyInit(PyObject* self, PyObject* args) {
  int i, j, k;
  PyObject *colors = 0;
  if (!PyArg_ParseTuple(args, "iii|O", &i, &j, &k, &colors)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "iii|i");
  }
  TXGrid::GetInstance()->InitGrid(i, j, k, colors == Py_True);
  return PythonExt::PyTrue();
}
//.............................................................................
PyObject *pyGetSize(PyObject *self, PyObject* args) {
  TXGrid *g = TXGrid::GetInstance();
  if (g->Data() == 0) {
    return Py_BuildValue("iii", 0, 0, 0);
  }
  return Py_BuildValue("iii",
    g->Data()->Length1(), g->Data()->Length2(), g->Data()->Length3());
}
//.............................................................................
PyObject* pySetMinMax(PyObject* self, PyObject* args) {
  float min, max;
  if (!PyArg_ParseTuple(args, "ff", &min, &max)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "ff");
  }
  TXGrid::GetInstance()->SetMinVal(min);
  TXGrid::GetInstance()->SetMaxVal(max);
  return PythonExt::PyTrue();
}
//.............................................................................
PyObject* pySetHole(PyObject* self, PyObject* args) {
  float min, max;
  if (!PyArg_ParseTuple(args, "ff", &min, &max)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "ff");
  }
  TXGrid::GetInstance()->SetMinHole(min);
  TXGrid::GetInstance()->SetMaxHole(max);
  return PythonExt::PyTrue();
}
//.............................................................................
PyObject* pySetVisible(PyObject* self, PyObject* args) {
  bool v;
  if (!PyArg_ParseTuple(args, "b", &v)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "b");
  }
  TXGrid::GetInstance()->SetVisible(v);
  return PythonExt::PyNone();
}
//.............................................................................
PyObject* pyInitSurface(PyObject* self, PyObject* args)  {
  bool v;
  float mask_inc = -100;
  if (!PyArg_ParseTuple(args, "b|f", &v, &mask_inc)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "b");
  }
  if (v) {
    TXGrid::GetInstance()->AdjustMap();
  }
  if (mask_inc != -100) {
    FractMask* fm = new FractMask;
    TGXApp &app = TGXApp::GetInstance();
    app.BuildSceneMask(*fm, mask_inc);
    TXGrid::GetInstance()->SetMask(*fm);

  }
  TXGrid::GetInstance()->InitIso();
  return PythonExt::PyNone();
}
//.............................................................................
PyObject* pySetSurfaceScale(PyObject* self, PyObject* args) {
    float scale;
    if (!PyArg_ParseTuple(args, "f", &scale)) {
        return PythonExt::InvalidArgumentException(__OlxSourceInfo, "f");
    }
    TXGrid::GetInstance()->SetScale(scale);
    return PythonExt::PyTrue();
}
//.............................................................................
PyObject* pyIsVisible(PyObject* self, PyObject* args) {
  return Py_BuildValue("b", TXGrid::GetInstance()->IsVisible());
}
//.............................................................................
//.............................................................................

static PyMethodDef XGRID_Methods[] = {
  {"Init", pyInit, METH_VARARGS, "initialises grid memory"},
  {"GetSize", pyGetSize, METH_VARARGS, "returns size of the grid"},
  {"Import", pyImport, METH_VARARGS, "imports grid from an array"},
  {"SetValue", pySetValue, METH_VARARGS, "sets grid value"},
  {"GetValue", pyGetValue, METH_VARARGS, "gets grid value"},
  {"SetMinMax", pySetMinMax, METH_VARARGS,
  "sets minimum and maximum vaues of the grid"},
  {"SetHole", pySetHole, METH_VARARGS,
  "sets minimum and maximum vaues of the grid to be avoided"},
  {"IsVisible", pyIsVisible, METH_VARARGS, "returns grid visibility status"},
  {"SetVisible", pySetVisible, METH_VARARGS, "sets grid visibility"},
  {"InitSurface", pyInitSurface, METH_VARARGS,
   "initialisess surface drawing. If optional distance from VdW surface is"
   " provided - the surface gets masked by the structure as well"},
  {"SetSurfaceScale", pySetSurfaceScale, METH_VARARGS,
   "Sets Scale (e.g. Isovalue) of surface drawing. Float expected."},
  {NULL, NULL, 0, NULL}
  };

olxcstr &TXGrid::ModuleName() {
  static olxcstr mn = "olex_xgrid";
  return mn;
}

PyObject *TXGrid::PyInit() {
  return PythonExt::init_module(ModuleName(), XGRID_Methods);
}
#endif // _PYTHON
