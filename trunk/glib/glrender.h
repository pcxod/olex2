#ifndef __olx_gl_render_H
#define __olx_gl_render_H
#include "glbase.h"
#include "groupobj.h"
#include "typelist.h"
#include "gloption.h"
#include "gllightmodel.h"
#include "glmaterial.h"
#include "ebasis.h"
#include "actions.h"
#include "threex3.h"
#include "library.h"

#include "tptrlist.h"
#include "talist.h"
#include "macroerror.h"

#include "paramlist.h"
#include "gpcollection.h"
// sorted pointer list should not give any performace boost...
//#include "sptrlist.h"

#undef DrawText
#undef GetObject

BeginGlNamespace()

const uint8_t
  glStereoColor = 0x0001,
  glStereoCross = 0x0002,
  glStereoAnaglyph = 0x0003;

class AGDrawObject;
class TGlGroup;

class TGlListManager  {
  GLuint FInc, FPos;
  TArrayList<GLuint> Lists;
public:
  TGlListManager();
  virtual ~TGlListManager();
  GLuint NewList();
  void ClearLists();
  void ReserveRange(unsigned int count);
  GLuint Count() const {  return FPos; }
  GLuint GetList(size_t index) const {  return Lists[index]; }
};

class TGlRenderer : public IEObject  {
  ObjectGroup<TGlMaterial, TGlPrimitive>  Primitives;  // a list of all groups of primitives
  TSStrPObjList<olxcstr,TGPCollection*, false> FCollections;
//  TPtrList<class TGPCollection> FCollections; // a named list of collections (TGPCollection)
//  TSPtrList<TGlMaterial> FTransluentObjects, FIdentityObjects, FTransluentIdentityObjects;
  TPtrList<TGlMaterial> FTransluentObjects, FIdentityObjects, FTransluentIdentityObjects;
  TPtrList<AGDrawObject> FGObjects;
  TPtrList<TGlGroup> FGroups;   // list of groups
  TGlGroup* FSelection;  // list of selected objects
  class TTextureManager* TextureManager;
  bool FSceneComplete;

//__________________ perspective related stuff
  bool FPerspective;
  float FPAngle,
/* zoom is used when drawn on a larger canvas, so that objects, which are draw in identity mode can fix their position */
    FZoom,
/* viewzoom it to be used with rasters when the scene is zoomed using LookAt function */
    FViewZoom,
    FProjectionLeft, FProjectionRight, FProjectionTop, FProjectionBottom 
    ;
//__________________Fog stuff
  bool Fog;
  int FogType;
  TGlOption FogColor;
  float FogDensity, FogStart, FogEnd;
//__________________
  int FWidth, FHeight, FLeft, FTop, FOWidth;
  TGlListManager FListManager;
  int CompiledListId;
protected:
  void DrawObjects(int x, int y, bool SelectObjects, bool SelectPrimitives);

  vec3d FMaxV, FMinV;
  bool Changed;
  TEBasis FBasis;
  class AGlScene *FScene; // system dependent staff

  static class TGraphicsStyles* FStyles;
  class TGlBackground *FBackground, *FCeiling;
  bool FGlImageChanged; // true if DrawMethod was used
  char *FGlImage;
  int FGlImageHeight, FGlImageWidth;
  uint8_t StereoFlag;
  double StereoAngle;
  TGlOption StereoLeftColor, StereoRightColor;
  mutable double SceneDepth;
  bool ATI;
public:
  TGlRenderer(AGlScene *S, int width, int height);
  virtual ~TGlRenderer();
  void Clear();
  void ClearPrimitives();
  AGlScene& GetScene()  const {  return *FScene; }
  void SetView(int x, int y, bool Select = false, short Res = 1);   // the functions set current matrix
  void SetView(short Res=1); // is used to set current view (when initialisation is done by an external librray
  // such as wxWidgets

  TGraphicsStyles& GetStyles()    const {  return *FStyles; }
  void CleanUpStyles(); // removes styles, which are not used by any collection
  void _OnStylesClear(); // is called by the FStyles only!
  void _OnStylesLoaded(); // is called by the FStyles only!

  DefPropBIsSet(Changed)

  bool IsCompiled()            const {  return CompiledListId != -1; }
  void Compile(bool v);

  // if true, then min/max are not updated for newly created objects
  bool IsSceneComplete()        const {  return FSceneComplete;  }
  void SetSceneComplete(bool v)       {  FSceneComplete = v;  }
  // basis manipulation
  TEBasis& GetBasis() {  return FBasis; }
  const TEBasis& GetBasis() const {  return FBasis; }
  void SetBasis(const TEBasis &B)  {  FBasis = B; }
  template <class VC> void Translate(const VC& V)  {  FBasis.Translate(V); };
  void TranslateX(double V)  {  FBasis.TranslateX(V);  }
  void TranslateY(double V)  {  FBasis.TranslateY(V);  }
  void TranslateZ(double V)  {  FBasis.TranslateZ(V);  }
  void RotateX(double V)  {  FBasis.RotateX(V); }
  void RotateY(double V)  {  FBasis.RotateY(V); }
  void RotateZ(double V)  {  FBasis.RotateZ(V); }
  double GetZoom() const {  return FBasis.GetZoom(); }
  void  SetZoom(double V);
  void ResetBasis()  {  FBasis.Reset();  }
  
