/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "glrender.h"
#include "glgroup.h"
#include "styles.h"
#include "glbackground.h"
#include "gltexture.h"
#include "library.h"
#include "bapp.h"
#include "log.h"
#include "estrbuffer.h"
#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

//..............................................................................
AGOSettings::AGOSettings(TGlRenderer &p, const olxstr &name)
  : parent(p), name(name), style(0)
{
  SetToDelete(false);
  style = &parent.GetStyles().NewStyle(name, true);
  style->SetPersistent(true);
  // manage this from the parent object just to make sure
  // make sure this is called first!
  //parent.GetStyles().OnClear.InsertFirst(this);
}
//..............................................................................
AGOSettings::~AGOSettings() {
  parent.GetStyles().OnClear.Remove(this);
}
//..............................................................................
bool AGOSettings::Enter(const IOlxObject *, const IOlxObject *, TActionQueue *) {
  style = 0;
  return true;
}
//..............................................................................
bool AGOSettings::Exit(const IOlxObject *, const IOlxObject *, TActionQueue *) {
  style = &parent.GetStyles().NewStyle(name, true);
  style->SetPersistent(true);
  OnStyleChange();
  return true;
}
//..............................................................................
//..............................................................................
//..............................................................................
GLuint TGlRenderer::TGlListManager::NewList() {
  if (Pos >= Lists.Count()*Inc) {
    GLuint s = olx_gl::genLists(Inc);
    if (s == GL_INVALID_VALUE || s == GL_INVALID_OPERATION) {
      throw TFunctionFailedException(__OlxSourceInfo, "glGenLists");
    }
    Lists.Add(s);
  }
  const GLuint lp = Lists.GetLast() + Pos%Inc;
  Pos++;
  return lp;
}
//..............................................................................
void TGlRenderer::TGlListManager::Clear()  {
  for (size_t i = 0; i < Lists.Count(); i += Inc) {
    olx_gl::deleteLists(Lists[i], Inc);
  }
  Pos = 0;
  Lists.Clear();
}
//..............................................................................
//..............................................................................
//..............................................................................
TGlRenderer::TGlRenderer(AGlScene *S, size_t width, size_t height)
  : PerspectiveMatrix(4, 4),
  Top(0), Left(0), Width((int)width), Height((int)height), OWidth(0),
  StereoLeftColor(0, 1, 1, 1), StereoRightColor(1, 0, 0, 1),
  OnDraw(TBasicApp::GetInstance().NewActionQueue(olxappevent_GL_DRAW)),
  OnClear(TBasicApp::GetInstance().NewActionQueue("GL_CLEAR"))
{
  SetToDelete(false);
  poly_stipple = 0;
  AbsoluteTop = 0;
  CompiledListId = -1;
  FScene = S;
  FZoom = 1;
  FViewZoom = 1;
  FScene->Parent(this);
  Selecting = FPerspective = false;
  SetPerspectiveAngle(45);
  StereoFlag = 0;
  StereoAngle = 3;
  LookAt(0,0,1);

  Fog = false;
  FogType = GL_EXP;
  FogColor = 0x7f7f7f;
  FogDensity = 1;
  FogStart = 0;
  FogEnd = 10;
  SceneDepth = -1;

  FSelection = new TGlGroup(*this, "Selection");
  FSelection->SetSelected(true);
  FBackground = new TGlBackground(*this, "Background", false);
  FBackground->SetVisible(false);
  FCeiling = new TGlBackground(*this, "Ceiling", true);
  FCeiling->SetVisible(false);
  FGlImageChanged = true; // will cause its update
  FGlImage = 0;
  TextureManager = new TTextureManager();
  FTranslucentObjects.SetIncrement(16);
  FCollections.SetIncrement(16);
  FGObjects.SetIncrement(16);
  MaxRasterZ = 1;

  NearPlane = 1;
  FarPlane = 10;

  GLUSelection = true;
  Styles.OnClear.Add(this);
}
//..............................................................................
TGlRenderer::~TGlRenderer() {
  //Styles.OnClear.Remove(this);
  Styles.OnClear.Clear();
  for (size_t i = 0; i < ObjectSettings.Count(); i++) {
    delete ObjectSettings.GetValue(i);
  }
  ObjectSettings.Clear();
  Clear();
  delete FBackground;
  delete FCeiling;
  delete FSelection;
  delete FScene;
  delete TextureManager;
  if (poly_stipple != 0) {
    delete[] poly_stipple;
  }
}
//..............................................................................
bool TGlRenderer::Enter(const IOlxObject *s, const IOlxObject *d, TActionQueue *q) {
  if (s != &Styles) {
    return false;
  }
  for (size_t i = 0; i < ObjectSettings.Count(); i++) {
    ObjectSettings.GetValue(i)->Enter(s, d, q);
  }
  for (size_t i = 0; i < FCollections.Count(); i++) {
    FCollections.GetValue(i)->SetStyle(0);
  }
  return true;
}
//..............................................................................
bool TGlRenderer::Exit(const IOlxObject *s, const IOlxObject *d, TActionQueue *q) {
  if (s != &Styles) {
    return false;
  }
  for (size_t i = 0; i < ObjectSettings.Count(); i++) {
    ObjectSettings.GetValue(i)->Exit(s, d, q);
  }
  for (size_t i = 0; i < FCollections.Count(); i++) {
    FCollections.GetValue(i)->SetStyle(
      &Styles.NewStyle(FCollections.GetValue(i)->GetName(), true));
  }
  // groups are deleted by Clear, so should be removed!
  for (size_t i = 0; i < FGroups.Count(); i++) {
    FGObjects.Remove(FGroups[i]);
  }
  AGDObjList GO = FGObjects.GetList();
  for (size_t i = 0; i < GO.Count(); i++) {
    GO[i]->OnPrimitivesCleared();
    GO[i]->SetCreated(false);
  }
  Clear();
  /* this has to be called before create so that dependent objects cause no
  problems
  */
  for (size_t i = 0; i < GO.Count(); i++) {
    GO[i]->OnStyleChange();
  }
  for (size_t i = 0; i < GO.Count(); i++) {
    // some loose objects as labels can be created twice otherwise
    if (!GO[i]->IsCreated()) {
      GO[i]->Create();
      GO[i]->Compile();
      GO[i]->SetCreated(true);
    }
  }
  TGraphicsStyle* gs = Styles.FindStyle("GL.Stereo");
  if (gs != 0) {
    StereoLeftColor = gs->GetParam("left", StereoLeftColor.ToString(), true);
    StereoRightColor = gs->GetParam("right", StereoRightColor.ToString(), true);
    StereoAngle = gs->GetParam("angle", StereoAngle, true).ToDouble();
  }
  return true;
}
//..............................................................................
void TGlRenderer::Initialise() {
  GetScene().MakeCurrent();
  InitLights();
  for (size_t i = 0; i < Primitives.ObjectCount(); i++) {
    Primitives.GetObject(i).Compile();
  }
  FSelection->Create();
  FBackground->Create();
  FCeiling->Create();
  olxcstr vendor((const char*)olx_gl::getString(GL_VENDOR));
  ATI = vendor.StartsFrom("ATI");
  olx_gl::get(GL_LINE_WIDTH, &LineWidth);
  GLUSelection = TBasicApp::GetInstance().GetOptions().FindValue(
    "gl_selection", !vendor.StartsFrom("Intel")).ToBool();
  if (TBasicApp::GetInstance().GetOptions().FindValue(
    "gl_multisample", TrueString()).ToBool())
  {
    olx_gl::enable(GL_MULTISAMPLE);
  }
}
//..............................................................................
void TGlRenderer::InitLights() {
  SetView(true, 1);
  olx_gl::enable(GL_LIGHTING);
  olx_gl::enable(GL_DEPTH_TEST);
  olx_gl::hint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  olx_gl::clearDepth(1.0f);
  olx_gl::depthFunc(GL_LEQUAL);
  olx_gl::colorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  LightModel.Init();
}
//..............................................................................
void TGlRenderer::Clear() {
  GetScene().MakeCurrent();
  OnClear.Enter(this);
  for (size_t i = 0; i < ObjectSettings.Count(); i++) {
    ObjectSettings.GetValue(i)->OnRendererClear();
  }
  FSelection->Clear();
  FGroups.DeleteItems();
  FGroups.Clear();
  ListManager.Clear();
  if (CompiledListId != -1) {
    olx_gl::deleteLists(CompiledListId, 1);
    CompiledListId = -1;
  }
  for (size_t i = 0; i < FCollections.Count(); i++) {
    delete FCollections.GetValue(i);
  }
  FCollections.Clear();
  for (size_t i = 0; i < Primitives.ObjectCount(); i++) {
    delete &Primitives.GetObject(i);
  }
  for (size_t i = 0; i < Primitives.PropertiesCount(); i++) {
    delete &Primitives.GetProperties(i);
  }
  Primitives.Clear();
  FTranslucentIdentityObjects.Clear();
  FTranslucentObjects.Clear();
  FIdentityObjects.Clear();
  FGObjects.Clear();
  ClearMinMax();
  ReleaseGlImage();
  OnClear.Exit(this);
}
//..............................................................................
void TGlRenderer::ClearObjects() {
  GetScene().MakeCurrent();
  FSelection->Clear();
  FGroups.DeleteItems();
  FGroups.Clear();
  if (CompiledListId != -1) {
    olx_gl::deleteLists(CompiledListId, 1);
    CompiledListId = -1;
  }
  for (size_t i = 0; i < FCollections.Count(); i++) {
    FCollections.GetValue(i)->ClearObjects();
  }
  FGObjects.Clear();
  ClearMinMax();
  ReleaseGlImage();
}
//..............................................................................
void TGlRenderer::ReleaseGlImage() {
  if (FGlImage != 0) {
    delete[] FGlImage;
    FGlImage = 0;
  }
}
//..............................................................................
void TGlRenderer::UpdateGlImage() {
  ReleaseGlImage();
  FGlImage = GetPixels();
  GlImageHeight = Height;
  GlImageWidth = Width;
  FGlImageChanged = false;
}
//..............................................................................
void TGlRenderer::ClearMinMax() {
  FMaxV[0] = FMaxV[1] = FMaxV[2] = -100;
  FMinV[0] = FMinV[1] = FMinV[2] = +100;
  SceneDepth = -1;
}
//..............................................................................
void TGlRenderer::UpdateMinMax(const vec3d& Min, const vec3d& Max) {
  if (Max[0] > FMaxV[0]) {
    FMaxV[0] = Max[0];
  }
  if (Max[1] > FMaxV[1]) {
    FMaxV[1] = Max[1];
  }
  if (Max[2] > FMaxV[2]) {
    FMaxV[2] = Max[2];
  }

  if (Min[0] < FMinV[0]) {
    FMinV[0] = Min[0];
  }
  if (Min[1] < FMinV[1]) {
    FMinV[1] = Min[1];
  }
  if (Min[2] < FMinV[2]) {
    FMinV[2] = Min[2];
  }
}
//..............................................................................
void TGlRenderer::operator = (const TGlRenderer &G)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
TGPCollection& TGlRenderer::NewCollection(const olxstr &Name) {
  TGPCollection *GPC = FCollections.Add(Name, new TGPCollection(*this, Name));
  GPC->SetStyle(&Styles.NewStyle(Name, true));
  return *GPC;
}
//..............................................................................
int TGlRenderer_CollectionComparator(const olxstr& c1, const olxstr& c2) {
  const size_t l = olx_min(c1.Length(), c2.Length());
  int dc = 0;
  size_t i=0;
  for( ; i < l; i++ )  {
    if (c1.CharAt(i) != c2.CharAt(i)) {
      break;
    }
    if (c1.CharAt(i) == '.') {
      dc++;
    }
  }
  if( i == l )  {
    if (c1.Length() == c2.Length()) {
      dc++;
    }
    else {
      if (l < c1.Length() && c1.CharAt(l) == '.') {
        dc++;
      }
      else {
        dc--;
      }
    }
  }
  return dc;
}
TGPCollection *TGlRenderer::FindCollectionX(const olxstr& Name,
  olxstr& CollName)
{
  const size_t di = Name.FirstIndexOf('.');
  if (di != InvalidIndex) {
    const size_t ind = FCollections.IndexOf(Name);
    if (ind != InvalidIndex) {
      return FCollections.GetValue(ind);
    }

    TGPCollection *BestMatch = 0;
    short maxMatchLevels = 0;
    for (size_t i = 0; i < FCollections.Count(); i++) {
      int dc = TGlRenderer_CollectionComparator(Name, FCollections.GetKey(i));
      if (dc == 0 || dc < maxMatchLevels) {
        continue;
      }
      // keep the one with shortest name
      if (BestMatch != 0 && dc == maxMatchLevels) {
        if (BestMatch->GetName().Length() > FCollections.GetKey(i).Length()) {
          BestMatch = FCollections.GetValue(i);
        }
      }
      else {
        BestMatch = FCollections.GetValue(i);
      }
      maxMatchLevels = dc;
    }
    if (BestMatch != 0) {
      if (Name.StartsFrom(BestMatch->GetName())) {
        return BestMatch;
      }
      CollName = Name.SubStringTo(di);
      return FindCollection(CollName);
    }
    CollName = Name.SubStringTo(di);
    return FindCollection(CollName);
  }
  else {
    CollName = Name;
    return FindCollection(Name);
  }
}
//..............................................................................
TGlPrimitive& TGlRenderer::NewPrimitive(short type) {
  return Primitives.AddObject(new TGlPrimitive(Primitives, *this, type));
}
//..............................................................................
void TGlRenderer::EnableFog(bool Set) {
  if (Set) {
    olx_gl::enable(GL_FOG);
    olx_gl::fog(GL_FOG_MODE, FogType);
    olx_gl::fog(GL_FOG_DENSITY, FogDensity);
    olx_gl::fog(GL_FOG_COLOR, FogColor.Data());
    olx_gl::fog(GL_FOG_START, FogStart);
    olx_gl::fog(GL_FOG_END, FogEnd);
  }
  else {
    olx_gl::disable(GL_FOG);
  }
  Fog = Set;
}
//..............................................................................
void TGlRenderer::EnablePerspective(bool Set) {
  FPerspective = Set;
}
//..............................................................................
void TGlRenderer::SetPerspectiveAngle(double angle) {
  FPAngle = tan(angle*M_PI / 360);
  double t = -1.0*FPAngle;
  double b = -t;
  double l = t;
  double r = -l;
  PerspectiveMatrix[0][0] = 2 * NearPlane / (r - l);
  PerspectiveMatrix[1][1] = 2 * NearPlane / (b - t);
  PerspectiveMatrix[2][2] = -(FarPlane + NearPlane) / (FarPlane - NearPlane);
  PerspectiveMatrix[2][3] = -2 * FarPlane*NearPlane / (FarPlane - NearPlane);
  PerspectiveMatrix[3][2] = -1;
}
//..............................................................................
double TGlRenderer::CalcRasterZ(double off) const {
  return olx_sign(GetMaxRasterZ())*(olx_abs(GetMaxRasterZ()) - off);
}
//..............................................................................
vec3d TGlRenderer::Project(const vec3d &v) const {
  vec3d shift = GetBasis().GetCenter();
  double sm = -(NearPlane + 1) / GetBasis().GetZoom();
  shift[0] += GetBasis().GetMatrix()[0][2] * sm;
  shift[1] += GetBasis().GetMatrix()[1][2] * sm;
  shift[2] += GetBasis().GetMatrix()[2][2] * sm;
  vec3d rs = (v + shift) * GetBasis().GetMatrix();
  rs *= GetBasis().GetZoom();
  if (!FPerspective  && StereoFlag == 0) {
    return rs;
  }
  evecd v1(4);
  v1[0] = rs[0];
  v1[1] = rs[1];
  v1[2] = rs[2];
  v1[3] = 1;
  v1 = PerspectiveMatrix*v1;
  return vec3d(v1[0] / v1[3], v1[1] / v1[3], v1[2] / v1[3]);
}
//..............................................................................
void TGlRenderer::Resize(size_t w, size_t h) {
  Resize(0, 0, w, h, 1);
}
//..............................................................................
void TGlRenderer::Resize(int l, int t, size_t w, size_t h, double Zoom) {
  Left = l;
  Top = t;
  if (StereoFlag == glStereoCross) {
    Width = (int)w / 2;
    OWidth = (int)w;
  }
  else {
    Width = (int)w;
  }
  Height = (int)h;
  FZoom = Zoom;
  FGlImageChanged = true;
  SetView(false);
}
//..............................................................................
void TGlRenderer::SetView(bool i, short Res)  {
  SetView(0, 0, i , false, Res);
}
//..............................................................................
void TGlRenderer::SetZoom(double V) {
  //const double MaxZ = olx_max(FMaxV.DistanceTo(FMinV), 1);
  //double dv = V*MaxZ;
  if (V < 0.001) { //  need to fix the zoom
    FBasis.SetZoom(0.001);
  }
  else if (V > 100) {
    FBasis.SetZoom(100);
  }
  else {
    FBasis.SetZoom(V);
  }
}
//..............................................................................
void TGlRenderer::SetView(int x, int y, bool identity, bool Select, short Res) {
  olx_gl::viewport(Left*Res, Top*Res, Width*Res, Height*Res);
  olx_gl::matrixMode(GL_PROJECTION);
  olx_gl::loadIdentity();
  if (Select && GLUSelection) {
    GLint vp[4];
    olx_gl::get(GL_VIEWPORT, vp);
    gluPickMatrix(x, Height - y, 3, 3, vp);
  }
  const double aspect = (double)Width / (double)Height;
  
  if (!identity) {
    if (FPerspective || StereoFlag != 0) {
      double right = FPAngle*aspect;
      olx_gl::frustum(right*FProjectionLeft, right*FProjectionRight,
        FPAngle*FProjectionTop, FPAngle*FProjectionBottom,
        NearPlane, FarPlane);
    }
    else {
      olx_gl::ortho(aspect*FProjectionLeft, aspect*FProjectionRight,
        FProjectionTop, FProjectionBottom,
        NearPlane, FarPlane);
    }
  }
  else {
    olx_gl::ortho(aspect*FProjectionLeft, aspect*FProjectionRight,
      FProjectionTop, FProjectionBottom, -1, 1);
  }
  olx_gl::matrixMode(GL_MODELVIEW);
  /* Mxv ->
    x = {(Bf[0][0]*x+Bf[0][1]*y+Bf[0][2]*z+Bf[0][3]*w)},
    y = {(Bf[1][0]*x+Bf[1][1]*y+Bf[1][2]*z+Bf[1][3]*w)},
    z = {(Bf[2][0]*x+Bf[2][1]*y+Bf[2][2]*z+Bf[2][3]*w)},
    w = {(Bf[3][0]*x+Bf[3][1]*y+Bf[3][2]*z+Bf[3][3]*w)}
  */
  if (!identity) {
    vec3f eye_p = vec3f(0, 0, 0);
    float Bf[4][4];
    memcpy(&Bf[0][0], GetBasis().GetMData(), 12 * sizeof(float));
    Bf[3][0] = eye_p[0];
    Bf[3][1] = eye_p[1];
    Bf[3][2] = eye_p[2];
    Bf[3][3] = 1;
    olx_gl::loadMatrix(&Bf[0][0]);
    olx_gl::scale(GetBasis().GetZoom());
    vec3d t = GetBasis().GetCenter();
    t += GetBasis().GetMatrix()*vec3d(0, 0, -(NearPlane+1) / GetBasis().GetZoom());
    olx_gl::translate(t);
  }
  else {
    olx_gl::loadIdentity();
  }
}
//..............................................................................
void TGlRenderer::SetupStencilFoInterlacedDraw(bool even) {
  if (poly_stipple == 0 || (poly_even != even)) {
    if (poly_stipple == 0) {
      poly_stipple = new GLubyte[128];
    }
    // horizontal interlacing
    for (size_t i=0; i < 128; i+=8) {
      *((uint32_t*)(&poly_stipple[i])) = even ? 0 : ~0;
      *((uint32_t*)(&poly_stipple[i+4])) = even ? ~0 : 0;
    }
    poly_even = even;
    // this is for the vertical interlacing
    //memset(poly_stipple, even ? 0x55 : 0xAA, 128);
  }
  SetView(true);
  olx_gl::FlagManager fm;
  fm.disable(GL_LIGHTING);
  fm.disable(GL_DEPTH_TEST);

  olx_gl::clear(GL_STENCIL_BUFFER_BIT);
  olx_gl::stencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
  olx_gl::stencilFunc(GL_ALWAYS, 1, ~0);
  fm.enable(GL_STENCIL_TEST);

  olx_gl::polygonStipple(poly_stipple);
  fm.enable(GL_POLYGON_STIPPLE);
  olx_gl::colorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

  const double aspect = (double)Width/(double)Height;
  glRectd(-0.5*aspect, -0.5, 0.5*aspect, 0.5);

  olx_gl::colorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}
