/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_render_H
#define __olx_gl_render_H
#include "glbase.h"
#include "groupobj.h"
#include "typelist.h"
#include "gloption.h"
#include "gllightmodel.h"
#include "glscene.h"
#include "ebasis.h"
#include "actions.h"
#include "ematrix.h"
#include "threex3.h"
#include "library.h"
#include "tptrlist.h"
#include "talist.h"
#include "macroerror.h"
#include "paramlist.h"
#include "gpcollection.h"
#include "glgroup.h"
#include "styles.h"
#undef DrawText
#undef GetObject
BeginGlNamespace()

const uint8_t
  glStereoColor = 0x0001,
  glStereoCross = 0x0002,
  glStereoAnaglyph = 0x0003,
  glStereoHardware = 0x0004,
  glStereoInterlace = 0x0005,
  glStereoMatrix = 0x0006;

enum glSelectionFlag {
  glSelectionNone, // does nothing
  glSelectionSelect,
  glSelectionUnselect,
  glSelectionInvert
};

class AGDrawObject;
class TGlGroup;
class TGlRenderer;

class AGOSettings : public AActionHandler {
protected:
  TGlRenderer &parent;
  olxstr name;
  TGraphicsStyle *style;
  virtual void OnStyleChange() = 0;;
public:
  AGOSettings(TGlRenderer &p, const olxstr &name);
  virtual ~AGOSettings();
  virtual void OnRendererClear() = 0;
  TGraphicsStyle *GetStyle() const { return style;  }

  template <typename value_t, typename name_t>
  value_t GetParam(const name_t &name, value_t &v, const value_t &def) const {
    if (style == 0) { return def; }
    return v != -1 ? v
      : (v = style->GetNumParam(name, def, true));
  }
  bool Enter(const IOlxObject *, const IOlxObject *, TActionQueue *);
  bool Exit(const IOlxObject *, const IOlxObject *, TActionQueue *);
};

class TGlRenderer : public AActionHandler {
  // a list of all groups of primitives
  ObjectGroup<TGlMaterial, TGlPrimitive>  Primitives;
  olxstr_dict<TGPCollection*, false> FCollections;
  TPtrList<TGlMaterial> FTranslucentObjects,
    FIdentityObjects, FTranslucentIdentityObjects;
  typedef sorted::PointerPointer<AGDrawObject> GObjectList;
  GObjectList FGObjects;
  TPtrList<TGlGroup> FGroups;   // list of groups
  TGlGroup* FSelection;  // list of selected objects
  class TTextureManager* TextureManager;
  bool FSceneComplete, Selecting;
//__________________ perspective related stuff
  bool FPerspective;
  ematd PerspectiveMatrix;
  double FPAngle,
  /* zoom is used when drawn on a larger canvas, so that objects, which are
  draw in identity mode can fix their position
  */
    FZoom,
  /* viewzoom it to be used with rasters when the scene is zoomed using LookAt
  function
  */
    FViewZoom,
    FProjectionLeft, FProjectionRight, FProjectionTop, FProjectionBottom
    ;
//__________________Fog stuff
  bool Fog;
  int FogType;
  TGlOption FogColor;
  float FogDensity, FogStart, FogEnd;
//__________________
  int Top, Left,
    AbsoluteTop; // on srcreen, to handle interlaced stereo
  int Width, Height, OWidth;
  double LineWidth, MaxRasterZ;
  int CompiledListId;
  GLubyte *poly_stipple;
  bool poly_even;
  void SetupStencilFoInterlacedDraw(bool even);
  GLubyte SelectionBuffer[4][4*3];
  olxstr_dict<AGOSettings *> ObjectSettings;
  mat3d StereoMatrix;
  vec3d StereoTranslation;
  bool Enter(const IOlxObject *, const IOlxObject *, TActionQueue *);
  bool Exit(const IOlxObject *, const IOlxObject *, TActionQueue *);
protected:
  void DrawObjects(int x, int y, bool SelectObjects, bool SelectPrimitives);

  vec3d FMaxV, FMinV;
  TEBasis FBasis;
  class AGlScene *FScene;