  TGlLightModel LightModel;
  // register your handler to swap buffers etc
  TActionQueue *OnDraw, *BeforeDraw;
  TActionQueue *OnStylesClear;  // Enter and Exit are called

  void Resize(int h, int w);
  void Resize(int Left, int Top, int h, int w, float Zoom);
// perspective stuff
  void EnablePerspective(bool Set);
  bool IsPerspectiveEnabled() const {  return FPerspective; }
  void SetPerspectiveAngle(double Angle);
  //fog stuff
  void EnableFog(bool Set);
  bool IsFogEnabled() const {  return Fog; }
  DefPropP(float, FogStart)
  DefPropP(float, FogEnd)
  DefPropP(int, FogType)
  DefPropP(float, FogDensity)
  DefPropC(TGlOption, FogColor)

  float GetExtraZoom() const {  return FZoom;  }
  bool IsColorStereo() const {  return StereoFlag==glStereoColor;  }
  bool IsCrossStereo() const {  return StereoFlag==glStereoCross;  }
  bool IsAnaglyphStereo() const {  return StereoFlag==glStereoAnaglyph;  }
  
  void Initialise();
  void InitLights();
  double CalcZoom() const { 
    const double df = SceneDepth < 0 ? (SceneDepth=olx_max(FMaxV.DistanceTo(FMinV), 1.0)) : SceneDepth;
    return 1./df;
  }
  double GetScale() const {  // to be used to calculate raster positions (x,y)
    return 1.0/(GetBasis().GetZoom()*FHeight);
  }

  double GetMaxRasterZ() const {  // to be used to calculate raster positions (z)
    return 1.0/FBasis.GetZoom();
  }
  /* this function provides extra value for use with rasters, when the scene is zoomed
  using LookAt function
  */
  double GetViewZoom() const {  return FViewZoom;  }
  // returns a "size of a pixel in currnet viewport"; use it to transform screen coor
  //dinates to internal coordinates of OpenGl Scene like follow: if an object has to
  //follow mouse pointer, then the change in coordinates should be x = x0+MouseX*GetScale()
  //y = y0+MouseY*GetScale()
  void UpdateMaxMin( const vec3d& Max, const vec3d& Min);
  void ClearMinMax();
  const vec3d& MaxDim() const { return FMaxV; }
  const vec3d& MinDim() const { return FMinV; }
  /* the actual width is to be used for modifying/restoring canvas size */
  int GetActualWidth()  const {  return (StereoFlag==glStereoCross) ? FOWidth : FWidth;  }
  /* opposite to the above one - this is used in all model calcuations */
  int GetWidth()  const {  return FWidth;  }
  int GetHeight() const {  return FHeight;  }
  int GetLeft()   const {  return FLeft;  }
  int GetTop()    const {  return FTop;  }
  void SetBasis(bool Identity = false);
  /*an "advanced" drawing procedure which draws an object on a static image
  . Used to implement faster drawing for such objects as text input controls and frames ...
  Be sure that the object is of "a modal" type, e.g. no significant drawing activity 
  interrupts it otherwise the drawing will be very slow ideed!
  */
  void UpdateGlImage();
  bool GlImageChanged() const {  return FGlImageChanged; }
  void DrawObject(class AGDrawObject *Object=NULL, bool DrawImage = false);
  void ReleaseGlImage();  // use to realease the internal memory allocated by the image

  void Draw();
  AGDrawObject* SelectObject(int x, int y, int depth=0);
  class TGlPrimitive* SelectPrimitive(int x, int y);

  size_t GroupCount() const {  return FGroups.Count(); }
  TGlGroup& GetGroup(size_t i) const {  return *FGroups[i]; }
  TGlGroup& NewGroup(const olxstr& collection_name);
  TGlGroup* FindGroupByName(const olxstr& colName);
  TGlGroup* FindObjectGroup(AGDrawObject& G);
  /* groups current selection and returns the created group object, or NULL
  if current selection had less than 2 elements */
  TGlGroup* GroupSelection(const olxstr& groupName);
  void ClearGroups();
  void UnGroupSelection();
  void UnGroup(TGlGroup& GlG);
  TGlGroup& GetSelection()  const {  return *FSelection; }
  void Select(AGDrawObject& G);
  void DeSelect(AGDrawObject& G);
  void ClearSelection();
  void SelectAll(bool Select);
  void InvertSelection();
  GLuint NewListId()  {  return FListManager.NewList(); }

  void operator = (const TGlRenderer &G);
  TGPCollection& NewCollection(const olxstr &Name);
  TGPCollection& GetCollection(int ind) const {  return *FCollections.GetObject(ind);  }
  template <class T>
  TGPCollection* FindCollection(const T& Name)  {
    size_t ind = FCollections.IndexOfComparable(Name);
    return (ind != InvalidIndex) ? FCollections.GetObject(ind) : NULL;
  }
  template <class T>
  TGPCollection& FindOrCreateCollection(const T& Name)  {
    size_t ind = FCollections.IndexOfComparable(Name);
    return (ind != InvalidIndex) ? *FCollections.GetObject(ind) : NewCollection(Name);
  }

