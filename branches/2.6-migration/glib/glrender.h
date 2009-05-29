//---------------------------------------------------------------------------

#ifndef glrenderH
#define glrenderH

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

class AGDrawObject;
class TGlGroup;

class TGlListManager  {
  int FInc, FPos;
  TIntList Lists;
public:
  TGlListManager();
  virtual ~TGlListManager();
  int NewList();
  void ClearLists();
  void ReserveRange(int count);
  inline int Count() const {  return FPos; }
  inline int GetList(int index) const {  return Lists[index]; }
};

class TGlRenderer : public IEObject  {
  ObjectGroup<TGlMaterial, TGlPrimitive>  Primitives;  // a list of all groups of primitives
  TSStrPObjList<CString,TGPCollection*, false> FCollections;
//  TPtrList<class TGPCollection> FCollections; // a named list of collections (TGPCollection)
//  TSPtrList<TGlMaterial> FTransluentObjects, FIdentityObjects, FTransluentIdentityObjects;
  TPtrList<TGlMaterial> FTransluentObjects, FIdentityObjects, FTransluentIdentityObjects;
  TPtrList<AGDrawObject> FGObjects;
  TPtrList<TGlGroup> FGroups;   // list of groups
  TGlGroup* FSelection;  // list of selected objects
  class TTextureManager* TextureManager;
  bool FSceneComplete;
  void SetBasis(bool Identity = false);

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
  bool FFog;
  int FFogType;
  TGlOption FFogColor;
  float FFogDensity, FFogStart, FFogEnd;
//__________________

  int FWidth, FHeight, FLeft, FTop;
  TGlListManager FListManager;
  int CompiledListId;
protected:
  void DrawObjects( bool SelectPrimitives, bool SelectObjects);

  vec3d FMaxV, FMinV;
  bool Changed;
  TEBasis *FBasis;
  class AGlScene *FScene; // system dependent staff

  static class TGraphicsStyles* FStyles;
  class TGlBackground *FBackground, *FCeiling;
  bool FGlImageChanged; // true if DrawMethod was used
  char *FGlImage;
  int FGlImageHeight, FGlImageWidth;
  TActionQList *FActions;
  mutable double CalculatedZoom;
  bool ATI;
public:
  TGlRenderer(AGlScene *S, int width, int height);
  virtual ~TGlRenderer();
  void Clear();
  void ClearPrimitives();
  inline AGlScene& GetScene()  const {  return *FScene; }
  void SetView(int x, int y, bool Select = false, short Res = 1);   // the functions set current matrix
  void SetView(short Res=1); // is used to set current view (when initialisation is done by an external librray
  // such as wxWidgets

  inline TGraphicsStyles& GetStyles()    const {  return *FStyles; }
  void CleanUpStyles(); // removes styles, which are not used by any collection
  void _OnStylesClear(); // is called by the FStyles only!
  void _OnStylesLoaded(); // is called by the FStyles only!

  DefPropB(Changed)

  inline bool IsCompiled()            const {  return CompiledListId != -1; }
  void Compile(bool v);

  // if true, then min/max are not updated for newly created objects
  inline bool IsSceneComplete()        const {  return FSceneComplete;  }
  inline void SetSceneComplete(bool v)       {  FSceneComplete = v;  }
  // basis manipulation
  inline TEBasis& GetBasis()                 {  return *FBasis; }
  inline const TEBasis& GetBasis()     const {  return *FBasis; }
  inline void SetBasis( const TEBasis &B)    {  *FBasis = B; }
  template <class VC>
    inline void Translate(const VC& V)       {  FBasis->Translate(V); };
  inline void TranslateX(double V)           {  FBasis->TranslateX(V);  }
  inline void TranslateY(double V)           {  FBasis->TranslateY(V);  }
  inline void TranslateZ(double V)           {  FBasis->TranslateZ(V);  }
  inline void RotateX(double V)              {  FBasis->RotateX(V); }
  inline void RotateY(double V)              {  FBasis->RotateY(V); }
  inline void RotateZ(double V)              {  FBasis->RotateZ(V); }
  inline double GetZoom()              const {  return FBasis->GetZoom(); }
  void  SetZoom(double V);
  inline void ResetBasis()                   {  FBasis->Reset(); }
  
  TGlLightModel LightModel;
  // register your handler to swap buffers etc
  TActionQueue *OnDraw, *BeforeDraw;
  TActionQueue *OnStylesClear;  // Enter and Exit are called