  TGraphicsStyles Styles;
  class TGlBackground *FBackground, *FCeiling;
  bool FGlImageChanged; // true if DrawMethod was used
  char *FGlImage;
  int GlImageHeight, GlImageWidth;
  uint8_t StereoFlag;
  double StereoAngle, NearPlane, FarPlane;
  TGlOption StereoLeftColor, StereoRightColor;
  mutable double SceneDepth;
  bool ATI, GLUSelection;

  class TGlListManager {
    GLuint Inc, Pos;
    TArrayList<GLuint> Lists;
  public:
    TGlListManager(GLuint inc=16) : Inc(inc), Pos(0)  {}
    virtual ~TGlListManager()  {  Clear();  }
    GLuint NewList();
    void Clear();
  } ListManager;
public:
  TGlRenderer(AGlScene *S, size_t width, size_t height);
  virtual ~TGlRenderer();
  void Clear();
  void ClearObjects();
  AGlScene& GetScene() const {  return *FScene;  }
  // the functions set current matrix
  void SetView(int x, int y, bool Identity, bool Select, short Res);
  /* is used to set current view (when initialisation is done by an external
  librray such as wxWidgets
  */
  void SetView(bool Identity, short Res=1);

  const TGraphicsStyles& GetStyles() const { return Styles; }
  TGraphicsStyles& GetStyles() { return Styles; }
  // removes styles, which are not used by any collection
  void CleanUpStyles();

  bool IsCompiled() const {  return CompiledListId != -1; }
  void Compile(bool v);

  // if true, then min/max are not updated for newly created objects
  bool IsSceneComplete() const {  return FSceneComplete;  }
  bool IsSelecting() const {  return Selecting;  }
  void SetSceneComplete(bool v) {  FSceneComplete = v;  }
  // basis manipulation
  TEBasis& GetBasis() {  return FBasis; }
  const TEBasis& GetBasis() const {  return FBasis; }
  void SetBasis(const TEBasis &B)  {  FBasis = B; }
  template <class VC> void Translate(const VC& V)  {  FBasis.Translate(V); }
  void TranslateX(double V)  {  FBasis.TranslateX(V);  }
  void TranslateY(double V)  {  FBasis.TranslateY(V);  }
  void TranslateZ(double V)  {  FBasis.TranslateZ(V);  }
  void RotateX(double V)  {  FBasis.RotateX(V); }
  void RotateY(double V)  {  FBasis.RotateY(V); }
  void RotateZ(double V)  {  FBasis.RotateZ(V); }
  double GetZoom() const {  return FBasis.GetZoom(); }
  void SetZoom(double V);
  void ResetBasis()  {  FBasis.Reset();  }

  TGlLightModel LightModel;
  TActionQueue &OnDraw, // register your handler to swap buffers etc
    &OnClear;

  void Resize(size_t w, size_t h);
  void Resize(int Left, int Top, size_t w, size_t h, double Zoom);
// perspective stuff
  void EnablePerspective(bool Set);
  bool IsPerspectiveEnabled() const {  return FPerspective; }
  void SetPerspectiveAngle(double Angle);
  vec3d Project(const vec3d &v) const;
  //fog stuff
  void EnableFog(bool Set);
  bool IsFogEnabled() const {  return Fog; }
  DefPropP(float, FogStart)
  DefPropP(float, FogEnd)
  DefPropP(int, FogType)
  DefPropP(float, FogDensity)
  DefPropC(TGlOption, FogColor)
  DefPropC(mat3d, StereoMatrix)
  DefPropC(vec3d, StereoTranslation)
  DefPropP(int, AbsoluteTop)
  DefPropP(uint8_t, StereoFlag)
  DefPropBIsSet(GLUSelection)
  double GetLineWidth() const {  return LineWidth;  }
  void SetLineWidth(double v);
  double GetExtraZoom() const {  return FZoom;  }
  bool ForcePlain() const { return IsColorStereo() || IsSelecting(); }
  bool IsColorStereo() const {  return StereoFlag==glStereoColor;  }
  bool IsCrossStereo() const {  return StereoFlag==glStereoCross;  }
  bool IsAnaglyphStereo() const {  return StereoFlag==glStereoAnaglyph;  }