  TGPCollection* FindCollectionX(const olxstr &Name, olxstr& CollName);
  // in comparison with previos function returns first
  // suitable colelction for example will return "C" for "C.C1.1_555"
  // if nor "C.C1" nether "C.C1.1_555" exists
  // if no collection if found, then CollName = "C", otherwise a name
  // of the returned collection
  size_t CollectionCount() const {  return FCollections.Count(); }
  void RemoveCollection(TGPCollection& GP);
  void RemoveCollections(const TPtrList<TGPCollection>& Collections);

  size_t PrimitiveCount() const {  return Primitives.ObjectCount(); }
  TGlPrimitive& GetPrimitive(size_t i) const {  return Primitives.GetObject(i); }
  TGlPrimitive& NewPrimitive(short type); // do not call directly, use GPCollection's method instead
  void RemovePrimitiveByTag(int index);
  void OnSetProperties(const TGlMaterial& P);
  void SetProperties(TGlMaterial& P);  // tracks transluent and identity objects
//  void ReplacePrimitives(TEList *CurObj, TEList *NewObj);

  void SetObjectsCapacity(size_t v)  { FGObjects.SetCapacity(v);  } 
  AGDrawObject& GetObject(size_t i) const {  return *FGObjects[i]; }
  void RemoveObject(AGDrawObject& D)  {  FGObjects.Remove(&D);  }
  void RemoveObjects(const TPtrList<AGDrawObject>& objects);
  void AddObject(AGDrawObject& G);
  size_t ObjectCount() const {  return FGObjects.Count(); }

  TTextureManager& GetTextureManager() const {  return *TextureManager;  }

  // GL interface
  // is used to orient the object (if anisotropic); the function should use
  // provided SetOrientation function
  template <class MC> void GlOrient(const MC& m )  {
    float Bf[4][4];
    Bf[0][0] = (float)m[0][0];  Bf[0][1] = (float)m[0][1];  Bf[0][2] = (float)m[0][2];  Bf[0][3] = 0;
    Bf[1][0] = (float)m[1][0];  Bf[1][1] = (float)m[1][1];  Bf[1][2] = (float)m[1][2];  Bf[1][3] = 0;
    Bf[2][0] = (float)m[2][0];  Bf[2][1] = (float)m[2][1];  Bf[2][2] = (float)m[2][2];  Bf[2][3] = 0;
    Bf[3][0] = 0;  Bf[3][1] = 0;  Bf[3][2] = 0;  Bf[3][3] = 1.0;
    glMultMatrixf(&Bf[0][0]);
  }
  void GlOrient( const TEBasis &b ) const {
    glMultMatrixf( b.GetMData() );
  }
  void LoadIdentity() const {  glLoadIdentity(); }
  // multiplies current matrix by m
  void GlOrient( const float *m )  const {  glMultMatrixf( m );  }

  void GlTranslate( float _x, float _y, float _z) const {
    glTranslatef(_x, _y, _z);
  }
  void GlRotate( float Angle, float _x, float _y, float _z) const {
    glRotatef(Angle, _x, _y, _z);
  }

  template <class VC> void GlTranslate(const VC& trans) const {
    glTranslatef((float)trans[0], (float)trans[1], (float)trans[2]);
  }

  void GlScale( const float _x, const float _y, const float _z) const {
    glScalef(_x, _y, _z);
  }
  // vendor specific, fixes for ATI
  void DrawText(TGlPrimitive& p, double x, double y, double z);
  // avoids clipping the text as well as vendor specific, uses screen coordinates
  void DrawTextSafe(const vec3d& pos, const olxstr& text, const class TGlFont& fnt);
  bool IsATI() const {  return ATI;  }
  void GlScale( const float S ) const {  glScalef(S, S, S);  }
  void EnableClipPlane(class TGlClipPlane *P, bool v);

  TGlBackground* Background()  {  return FBackground; }
  TGlBackground* Ceiling()     {  return FCeiling; }

  char* GetPixels(bool useMalloc=false, short aligment=4);
  /* the functions set current view for drawinf large picture x, y- curent quadraterial,
   res - required resolution; to be used in conjunction with GetPixels
  */
  void LookAt(double x, double y, short res);

  static TGraphicsStyles& _GetStyles();

  void LibCompile(const TStrObjList& Params, TMacroError& E);
  void LibStereo(const TStrObjList& Params, TMacroError& E);
  void LibStereoColor(const TStrObjList& Params, TMacroError& E);
  void LibFog(TStrObjList& Cmds, const TParamList& Options, TMacroError &E);
  void LibPerspective(TStrObjList& Cmds, const TParamList& Options, TMacroError& E);
  void LibZoom(TStrObjList& Cmds, const TParamList& Options, TMacroError& E);
  void LibCalcZoom(const TStrObjList& Params, TMacroError& E);
  TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
};


EndGlNamespace()
#endif