  void Resize(int h, int w);
  void Resize(int Left, int Top, int h, int w, float Zoom);
// perspective stuff
  void EnablePerspective(bool Set);
  bool IsPerspectiveEnabled()           const {  return FPerspective; }
  void SetPerspectiveAngle(double Angle);
  //fog stuff
  void EnableFog(bool Set);
  inline bool IsFogEnabled()            const {  return FFog; }
  inline void SetFogStart(float v)            {  FFogStart = v;  }
  inline float GetFogStart()            const {  return FFogStart;  }
  inline void SetFogEnd(float v)              {  FFogEnd = v;  }
  inline float GetFogEnd()              const {  return FFogEnd;  }
  inline void SetFogType(int v)               {  FFogType = v;  }
  inline int GetFogType()               const {  return FFogType;  }
  inline void SetFogDensity(float v)          {  FFogDensity = v;  }
  inline float GetFogDensity()          const {  return FFogDensity;  }
  inline void SetFogColor(int Cl)             {  FFogColor = Cl;  }
  inline void SetFogColor(const TGlOption& Cl){  FFogColor = Cl;  }
  inline TGlOption& FogColor()                {  return FFogColor;  }
  inline const TGlOption& GetFogColor() const {  return FFogColor;  }

  inline float GetExtraZoom()           const {  return FZoom;  }

  void Initialise();
  void InitLights();
  inline double CalcZoom() const { 
    return CalculatedZoom < 0 ? (CalculatedZoom=olx_max(FMaxV.DistanceTo(FMinV), 1.0)) : CalculatedZoom;
  }
  inline double GetScale() const {  // to be used to calculate raster positions (x,y)
    double df = CalcZoom();
    return (df*df)/(GetBasis().GetZoom()*FHeight);
  }

  inline double GetMaxRasterZ() const {  // to be used to calculate raster positions (z)
    double df = CalcZoom();
    return (df*df)/FBasis->GetZoom()-1;
  }
  /* this function provides extra value for use with rasters, when the scene is zoomed
  using LookAt function
  */
  inline double GetViewZoom()  const  {  return FViewZoom;  }
  // returns a "size of a pixel in currnet viewport"; use it to transform screen coor
  //dinates to internal coordinates of OpenGl Scene like follow: if an object has to
  //follow mouse pointer, then the change in coordinates should be x = x0+MouseX*GetScale()
  //y = y0+MouseY*GetScale()
  void UpdateMaxMin( const vec3d& Max, const vec3d& Min);
  void ClearMinMax();
  const vec3d& MaxDim() const { return FMaxV; }
  const vec3d& MinDim() const { return FMinV; }
  // Scene.Initialise must be called before to initialise drawing
  // contexts
  inline int GetWidth()  const {  return FWidth;  }
  inline int GetHeight() const {  return FHeight;  }
  inline int GetLeft()   const {  return FLeft;  }
  inline int GetTop()    const {  return FTop;  }
  /*an "advanced" drawing procedure which draws an object on a static image
  . Used to implement faster drawing for such objects as text input controls and frames ...
  Be sure that the object is of "a modal" type, e.g. no significant drawing activity 
  interrupts it otherwise the drawing will be very slow ideed!
  */
  void UpdateGlImage();
  inline bool GlImageChanged() const {  return FGlImageChanged; }
  void DrawObject(class AGDrawObject *Object=NULL, bool DrawImage = false);
  void ReleaseGlImage();  // use to realease the internal memory allocated by the image

  void Draw();
  AGDrawObject* SelectObject(int x, int y, int depth=0);
  class TGlPrimitive* SelectPrimitive(int x, int y);

  inline int GroupCount() const {  return FGroups.Count(); }
  inline TGlGroup& GetGroup(int i) const {  return *FGroups[i]; }
  TGlGroup& NewGroup(const olxstr& collection_name);
  TGlGroup* FindGroupByName(const olxstr& colName);
  TGlGroup* FindObjectGroup(AGDrawObject& G);
  /* groups current selection and returns the created group object, or NULL
  if current selection had less than 2 elements */
  TGlGroup* GroupSelection(const olxstr& groupName);
  void ClearGroups();
  void UnGroupSelection();
  void UnGroup(TGlGroup& GlG);
  inline TGlGroup& GetSelection()  const {  return *FSelection; }
  void Select(AGDrawObject& G);
  void DeSelect(AGDrawObject& G);
  void ClearSelection();
  void SelectAll(bool Select);
  void InvertSelection();
  inline int NewListId()  {  return FListManager.NewList(); }