  void Initialise();
  void InitLights();
  double CalcZoom() const {
    const double df = SceneDepth < 0 ?
      (SceneDepth=olx_max(FMaxV.DistanceTo(FMinV), 1.0)) : SceneDepth;
    return 1./df;
  }
  /* returns a "size of a pixel in current viewport"; use it to transform
  screen coordinates to internal coordinates of OpenGl Scene like follow: if an
  object has to follow mouse pointer, then the change in coordinates should be
  x = x0+MouseX*GetScale() y = y0+MouseY*GetScale()
  */
  double GetScale() const {  // to be used to calculate raster positions (x,y)
    return 1./Height;
  }
  /* to be used to calculate raster positions (z) */
  double GetMaxRasterZ() const {  return MaxRasterZ;  }
  // calculates the Z value for the given offset
  double CalcRasterZ(double off) const;
  /* this function provides extra value for use with rasters, when the scene is
  zoomed using LookAt function
  */
  double GetViewZoom() const {  return FViewZoom;  }
  void UpdateMinMax(const vec3d& Max, const vec3d& Min);
  void ClearMinMax();
  const vec3d& MaxDim() const { return FMaxV; }
  const vec3d& MinDim() const { return FMinV; }
  /* the actual width is to be used for modifying/restoring canvas size */
  size_t GetActualWidth() const {
    return (StereoFlag==glStereoCross) ? OWidth : Width;
  }
  /* opposite to the above one - this is used in all model calcuations */
  int GetWidth() const {  return Width;  }
  int GetHeight() const {  return Height;  }
  int GetLeft() const {  return Left;  }
  int GetTop() const {  return Top;  }
  /*an "advanced" drawing procedure which draws an object on a static image
  . Used to implement faster drawing for such objects as text input controls
  and frames...  Be sure that the object is of "a modal" type, e.g. no
  significant drawing activity interrupts it otherwise the drawing will be very
  slow ideed!
  */
  void UpdateGlImage();
  bool GlImageChanged() const {  return FGlImageChanged; }
  // use to realease the internal memory allocated by the image
  void ReleaseGlImage();

  void Draw();
  void DrawSilhouette();
  // to be called if the underlying OpenGl Context is about to change
  void BeforeContextChange();
  // to be called if the underlying OpenGl Context has changed
  void AfterContextChange();
  // this is to be used by groups
  void HandleSelection(const AGDrawObject &o, const TGlPrimitive &p,
    bool SelectObjects, bool SelectPrimitives) const;
  AGDrawObject* SelectObject(int x, int y);
  TGlPrimitive* SelectPrimitive(int x, int y);

  size_t GroupCount() const {  return FGroups.Count();  }
  TGlGroup& GetGroup(size_t i) const {  return *FGroups[i];  }
  const TPtrList<TGlGroup>& GetGroups() const {  return FGroups;  }
  TGlGroup& NewGroup(const olxstr& collection_name);
  TGlGroup* FindGroupByName(const olxstr& colName) const;
  TGlGroup* FindObjectGroup(const AGDrawObject& G) const;
  /* groups current selection and returns the created group object, or NULL
  if current selection had less than 2 elements */
  TGlGroup* GroupSelection(const olxstr& groupName);
  /*
  clears the groups, if clean is specified - it makse sure that the groups are
  removed from the Object list.
  */
  void ClearGroups(bool clean);
  void Ungroup(TGlGroup& GlG);
  TGlGroup& GetSelection() const {  return *FSelection; }
  template <class SelType> SelType& ReplaceSelection() {
    FSelection->GetPrimitives().RemoveObject(*FSelection);
    FSelection->SetSelected(false);
    FGObjects.Remove(FSelection);
    delete FSelection;
    SelType* sel = new SelType(*this, "Selection");
    FSelection = sel;
    FSelection->Create();
    return *sel;
  }
  void Select(AGDrawObject& G, bool v); // convinience function...
  void Select(AGDrawObject& G, glSelectionFlag flag);
  void Select(AGDrawObject& G);
  void Deselect(AGDrawObject& G);
  void ClearSelection();
  void SelectAll(bool Select);
  void InvertSelection();
  GLuint NewListId()  {  return ListManager.NewList();  }