//..............................................................................
void TGlRenderer::Draw() {
  if (Width < 50 || Height < 50 || !GetScene().MakeCurrent()) {
    return;
  }
  olx_gl::enable(GL_NORMALIZE);
  olx_gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  olx_gl::cullFace(GL_BACK);
  OnDraw.Enter(this);
  //glLineWidth( (float)(0.07/GetScale()) );
  //glPointSize( (float)(0.07/GetScale()) );
  if (StereoFlag == glStereoColor) {
    olx_gl::clearColor(0.0, 0.0, 0.0, 0.0);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const double ry = GetBasis().GetRY();
    olx_gl::FlagManager fm;
    fm.disable(GL_ALPHA_TEST);
    fm.disable(GL_BLEND);
    fm.disable(GL_CULL_FACE);
    fm.enable(GL_COLOR_MATERIAL);
    olx_gl::colorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    // right eye
    GetBasis().RotateY(ry + StereoAngle);
    olx_gl::colorMask(
      StereoRightColor[0] != 0,
      StereoRightColor[1] != 0,
      StereoRightColor[2] != 0,
      StereoRightColor[3] != 0);
    olx_gl::color(StereoRightColor.Data());
    DrawObjects(0, 0, false, false);
    //left eye
    GetBasis().RotateY(ry - StereoAngle);
    olx_gl::clear(GL_DEPTH_BUFFER_BIT);
    fm.enable(GL_BLEND);
    olx_gl::blendFunc(GL_ONE, GL_ONE);
    olx_gl::colorMask(
      StereoLeftColor[0] != 0,
      StereoLeftColor[1] != 0,
      StereoLeftColor[2] != 0,
      StereoLeftColor[3] != 0);
    olx_gl::color(StereoLeftColor.Data());
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry);
    olx_gl::colorMask(true, true, true, true);
  }
  // http://local.wasp.uwa.edu.au/~pbourke/texture_colour/anaglyph/
  else if (StereoFlag == glStereoAnaglyph) {
    const double ry = GetBasis().GetRY();
    olx_gl::clearColor(0.0, 0.0, 0.0, 0.0);
    olx_gl::clearAccum(0.0, 0.0, 0.0, 0.0);
    olx_gl::colorMask(true, true, true, true);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // right eye
    GetBasis().RotateY(ry + StereoAngle);
    olx_gl::colorMask(
      StereoRightColor[0] != 0,
      StereoRightColor[1] != 0,
      StereoRightColor[2] != 0,
      StereoRightColor[3] != 0);
    DrawObjects(0, 0, false, false);
    olx_gl::colorMask(true, true, true, true);
    olx_gl::accum(GL_LOAD, 1);
    // left eye
    GetBasis().RotateY(ry - StereoAngle);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    olx_gl::colorMask(
      StereoLeftColor[0] != 0,
      StereoLeftColor[1] != 0,
      StereoLeftColor[2] != 0,
      StereoLeftColor[3] != 0);
    DrawObjects(0, 0, false, false);
    olx_gl::colorMask(true, true, true, true);
    olx_gl::accum(GL_ACCUM, 1);
    olx_gl::accum(GL_RETURN, 1.0);
    GetBasis().RotateY(ry);
  }
  else if (StereoFlag == glStereoHardware) {
    const double ry = GetBasis().GetRY();
    GetBasis().RotateY(ry + StereoAngle);
    olx_gl::drawBuffer(GL_BACK_LEFT);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry - StereoAngle);
    olx_gl::drawBuffer(GL_BACK_RIGHT);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DrawObjects(0, 0, false, false);
    olx_gl::drawBuffer(GL_BACK);
    GetBasis().RotateY(ry);
  }
  else if (StereoFlag == glStereoInterlace) {
    olx_gl::drawBuffer(GL_BACK);
    SetupStencilFoInterlacedDraw((AbsoluteTop % 2) != 0);
    olx_gl::FlagEnabler fe_(GL_STENCIL_TEST);
    olx_gl::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    const double ry = GetBasis().GetRY();
    GetBasis().RotateY(ry + StereoAngle);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    olx_gl::stencilFunc(GL_EQUAL, 0, ~0);
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry - StereoAngle);
    olx_gl::stencilFunc(GL_NOTEQUAL, 0, ~0);
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry);
  }
  else if (StereoFlag == glStereoCross) {
    const double ry = GetBasis().GetRY();
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const int _l = Left;
    GetBasis().RotateY(ry + StereoAngle);
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry - StereoAngle);
    Left = Width;
    DrawObjects(0, 0, false, false);
    GetBasis().RotateY(ry);
    Left = _l;
  }
  else if (StereoFlag == glStereoMatrix) {
    mat3d m = GetBasis().GetMatrix();
    vec3d t = GetBasis().GetCenter();

    olx_gl::clearColor(0.0, 0.0, 0.0, 0.0);
    olx_gl::clearAccum(0.0, 0.0, 0.0, 0.0);
    olx_gl::colorMask(true, true, true, true);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    olx_gl::colorMask(
      StereoRightColor[0] != 0,
      StereoRightColor[1] != 0,
      StereoRightColor[2] != 0,
      StereoRightColor[3] != 0);
    DrawObjects(0, 0, false, false);
    olx_gl::colorMask(true, true, true, true);
    olx_gl::accum(GL_LOAD, 1);

    GetBasis().SetMatrix(StereoMatrix*m);
    GetBasis().Translate(StereoTranslation);
    olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    olx_gl::colorMask(
      StereoLeftColor[0] != 0,
      StereoLeftColor[1] != 0,
      StereoLeftColor[2] != 0,
      StereoLeftColor[3] != 0);
    DrawObjects(0, 0, false, false);
    olx_gl::colorMask(true, true, true, true);
    olx_gl::accum(GL_ACCUM, 1);
    olx_gl::accum(GL_RETURN, 1.0);

    GetBasis().SetCenter(t);
    GetBasis().SetMatrix(m);
  }
  else {
    if (!GetScene().StartDraw()) {
      return;
    }
    DrawObjects(0, 0, false, false);
  }
  GetScene().EndDraw();
  FGlImageChanged = true;
  OnDraw.Execute(this);
  OnDraw.Exit(this);
}
//..............................................................................
void TGlRenderer::DrawSilhouette() {
  bool glu_sel = GLUSelection;
  GLUSelection = false;
  GetScene().MakeCurrent();
  GetScene().StartDraw();
  for (size_t i = 0; i < ObjectCount(); i++) {
    GetObject(i).SetTag((index_t)(i + 1));
  }
  DrawObjects(0, 0, true, false);
  GetScene().EndDraw();
  GLUSelection = glu_sel;
}
//..............................................................................
void TGlRenderer::HandleSelection(const AGDrawObject &o, const TGlPrimitive &p,
  bool SelectObjects, bool SelectPrimitives) const
{
  if (GLUSelection) {
    if (SelectObjects) {
      olx_gl::loadName((GLuint)o.GetTag());
    }
    else if (SelectPrimitives) {
      olx_gl::loadName((GLuint)p.GetTag());
    }
  }
  else {
    if (SelectObjects) {
      olx_gl::color(o.GetTag());
    }
    else if (SelectPrimitives) {
      olx_gl::color(p.GetTag());
    }
  }
}
//..............................................................................
//..............................................................................
int CompareObjectsZ_1(
  const AnAssociation3<TGlPrimitive *, AGDrawObject *, double> &a,
  const AnAssociation3<TGlPrimitive *,
  AGDrawObject *, double> &b)
{
  return olx_cmp(a.c, b.c);
}
//..............................................................................
void TGlRenderer::DrawObjects(int x, int y, bool SelectObjects,
  bool SelectPrimitives)
{
  olx_gl::pushAttrib(GL_ALL_ATTRIB_BITS);
  Selecting = (SelectObjects || SelectPrimitives);
  const bool skip_mat = (StereoFlag == glStereoColor || Selecting);
  if (Selecting) {
    olx_gl::enable(GL_COLOR_MATERIAL);
    olx_gl::disable(GL_LIGHTING);
    glShadeModel(GL_FLAT);
  }
  static const int DrawMask = sgdoSelected | sgdoGrouped | sgdoHidden;
  if (!FIdentityObjects.IsEmpty() || FSelection->GetGlM().IsIdentityDraw()) {
    SetView(x, y, true, Selecting, 1);
    const size_t id_obj_count = FIdentityObjects.Count();
    for (size_t i = 0; i < id_obj_count; i++) {
      TGlMaterial* GlM = FIdentityObjects[i];
      GlM->Init(skip_mat);
      const size_t obj_count = GlM->ObjectCount();
      for (size_t j = 0; j < obj_count; j++) {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM->GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        const size_t c_obj_count = GPC->ObjectCount();
        for (size_t k = 0; k < c_obj_count; k++) {
          AGDrawObject& GDO = GPC->GetObject(k);
          if (GDO.MaskFlags(DrawMask) != 0) {
            continue;
          }
          HandleSelection(GDO, GlP, SelectObjects, SelectPrimitives);
          olx_gl::pushMatrix();
          if (GDO.Orient(GlP)) {// the object has drawn itself
            olx_gl::popMatrix();
            continue;
          }
          GlP.Draw();
          olx_gl::popMatrix();
        }
      }
    }
    if (FSelection->GetGlM().IsIdentityDraw()) {
      olx_gl::pushAttrib(GL_ALL_ATTRIB_BITS);
      FSelection->Draw(SelectPrimitives, SelectObjects);
      olx_gl::popAttrib();
    }
    SetView(x, y, false, Selecting, 1);
  }
  else {
    SetView(x, y, false, Selecting, 1);
  }

  if (!Selecting && IsCompiled()) {
    olx_gl::callList(CompiledListId);
  }
  else {
    const size_t prim_count = Primitives.PropertiesCount();
    for (size_t i = 0; i < prim_count; i++) {
      TGlMaterial& GlM = Primitives.GetProperties(i);
      if (GlM.IsIdentityDraw()) {
        continue;  // already drawn
      }
      if (GlM.IsTransparent()) {
        continue;  // will be drawn
      }
      GlM.Init(skip_mat);
      const size_t obj_count = GlM.ObjectCount();
      for (size_t j = 0; j < obj_count; j++) {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM.GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        if (GPC == 0) {
          continue;
        }
        const size_t c_obj_count = GPC->ObjectCount();
        for (size_t k = 0; k < c_obj_count; k++) {
          AGDrawObject& GDO = GPC->GetObject(k);
          if (GDO.MaskFlags(DrawMask) != 0) {
            continue;
          }
          HandleSelection(GDO, GlP, SelectObjects, SelectPrimitives);
          olx_gl::pushMatrix();
          if (GDO.Orient(GlP)) { // the object has drawn itself
            olx_gl::popMatrix();
            continue;
          }
          GlP.Draw();
          olx_gl::popMatrix();
        }
      }
    }
  }
  const size_t group_count = FGroups.Count();
  for (size_t i = 0; i < group_count; i++) {
    if (FGroups[i]->GetParentGroup() == 0 &&
      !FGroups[i]->GetGlM().IsTransparent())
    {
      FGroups[i]->Draw(SelectPrimitives, SelectObjects);
    }
  }
  const size_t trans_obj_count = FTranslucentObjects.Count();
  //olx_gl::disable(GL_DEPTH_TEST);
  /* disabling the depth test does help for a set of transparent objects but
  then it does not help if there are any solid objects on the way
  */
  TTypeList<AnAssociation3<TGlPrimitive*, AGDrawObject*, double> > to_render;
  for (size_t i = 0; i < trans_obj_count; i++) {
    TGlMaterial* GlM = FTranslucentObjects[i];
    GlM->Init(skip_mat);
    const size_t obj_count = GlM->ObjectCount();
    for (size_t j = 0; j < obj_count; j++) {
      TGlPrimitive& GlP = (TGlPrimitive&)GlM->GetObject(j);
      TGPCollection* GPC = GlP.GetParentCollection();
      const size_t c_obj_count = GPC->ObjectCount();
      for (size_t k = 0; k < c_obj_count; k++) {
        AGDrawObject& GDO = GPC->GetObject(k);
        if (GDO.MaskFlags(DrawMask) != 0) {
          continue;
        }
        to_render.AddNew(&GlP, &GDO, GDO.CalcZ());
      }
    }
  }
  QuickSorter::SortSF(to_render, &CompareObjectsZ_1);
  for (size_t i = 0; i < to_render.Count(); i++) {
    HandleSelection(*to_render[i].b,
      *to_render[i].a, SelectObjects, SelectPrimitives);
    if (!skip_mat) {
      to_render[i].a->GetProperties().Init(false);
    }
    olx_gl::pushMatrix();
    if (to_render[i].b->Orient(*to_render[i].a)) {
      olx_gl::popMatrix();
      continue;
    }
    to_render[i].a->Draw();
    olx_gl::popMatrix();
  }
  //olx_gl::enable(GL_DEPTH_TEST);
  for (size_t i = 0; i < group_count; i++) {
    if (FGroups[i]->GetParentGroup() == 0 &&
      FGroups[i]->GetGlM().IsTransparent())
    {
      FGroups[i]->Draw(SelectPrimitives, SelectObjects);
    }
  }

  if (!FSelection->GetGlM().IsIdentityDraw()) {
    olx_gl::pushAttrib(GL_ALL_ATTRIB_BITS);
    FSelection->Draw(SelectPrimitives, SelectObjects);
    olx_gl::popAttrib();
  }
  if (!FTranslucentIdentityObjects.IsEmpty()) {
    SetView(x, y, true, Selecting, 1);
    const size_t trans_id_obj_count = FTranslucentIdentityObjects.Count();
    for (size_t i = 0; i < trans_id_obj_count; i++) {
      TGlMaterial* GlM = FTranslucentIdentityObjects[i];
      GlM->Init(skip_mat);
      const size_t obj_count = GlM->ObjectCount();
      for (size_t j = 0; j < obj_count; j++) {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM->GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        const size_t c_obj_count = GPC->ObjectCount();
        for (size_t k = 0; k < c_obj_count; k++) {
          AGDrawObject& GDO = GPC->GetObject(k);
          if (GDO.MaskFlags(DrawMask) != 0) {
            continue;
          }
          HandleSelection(GDO, GlP, SelectObjects, SelectPrimitives);
          olx_gl::pushMatrix();
          if (GDO.Orient(GlP)) { // the object has drawn itself
            olx_gl::popMatrix();
            continue;
          }
          GlP.Draw();
          olx_gl::popMatrix();
        }
      }
    }
  }
  olx_gl::popAttrib();
}
//..............................................................................
AGDrawObject* TGlRenderer::SelectObject(int x, int y) {
  if ((Width*Height) <= 100) {
    return 0;
  }
  for (size_t i = 0; i < ObjectCount(); i++) {
    GetObject(i).SetTag((int)(i + 1));
  }
  if (GLUSelection) {
    AGDrawObject *Result = 0;
    olx_array_ptr<GLuint> selectBuf(new GLuint[MAXSELECT]);
    if (!GetScene().StartSelect(x, y, selectBuf)) {
      return 0;
    }
    DrawObjects(x, y, true, false);
    int hits = GetScene().EndSelect();
    if (hits >= 1) {
      if (hits == 1) {
        GLuint in = selectBuf[(hits - 1) * 4 + 3];
        if (in >= 1 && in <= ObjectCount()) {
          Result = &GetObject(in - 1);
        }
      }
      else {
        unsigned int maxz = ~0;
        GLuint in = 0;
        for (int i = 0; i < hits; i++) {
          if (selectBuf[i * 4 + 1] < maxz) {
            in = i;
            maxz = selectBuf[i * 4 + 1];
          }
        }
        if ((int)(in)* 4 + 3 < 0) {
          return 0;
        }
        in = selectBuf[(in)* 4 + 3] - 1;
        if (in < ObjectCount()) {
          Result = &GetObject(in);
        }
      }
    }
    return Result;
  }
  else {
    if (!GetScene().StartDraw()) {
      return 0;
    }
    olx_gl::FlagDisabler fd_(GL_FOG);
    SetView(x, y, false, true, 1);
    DrawObjects(x, y, true, false);
    GetScene().EndDraw();
    fd_.enable();
    memset(&SelectionBuffer, 0, sizeof(SelectionBuffer));
    olx_gl::readPixels(x - 1, Height - y - 1, 3, 3, GL_RGB, GL_UNSIGNED_BYTE,
      &SelectionBuffer[0]);
    static const size_t indices[9][2] = {
      {1,3}, {1,0}, {1,6},
      {0,3}, {3,3},
      {0,0}, {0,6}, {3,0}, {3,6}
    };
    for (int i = 0; i < 9; i++) {
      size_t idx = OLX_RGB(
        SelectionBuffer[indices[i][0]][indices[i][1]],
        SelectionBuffer[indices[i][0]][indices[i][1] + 1],
        SelectionBuffer[indices[i][0]][indices[i][1] + 2]) - 1;
      if (idx < FGObjects.Count() && FGObjects[idx]->IsVisible()) {
        return FGObjects[idx];
      }
    }
    return 0;
  }
}
//..............................................................................
TGlPrimitive* TGlRenderer::SelectPrimitive(int x, int y) {
  if ((Width*Height) <= 100) {
    return 0;
  }
  for (size_t i = 0; i < Primitives.ObjectCount(); i++)
    Primitives.GetObject(i).SetTag((index_t)(i + 1));
  if (GLUSelection) {
    TGlPrimitive *Result = 0;
    olx_array_ptr<GLuint> selectBuf(new GLuint[MAXSELECT]);
    if (!GetScene().StartSelect(x, y, selectBuf)) {
      return 0;
    }
    DrawObjects(x, y, false, true);
    GetScene().EndSelect();
    int hits = olx_gl::renderMode(GL_RENDER);
    if (hits >= 1) {
      if (hits == 1) {
        GLuint in = selectBuf[(hits - 1) * 4 + 3];
        if (in >= 1 && in <= (PrimitiveCount() + 1)) {
          Result = &GetPrimitive(in - 1);
        }
      }
      else {
        unsigned int maxz = ~0;
        GLuint in = 0;
        for (int i = 0; i < hits; i++) {
          if (selectBuf[i * 4 + 1] < maxz) {
            in = i;
            maxz = selectBuf[i * 4 + 1];
          }
        }
        in = selectBuf[in * 4 + 3];
        if (in >= 1 && in <= (PrimitiveCount() + 1)) {
          Result = &GetPrimitive(in - 1);
        }
      }
    }
    return Result;
  }
  else {
    if (!GetScene().StartDraw()) {
      return 0;
    }
    SetView(x, y, false, true, 1);
    DrawObjects(x, y, false, true);
    GetScene().EndDraw();
    memset(&SelectionBuffer, 0, sizeof(SelectionBuffer));
    olx_gl::readPixels(x - 1, Height - y - 1, 3, 3, GL_RGB, GL_UNSIGNED_BYTE,
      &SelectionBuffer[0]);
    size_t idx = OLX_RGB(
      SelectionBuffer[1][3],
      SelectionBuffer[1][4],
      SelectionBuffer[1][5]) - 1;
    return (idx < PrimitiveCount()) ? &GetPrimitive(idx) : 0;
  }
}
//..............................................................................
TGlGroup* TGlRenderer::FindObjectGroup(const AGDrawObject& G) const {
  // get the topmost group
  TGlGroup* G1 = G.GetParentGroup();
  if (G1 == 0) {
    return 0;
  }
  while (G1->GetParentGroup() != 0) {
    if (G1->GetParentGroup() == FSelection) {
      break;
    }
    G1 = G1->GetParentGroup();
  }
  return (G1 == FSelection) ? 0 : G1;
}
//..............................................................................
void TGlRenderer::Select(AGDrawObject& G) {
  G.SetSelected(FSelection->Add(G));
}
//..............................................................................
void TGlRenderer::Deselect(AGDrawObject& G) {
  if (G.GetParentGroup() == FSelection) {
    FSelection->Remove(G);
  }
}
//..............................................................................
void TGlRenderer::Select(AGDrawObject& G, bool v) {
  if (v) {
    if (!G.IsSelected()) {
      Select(G);
    }
  }
  else if (G.IsSelected()) {
    Deselect(G);
  }
}
//..............................................................................
void TGlRenderer::Select(AGDrawObject& G, glSelectionFlag v) {
  switch (v) {
  case glSelectionSelect:
    if (!G.IsSelected()) {
      Select(G);
    }
    break;
  case glSelectionUnselect:
    if (G.IsSelected()) {
      Deselect(G);
    }
    break;
  case glSelectionInvert:
    Select(G, !G.IsSelected());
  }
}
//..............................................................................
void TGlRenderer::InvertSelection() {
  AGDObjList to_select;
  uint32_t mask = sgdoSelected | sgdoHidden | sgdoGrouped;
  const size_t oc = FGObjects.Count();
  for (size_t i = 0; i < oc; i++) {
    AGDrawObject* GDO = FGObjects[i];
    if (GDO->MaskFlags(mask) == 0 &&
      GDO->IsSelectable() &&
      GDO != FSelection)
    {
      to_select.Add(GDO);
    }
  }
  FSelection->SetSelected(false);
  FSelection->Clear();
  for (size_t i = 0; i < to_select.Count(); i++) {
    to_select[i]->SetSelected(FSelection->Add(*to_select[i]));
  }
}
//..............................................................................
void TGlRenderer::SelectAll(bool Select) {
  if (Select) {
    for (size_t i = 0; i < ObjectCount(); i++) {
      AGDrawObject& GDO = GetObject(i);
      // grouped covers selected
      if (!GDO.IsGrouped() && GDO.IsVisible() && GDO.IsSelectable()) {
        GDO.SetSelected(FSelection->Add(GDO));
      }
    }
  }
  else {
    FSelection->SetSelected(false);
    FSelection->Clear();
  }
}
//..............................................................................
void TGlRenderer::ClearGroups(bool clean) {
  if (clean) {
    FGObjects.ForEach(ACollectionItem::TagSetter(0));
  }
  for (size_t i = 0; i < FGroups.Count(); i++) {
    if (FGroups[i]->IsSelected()) {
      Deselect(*FGroups[i]);
    }
    FGroups[i]->Clear();
    FGroups[i]->SetTag(1);
    FGroups[i]->GetPrimitives().RemoveObject(*FGroups[i]);
  }
  if (clean) {
    FGObjects.Pack(ACollectionItem::TagAnalyser(1));
  }
  FGroups.DeleteItems();
  FGroups.Clear();
}
//..............................................................................
TGlGroup* TGlRenderer::FindGroupByName(const olxstr& colName) const {
  for (size_t i = 0; i < FGroups.Count(); i++) {
    if (FGroups[i]->GetCollectionName() == colName) {
      return FGroups[i];
    }
  }
  return 0;
}
//..............................................................................
void TGlRenderer::ClearSelection() {
  FSelection->Clear();
}
//..............................................................................
TGlGroup* TGlRenderer::GroupSelection(const olxstr& groupName) {
  if (FSelection->Count() > 0) {
    AGDObjList ungroupable;
    if (!FSelection->TryToGroup(ungroupable)) {
      return 0;
    }
    TGlGroup *OS = FSelection;
    FGroups.Add(FSelection);
    OS->GetPrimitives().RemoveObject(*OS);
    FSelection = new TGlGroup(*this, "Selection");
    FSelection->Create();
    for (size_t i = 0; i < ungroupable.Count(); i++) {
      FSelection->Add(*ungroupable[i]);
    }
    // read style information for this particular group
    OS->SetSelected(false);
    FGObjects.Remove(OS);  // avoid duplication in the list!
    OS->Create(groupName);
    return OS;
  }
  return 0;
}
//..............................................................................
TGlGroup& TGlRenderer::NewGroup(const olxstr& collection_name) {
  return *FGroups.Add(new TGlGroup(*this, collection_name));
}
//..............................................................................
void TGlRenderer::Ungroup(TGlGroup& OS) {
  FGroups.Remove(OS);
  FSelection->Remove(OS);

  AGDObjList Objects(OS.Count());
  for (size_t i = 0; i < OS.Count(); i++) {
    Objects[i] = &OS[i];
  }
  OS.GetPrimitives().RemoveObject(OS); //
  FGObjects.Remove(&OS);
  delete &OS;  // it will reset Parent group to NULL in the objects
  for (size_t i = 0; i < Objects.Count(); i++) {
    FSelection->Add(*Objects[i]);
  }
  FSelection->SetSelected(true);
}
//..............................................................................
void TGlRenderer::EnableClipPlane(TGlClipPlane *P, bool v) {
  if (v) {
    double v[4];
    v[0] = P->Equation()[0];
    v[1] = P->Equation()[1];
    v[2] = P->Equation()[2];
    v[3] = P->Equation()[3];
    olx_gl::clipPlane(P->Id(), &v[0]);
    olx_gl::enable(P->Id());
  }
  else {
    olx_gl::disable(P->Id());
  }
}
//..............................................................................
// tracks translucent and identity objects
void TGlRenderer::SetProperties(TGlMaterial& P) {
  if (P.IsTransparent() && P.IsIdentityDraw()) {
    FTranslucentIdentityObjects.AddUnique(&P);
    return;
  }
  if (P.IsTransparent()) {
    FTranslucentObjects.AddUnique(&P);
    return;
  }
  if (P.IsIdentityDraw()) {
    FIdentityObjects.AddUnique(&P);
    return;
  }
}
//..............................................................................
// tracks translucent and identity objects
void TGlRenderer::OnSetProperties(const TGlMaterial& P) {
  // the properties will not be removed
  if (P.ObjectCount() > 1) {
    return;
  }
  if (P.IsTransparent() && P.IsIdentityDraw()) {
    FTranslucentIdentityObjects.Remove(P);
    return;
  }
  if (P.IsTransparent()) {
    FTranslucentObjects.Remove(P);
    return;
  }
  if (P.IsIdentityDraw()) {
    FIdentityObjects.Remove(P);
    return;
  }
}
//..............................................................................
void TGlRenderer::RemoveObjects(const AGDObjList& objects) {
  ACollectionItem::Exclude<>(FGObjects, objects);
}
//..............................................................................
//struct ADO : public ADestructionObserver {
//  TGlRenderer &parent;
//  ADO(TGlRenderer &parent) : parent(parent)
//  {}
//  virtual void call(class APerishable* obj) const {
//    if (TBasicApp::HasInstance()) {
//      for (size_t i = 0; i < parent.ObjectCount(); i++) {
//        APerishable *x = dynamic_cast<APerishable *>(&parent.GetObject(i));
//        if ( x == obj) {
//          TBasicApp::NewLogEntry() << typeid(&parent.GetObject(i)).name();
//          break;
//        }
//      }
//    }
//  }
//  virtual bool operator == (const ADestructionObserver *x) const {
//    return this == x;
//  }
//  virtual ADestructionObserver *clone() const {
//    return new ADO(parent);
//  }
//};
void TGlRenderer::AddObject(AGDrawObject& G) {
  FGObjects.AddUnique(&G);
  //G.AddDestructionObserver(*(new ADO(*this)));
  if (FSceneComplete || !G.IsVisible()) {
    return;
  }
  vec3d MaxV, MinV;
  if (G.GetDimensions(MaxV, MinV)) {
    UpdateMinMax(MinV, MaxV);
  }
}
//..............................................................................
void TGlRenderer::RemoveCollection(TGPCollection& GP) {
  FTranslucentIdentityObjects.Clear();
  FTranslucentObjects.Clear();
  FIdentityObjects.Clear();

  Primitives.GetObjects().ForEach(ACollectionItem::TagSetter(-1));
  GP.GetPrimitives().ForEach(ACollectionItem::TagSetter(0));
  Primitives.RemoveObjectsByTag(0);
  FCollections.Delete(FCollections.IndexOfValue(&GP));
  for (size_t i = 0; i < Primitives.PropertiesCount(); i++) {
    TGlMaterial& GlM = Primitives.GetProperties(i);
    if (GlM.IsTransparent() && GlM.IsIdentityDraw()) {
      FTranslucentIdentityObjects.Add(GlM);
    }
    else if (GlM.IsTransparent()) {
      FTranslucentObjects.Add(GlM);
    }
    else if (GlM.IsIdentityDraw()) {
      FIdentityObjects.Add(GlM);
    }
  }
  delete &GP;
}
//..............................................................................
void TGlRenderer::RemoveCollections(const TPtrList<TGPCollection>& Colls_) {
  if (Colls_.IsEmpty()) {
    return;
  }
  TPtrList<TGPCollection> colls = ACollectionItem::Unique(Colls_);
  FTranslucentIdentityObjects.Clear();
  FTranslucentObjects.Clear();
  FIdentityObjects.Clear();
  Primitives.GetObjects().ForEach(ACollectionItem::TagSetter(-1));
  for (size_t i = 0; i < colls.Count(); i++) {
    colls[i]->GetPrimitives().ForEach(ACollectionItem::TagSetter(0));
    const size_t col_ind = FCollections.IndexOfValue(colls[i]);
    FCollections.Delete(col_ind);
    delete colls[i];
  }
  Primitives.RemoveObjectsByTag(0);
  for (size_t i = 0; i < Primitives.PropertiesCount(); i++) {
    TGlMaterial& GlM = Primitives.GetProperties(i);
    if (GlM.IsTransparent() && GlM.IsIdentityDraw()) {
      FTranslucentIdentityObjects.Add(&GlM);
    }
    else if (GlM.IsTransparent()) {
      FTranslucentObjects.Add(GlM);
    }
    else if (GlM.IsIdentityDraw()) {
      FIdentityObjects.Add(GlM);
    }
  }
}
//..............................................................................
void TGlRenderer::LookAt(double x, double y, short res) {
  FViewZoom = (float)(1.0 / res);
  FProjectionLeft = (float)(x / (double)res - 0.5);
  FProjectionRight = (float)((x + 1) / (double)res - 0.5);
  FProjectionTop = (float)(y / (double)res - 0.5);
  FProjectionBottom = (float)((y + 1) / (double)res - 0.5);
}
//..............................................................................
char* TGlRenderer::GetPixels(bool useMalloc, short aligment, GLuint format) {
  GetScene().MakeCurrent();
  char *Bf;
  short extraBytes = aligment - (Width * 3) % aligment;
  if (useMalloc) {
    Bf = (char*)malloc((Width * 3 + extraBytes)*Height);
  }
  else {
    Bf = new char[(Width * 3 + extraBytes)*Height];
  }
  if (Bf == 0) {
    throw TOutOfMemoryException(__OlxSourceInfo);
  }
  olx_gl::readBuffer(GL_BACK);
  olx_gl::pixelStore(GL_PACK_ALIGNMENT, aligment);
  olx_gl::readPixels(0, 0, Width, Height, format, GL_UNSIGNED_BYTE, Bf);
  return Bf;
}
//..............................................................................
void TGlRenderer::RemovePrimitiveByTag(int in) {
  Primitives.RemoveObjectsByTag(in);
  FTranslucentIdentityObjects.Clear();
  FTranslucentObjects.Clear();
  FIdentityObjects.Clear();
  for (size_t i = 0; i < Primitives.PropertiesCount(); i++) {
    TGlMaterial& GlM = Primitives.GetProperties(i);
    if (GlM.IsTransparent() && GlM.IsIdentityDraw()) {
      FTranslucentIdentityObjects.Add(GlM);
    }
    else if (GlM.IsTransparent()) {
      FTranslucentObjects.Add(GlM);
    }
    else if (GlM.IsIdentityDraw()) {
      FIdentityObjects.Add(GlM);
    }
  }
}
//..............................................................................
// removes styles, which are not used by any collection
void TGlRenderer::CleanUpStyles() {
  Styles.SetStylesTag(0);
  for (size_t i = 0; i < FCollections.Count(); i++) {
    FCollections.GetValue(i)->GetStyle().SetTag(1);
  }
  Styles.RemoveStylesByTag(0);
}
//..............................................................................
void TGlRenderer::Compile(bool v) {
  /* remark: it works, but gives no performance boost ... */
  //return;
  if (v) {
    if (CompiledListId == -1) {
      CompiledListId = olx_gl::genLists(1);
    }
    olx_gl::newList(CompiledListId, GL_COMPILE);
    for (size_t i = 0; i < Primitives.PropertiesCount(); i++) {
      TGlMaterial& GlM = Primitives.GetProperties(i);
      if (GlM.IsIdentityDraw()) {
        continue;  // already drawn
      }
      if (GlM.IsTransparent()) {
        continue;  // will be drawn
      }
      GlM.Init(false);
      for (size_t j = 0; j < GlM.ObjectCount(); j++) {
        TGlPrimitive& GlP = (TGlPrimitive&)GlM.GetObject(j);
        TGPCollection* GPC = GlP.GetParentCollection();
        if (GPC == 0) {
          continue;
        }
        for (size_t k = 0; k < GPC->ObjectCount(); k++) {
          AGDrawObject& GDO = GPC->GetObject(k);
          if (!GDO.IsVisible()) {
            continue;
          }
          if (GDO.IsSelected()) {
            continue;
          }
          if (GDO.IsGrouped()) {
            continue;
          }
          olx_gl::pushMatrix();
          if (GDO.Orient(GlP)) { // the object has drawn itself
            olx_gl::popMatrix();
            continue;
          }
          GlP.Draw();
          olx_gl::popMatrix();
        }
      }
    }
    olx_gl::endList();
  }
  else {
    if (CompiledListId != -1) {
      olx_gl::deleteLists(CompiledListId, 1);
      CompiledListId = -1;
    }
  }
}
//..............................................................................
void TGlRenderer::DrawText(TGlPrimitive& p, double x, double y, double z) {
  p.GetFont()->Reset_ATI(ATI);
  olx_gl::rasterPos(x, y, z);
  p.Draw();
}
//..............................................................................
void TGlRenderer::DrawTextSafe(const vec3d& pos, const olxstr& text,
  const TGlFont& fnt)
{
  fnt.Reset_ATI(ATI);
  // set a valid raster position
  olx_gl::rasterPos(0.0, 0.0, pos[2]);
  olx_gl::bitmap(0, 0, 0, 0,
    (float)(pos[0] / FViewZoom), (float)(pos[1] / FViewZoom), 0);
  fnt.DrawRasterText(text);
}
//..............................................................................
void TGlRenderer::SetLineWidth(double lw) {
  LineWidth = lw;
  olx_gl::lineWidth(LineWidth);
}
//..............................................................................
void TGlRenderer::BeforeContextChange() {
  Clear();
  TextureManager->BeforeContextChange();
}
//..............................................................................
void TGlRenderer::AfterContextChange() {
  TextureManager->AfterContextChange();
}
//..............................................................................
TEBitArray::const_type TGlRenderer::GetVisibility() {
  TEBitArray vis(FGObjects.Count());
  for (size_t i = 0; i < FGObjects.Count(); i++) {
    vis.Set(i, FGObjects[i]->IsVisible());
  }
  return vis;
}
//..............................................................................
void TGlRenderer::SetVisibility(const TEBitArray &v) {
  if (v.Count() != FGObjects.Count()) {
    throw TInvalidArgumentException(__OlxSourceInfo, "visibility bit array");
  }
  for (size_t i = 0; i < FGObjects.Count(); i++) {
    FGObjects[i]->SetVisible(v[i]);
  }
}
//..............................................................................
double TGlRenderer::CalcZoom(bool for_selection) const {
  if (!for_selection) {
    const double df = SceneDepth < 0 ?
      (SceneDepth = olx_max(FMaxV.DistanceTo(FMinV), 1.0)) : SceneDepth;
    return 1. / df;
  }
  else {
    vec3d minv(1000), maxv(-1000);
    size_t cnt = 0;
    for (size_t i = 0; i < FSelection->Count(); i++) {
      if ((*FSelection)[i].GetDimensions(maxv, minv)) {
        cnt++;
      }
    }
    if (cnt == 0) {
      return CalcZoom(false);
    }
    return 1./ olx_max(maxv.DistanceTo(minv), 1.0);
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
void TGlRenderer::LibCompile(const TStrObjList& Params, TMacroData& E) {
  Compile(Params[0].ToBool());
}
//..............................................................................
void TGlRenderer::LibPerspective(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  if (Cmds.IsEmpty()) { EnablePerspective(false);  return; }
  if (!Cmds[0].IsNumber()) {
    E.ProcessingError(__OlxSrcInfo, "please specify a number in range [1-90]");
    return;
  }
  double v = Cmds[0].ToDouble();
  if (v < 0.5) {
    v = 1;
  }
  if (v > 180) {
    v = 180;
  }

  SetPerspectiveAngle(v);
  EnablePerspective(true);
}
//..............................................................................
void TGlRenderer::LibFog(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  if (Cmds.Count() == 1) {
    SetFogType(GL_LINEAR);
    SetFogStart(2.0f);
    SetFogEnd(3.3f);
    SetFogColor(Cmds[0].SafeUInt<uint32_t>());
    EnableFog(true);
  }
  else if (Cmds.Count() == 3) {
    SetFogType(GL_LINEAR);
    SetFogStart(Cmds[1].ToFloat());
    SetFogEnd(Cmds[2].ToFloat());
    SetFogColor(Cmds[0].SafeUInt<uint32_t>());
    EnableFog(true);
  }
  else {
    EnableFog(false);
  }
}
//..............................................................................
void TGlRenderer::LibZoom(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  olxstr wv = Options.FindValue("wheel");
  if (!wv.IsEmpty()) {
    double df = 300;
    if (Options.GetBoolOption('a')) {
      df /= (CalcZoom() * 4);
    }
    SetZoom(GetZoom() + wv.ToDouble() / df);
    TBasicApp::GetInstance().Update();
    return;
  }
  if (Cmds.IsEmpty()) {
    SetZoom(CalcZoom());
    return;
  }
  bool absolute = Options.GetBoolOption('a');
  if (absolute) {
    SetZoom(Cmds[0].ToDouble());
  }
  else {
    double zoom = GetZoom() + Cmds[0].ToDouble();
    if (zoom < 0.001) {
      zoom = 0.001;
    }
    SetZoom(zoom);
  }
}
//..............................................................................
void TGlRenderer::LibCalcZoom(const TStrObjList& Params, TMacroData& E) {
  E.SetRetVal(
    CalcZoom(Params.IsEmpty() ? false : Params[0].ToBool())
  );
}
//..............................................................................
void TGlRenderer::LibGetZoom(const TStrObjList& Params, TMacroData& E) {
  E.SetRetVal(GetZoom());
}
//..............................................................................
void TGlRenderer::LibStereo(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    if (StereoFlag == glStereoColor) {
      E.SetRetVal<olxstr>("color");
    }
    else if (StereoFlag == glStereoCross) {
      E.SetRetVal<olxstr>("cross");
    }
    else if (StereoFlag == glStereoAnaglyph) {
      E.SetRetVal<olxstr>("anaglyph");
    }
    else if (StereoFlag == glStereoHardware) {
      E.SetRetVal<olxstr>("hardware");
    }
    else {
      E.SetRetVal<olxstr>("none");
    }
  }
  else {
    if (OWidth != 0) {
      Width = OWidth;
      OWidth = 0;
    }
    if (Params[0].Equalsi("color")) {
      StereoFlag = glStereoColor;
    }
    else if (Params[0].Equalsi("anaglyph")) {
      GLint bits = 0;
      olx_gl::get(GL_ACCUM_RED_BITS, &bits);
      if (bits == 0) {
        TBasicApp::NewLogEntry(logError) <<
          "Sorry accumulation buffer is not initialised/available";
      }
      else {
        StereoFlag = glStereoAnaglyph;
      }
    }
    else if (Params[0].Equalsi("interlace")) {
      GLint bits = 0;
      olx_gl::get(GL_STENCIL_BITS, &bits);
      if (bits == 0) {
        TBasicApp::NewLogEntry(logError) <<
          "Sorry stencil buffer is not initialised/available";
      }
      else {
        StereoFlag = glStereoInterlace;
      }
    }
    else if (Params[0].Equalsi("cross")) {
      olx_gl::clearColor(LightModel.GetClearColor().Data());
      OWidth = Width;
      Width /= 2;
      StereoFlag = glStereoCross;
    }
    else if (Params[0].Equalsi("hardware")) {
      GLboolean stereo_supported = GL_FALSE;
      olx_gl::get(GL_STEREO, &stereo_supported);
      if (stereo_supported == GL_FALSE) {
        TBasicApp::NewLogEntry(logError) <<
          "Sorry stereo buffers are not initialised/available";
      }
      else {
        olx_gl::clearColor(LightModel.GetClearColor().Data());
        StereoFlag = glStereoHardware;
      }
    }
    else {
      olx_gl::clearColor(LightModel.GetClearColor().Data());
      StereoFlag = 0;
    }
    if (Params.Count() == 2) {
      StereoAngle = Params[1].ToDouble();
    }
  }
  TGraphicsStyle& gs = Styles.NewStyle("GL.Stereo", true);
  gs.SetParam("angle", StereoAngle, true);
}
//..............................................................................
void TGlRenderer::LibStereoColor(const TStrObjList& Params, TMacroData& E) {
  TGlOption* glo = Params[0].Equalsi("left") ? &StereoLeftColor :
    (Params[0].Equalsi("right") ? &StereoRightColor : 0);
  if (glo == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "undefined parameter, left/right is expected");
    return;
  }
  if (Params.Count() == 1) {
    E.SetRetVal(glo->ToString());
  }
  if (Params.Count() == 2) {
    *glo = Params[1].SafeUInt<uint32_t>();
    (*glo)[3] = 1;
  }
  else if (Params.Count() == 4) {
    (*glo)[0] = Params[1].ToFloat();
    (*glo)[1] = Params[2].ToFloat();
    (*glo)[2] = Params[3].ToFloat();
    (*glo)[3] = 1;
  }
  TGraphicsStyle& gs = Styles.NewStyle("GL.Stereo", true);
  gs.SetParam("left", StereoLeftColor.ToString(), true);
  gs.SetParam("right", StereoRightColor.ToString(), true);
}
//..............................................................................
void TGlRenderer::LibLineWidth(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    E.SetRetVal(GetLineWidth());
  }
  else {
    SetLineWidth(Params[0].ToDouble());
  }
}
//..............................................................................
void LibPointSize(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    GLdouble ps = 0;
    olx_gl::get(GL_POINT_SIZE, &ps);
    E.SetRetVal(olxstr(ps) << ',' << olx_gl::isEnabled(GL_POINT_SMOOTH));
  }
  else {
    olx_gl::pointSize(Params[0].ToDouble());
    if (Params.Count() > 1) {
      if (Params[1].ToBool()) {
        olx_gl::enable(GL_POINT_SMOOTH);
      }
      else {
        olx_gl::disable(GL_POINT_SMOOTH);
      }
    }
  }
}
//..............................................................................
void TGlRenderer::LibBasis(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    TDataItem di(0, EmptyString());
    TEStrBuffer out;
    GetBasis().ToDataItem(di);
    di.SaveToStrBuffer(out);
    E.SetRetVal(out.ToString());
  }
  else {
    TDataItem di(0, EmptyString());
    di.LoadFromString(0, Params[0], 0);
    GetBasis().FromDataItem(di);
  }
}
//..............................................................................
void TGlRenderer::LibRasterZ(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    E.SetRetVal(GetMaxRasterZ());
  }
  else {
    MaxRasterZ = Params[0].ToDouble();
  }
}
//..............................................................................
TLibrary*  TGlRenderer::ExportLibrary(const olxstr& name) {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("gl") : name);
  lib->Register(
    new TFunction<TGlRenderer>(this, &TGlRenderer::LibCompile, "Compile",
      fpOne,
      "Compiles or decompiles the model according to the boolean parameter")
  );
  lib->Register(
    new TMacro<TGlRenderer>(this, &TGlRenderer::LibPerspective, "Perspective",
      EmptyString(), fpNone | fpOne,
      "Un/Sets perspective view")
  );
  lib->Register(
    new TMacro<TGlRenderer>(this, &TGlRenderer::LibFog, "Fog",
      EmptyString(), fpNone | fpOne | fpThree,
      "Sets fog color, fog without arguments removes fog")
  );
  lib->Register(
    new TMacro<TGlRenderer>(this, &TGlRenderer::LibZoom, "Zoom",
      "a-[false] set absolute zoom value vs. relative&;"
      "wheel-use mouse wheel to set zoom",
      fpNone | fpOne | fpTwo,
      "If no arguments provided - resets zoom to fit to screen, otherwise "
      "increments/decrements current zoom by provided value (default) or "
      "sets absolute according to the -a option.")
  );
  lib->Register(
    new TFunction<TGlRenderer>(this, &TGlRenderer::LibCalcZoom, "CalcZoom",
      fpNone|fpOne, "Returns optimal zoom value. Takes optional bool value to "
      "specify if to calculate zoom for the selection rather than the whole scene")
  );
  lib->Register(
    new TFunction<TGlRenderer>(this, &TGlRenderer::LibGetZoom, "GetZoom",
      fpNone, "Returns current zoom value")
  );
  lib->Register(
    new TFunction<TGlRenderer>(this, &TGlRenderer::LibStereo, "Stereo",
      fpNone | fpOne | fpTwo,
      "Returns/sets color/cross/anaglyph/hardware stereo mode and optionally "
      "stereo angle [3]")
  );
  lib->Register(
    new TFunction<TGlRenderer>(this, &TGlRenderer::LibStereoColor,
      "StereoColor",
      fpOne | fpTwo | fpFour,
      "Returns/sets colors for left/right color stereo mode glasses")
  );
  lib->Register(
    new TFunction<TGlRenderer>(this, &TGlRenderer::LibLineWidth,
      "LineWidth",
      fpNone | fpOne,
      "Returns/sets width of the raster OpenGl line")
  );
  lib->Register(
    new TStaticFunction(&LibPointSize,
      "PointSize",
      fpNone | fpOne | fpTwo,
      "Returns/sets point size")
  );
  lib->Register(
    new TFunction<TGlRenderer>(this, &TGlRenderer::LibBasis,
      "Basis",
      fpNone | fpOne,
      "Returns/sets view basis")
  );
  lib->Register(
    new TFunction<TGlRenderer>(this, &TGlRenderer::LibRasterZ,
      "RasterZ",
      fpNone | fpOne,
      "Returns/sets maximum value of the raster Z 1 or -1 is typically "
      "expected")
  );
  lib->AttachLibrary(LightModel.ExportLibrary("lm"));
  lib->AttachLibrary(GetScene().ExportLibrary("scene"));
  return lib;
}
//..............................................................................