  void operator = (const TGlRenderer &G);
  TGPCollection& NewCollection(const olxstr &Name);
  TGPCollection& GetCollection(int ind) const {  return *FCollections.GetObject(ind);  }
  template <class T>
  TGPCollection* FindCollection(const T& Name)  {
    int ind = FCollections.IndexOfComparable(Name);
    return (ind != -1) ? FCollections.GetObject(ind) : NULL;
  }
  template <class T>
  TGPCollection& FindOrCreateCollection(const T& Name)  {
    int ind = FCollections.IndexOfComparable(Name);
    return (ind != -1) ? *FCollections.GetObject(ind) : NewCollection(Name);
  }

  TGPCollection* FindCollectionX(const olxstr &Name, olxstr& CollName);
  // in comparison with previos function returns first
  // suitable colelction for example will return "C" for "C.C1.1_555"
  // if nor "C.C1" nether "C.C1.1_555" exists
  // if no collection if found, then CollName = "C", otherwise a name
  // of the returned collection
  inline int CollectionCount() const {  return FCollections.Count(); }
  void RemoveCollection(TGPCollection& GP);
  void RemoveCollections(const TPtrList<TGPCollection>& Collections);

  inline int PrimitiveCount() const {  return Primitives.ObjectCount(); }
  TGlPrimitive& GetPrimitive(int i) const {  return Primitives.GetObject(i); }
  TGlPrimitive& NewPrimitive(short type); // do not call directly, use GPCollection's method instead
  void RemovePrimitive(int in);
  void OnSetProperties(const TGlMaterial& P);
  void SetProperties(TGlMaterial& P);  // tracks transluent and identity objects
//  void ReplacePrimitives(TEList *CurObj, TEList *NewObj);

  inline void SetObjectsCapacity(int v)        { FGObjects.SetCapacity(v);  } 
  inline AGDrawObject& GetObject( int i) const {  return *FGObjects[i]; }
  inline void RemoveObject(AGDrawObject& D)    {  FGObjects.Remove(&D);  }
  void RemoveObjects(const TPtrList<AGDrawObject>& objects);
  void AddObject(AGDrawObject& G);
  inline int ObjectCount() const {  return FGObjects.Count(); }

  inline TTextureManager& GetTextureManager() const {  return *TextureManager;  }

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
  inline void GlOrient( const TEBasis &b ) const {
    glMultMatrixf( b.GetMData() );
  }
  inline void LoadIdentity() const {  glLoadIdentity(); }
  // multiplies current matrix by m
  inline void GlOrient( const float *m )  const {  glMultMatrixf( m );  }

  inline void GlTranslate( float _x, float _y, float _z) const {
    glTranslatef(_x, _y, _z);
  }
  inline void GlRotate( float Angle, float _x, float _y, float _z) const {
    glRotatef(Angle, _x, _y, _z);
  }

  template <class VC> inline void GlTranslate(const VC& trans) const {
    glTranslatef((float)trans[0], (float)trans[1], (float)trans[2]);
  }

  inline void GlScale( const float _x, const float _y, const float _z) const {
    glScalef(_x, _y, _z);
  }
  // vendor specific, fixes for ATI
  void DrawText(TGlPrimitive& p, double x, double y, double z);
  // avoids clipping the text as well as vendor specific, uses screen coordinates
  void DrawTextSafe(const vec3d& pos, const olxstr& text, const class TGlFont& fnt);
  bool IsATI() const {  return ATI;  }
  inline void GlScale( const float S ) const {  glScalef(S, S, S);  }
  void EnableClipPlane(class TGlClipPlane *P, bool v);

  inline TGlBackground* Background()  {  return FBackground; }
  inline TGlBackground* Ceiling()     {  return FCeiling; }

  char* GetPixels(bool useMalloc=false, short aligment=4);
  /* the functions set current view for drawinf large picture x, y- curent quadraterial,
   res - required resolution; to be used in conjunction with GetPixels
  */
  void LookAt(double x, double y, short res);

  static TGraphicsStyles& _GetStyles();

  void LibCompile(const TStrObjList& Params, TMacroError& E);
  void LibFog(TStrObjList &Cmds, const TParamList &Options, TMacroError &E);
  void LibPerspective(TStrObjList &Cmds, const TParamList &Options, TMacroError &E);
  void LibZoom(TStrObjList &Cmds, const TParamList &Options, TMacroError &E);
  TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
};


EndGlNamespace()
#endif