  void operator = (const TGlRenderer &G);
  TGPCollection& NewCollection(const olxstr &Name);
  TGPCollection& GetCollection(size_t ind) const {
    return *FCollections.GetValue(ind);
  }
  template <class T>
  TGPCollection* FindCollection(const T& Name)  {
    const size_t ind = FCollections.IndexOf(Name);
    return (ind != InvalidIndex) ? FCollections.GetValue(ind) : NULL;
  }
  template <class T>
  TGPCollection& FindOrCreateCollection(const T& Name)  {
    const size_t ind = FCollections.IndexOf(Name);
    return (ind != InvalidIndex) ? *FCollections.GetValue(ind)
      : NewCollection(Name);
  }

  /* in comparison with previos function returns first
   suitable colelction for example will return "C" for "C.C1.1_555"
   if nor "C.C1" nether "C.C1.1_555" exists
   if no collection if found, then CollName = "C", otherwise a name
   of the returned collection
   */
  TGPCollection* FindCollectionX(const olxstr &Name, olxstr& CollName);

  size_t CollectionCount() const {  return FCollections.Count(); }
  void RemoveCollection(TGPCollection& GP);
  void RemoveCollections(const TPtrList<TGPCollection>& Collections);

  size_t PrimitiveCount() const {  return Primitives.ObjectCount(); }
  TGlPrimitive& GetPrimitive(size_t i) const {
    return Primitives.GetObject(i);
  }
  // do not call directly, use GPCollection's method instead
  TGlPrimitive& NewPrimitive(short type);
  void RemovePrimitiveByTag(int index);
  void OnSetProperties(const TGlMaterial& P);
  // tracks translucent and identity objects
  void SetProperties(TGlMaterial& P);

  void SetObjectsCapacity(size_t v)  { FGObjects.SetCapacity(v);  }
  AGDrawObject& GetObject(size_t i) const {  return *FGObjects[i]; }
  void RemoveObject(AGDrawObject& D)  {  FGObjects.Remove(&D);  }
  void RemoveObjects(const AGDObjList& objects);
  void AddObject(AGDrawObject& G);
  size_t ObjectCount() const {  return FGObjects.Count(); }

  TTextureManager& GetTextureManager() const {  return *TextureManager;  }

  // vendor specific, fixes for ATI
  void DrawText(TGlPrimitive& p, double x, double y, double z);
  /* avoids clipping the text as well as vendor specific, uses screen
  coordinates
  */
  void DrawTextSafe(const vec3d& pos, const olxstr& text, const TGlFont& fnt);
  bool IsATI() const {  return ATI;  }
  void EnableClipPlane(class TGlClipPlane *P, bool v);

  TGlBackground* Background()  {  return FBackground; }
  TGlBackground* Ceiling()     {  return FCeiling; }
  /* returns 24 bit colors, formatis GL_RGB or GL_BGR_EXT */
  char* GetPixels(bool useMalloc=false, short aligment=4, GLuint format=GL_RGB);
  /* the functions set current view for drawinf large picture x, y- curent
  quadraterial, res - required resolution; to be used in conjunction with
  GetPixels
  */
  void LookAt(double x, double y, short res);

  template <class n_t>
  AGOSettings *FindSettings(const n_t &name) {
    return ObjectSettings.Find(name, NULL);
  }

  AGOSettings &RegisterSettings(AGOSettings &s, const olxstr &name) {
    return *ObjectSettings.Add(name, &s);
  }

  TEBitArray::const_type GetVisibility();
  void SetVisibility(const TEBitArray &v);

  void LibCompile(const TStrObjList& Params, TMacroData& E);
  void LibStereo(const TStrObjList& Params, TMacroData& E);
  void LibStereoColor(const TStrObjList& Params, TMacroData& E);
  void LibFog(TStrObjList& Cmds, const TParamList& Options, TMacroData &E);
  void LibPerspective(TStrObjList& Cmds, const TParamList& Options,
    TMacroData& E);
  void LibZoom(TStrObjList& Cmds, const TParamList& Options, TMacroData& E);
  void LibCalcZoom(const TStrObjList& Params, TMacroData& E);
  void LibGetZoom(const TStrObjList& Params, TMacroData& E);
  void LibLineWidth(const TStrObjList& Params, TMacroData& E);
  void LibBasis(const TStrObjList& Params, TMacroData& E);
  void LibRasterZ(const TStrObjList& Params, TMacroData& E);
  TLibrary* ExportLibrary(const olxstr& name=EmptyString());
};

EndGlNamespace()
#endif
