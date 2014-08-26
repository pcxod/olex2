/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_xatom_H
#define __olx_gxl_xatom_H
#include "glrender.h"
#include "glprimitive.h"
#include "glmousehandler.h"
#include "gllabel.h"
#include "styles.h"
#include "ellipsoid.h"
#include "satom.h"
BeginGxlNamespace()

class TXBond;

const uint16_t
  adsSphere       = 1,  // atom draw styles
  adsEllipsoid    = 2,
  adsStandalone   = 3,
  adsOrtep        = 4,
  adsSpecial      = 0x1000;

const short
  darPers     = 0x0001, // default atom radii
  darIsot     = 0x0002,
  darPack     = 0x0004,
  darBond     = 0x0008,
  darIsotH    = 0x0010,  // only affects H, others as darIsot
  darVdW      = 0x0020,
  darCustom   = 0x0040;
const short
  polyNone      = 0,
  polyAuto      = 1,  // polyhedron type
  polyRegular   = 2,
  polyPyramid   = 3,
  polyBipyramid = 4;

const uint32_t
  xatom_PolyId        = 1,
  xatom_SphereId      = 2,
  xatom_SmallSphereId = 3,
  xatom_RimsId        = 4,
  xatom_DisksId       = 5,
  xatom_CrossId       = 6,
  xatom_TetrahedronId = 7
  ;

class TXAtom: public TSAtom, public AGlMouseHandlerImp  {
public:
  struct Poly {
    TArrayList<vec3f> vecs;
    TTypeList<vec3f> norms;
    TTypeList<TVector3<size_t> > faces;
  };
private:
  short FDrawStyle, FRadius;
  static uint8_t PolyhedronIndex,
    SphereIndex,
    SmallSphereIndex,
    RimsIndex,
    DisksIndex,
    CrossIndex,
    TetrahedronIndex
    ;
  static GLuint
    OrtepSpheres, // 8 glLists
    LockedAtomSphere, // 1 list
    ConstrainedAtomSphere // 1 list
    ;
  static int16_t QualityValue;
  GLuint ActualSphere;
  // picks the correct sphere for rendering
  void InitActualSphere();
  static float MinQAlpha;
  Poly* Polyhedron;
  TXGlLabel* Label;
  bool label_forced;
  void CreatePolyhedron(bool v);
  // returns the center of the created polyhedron
  vec3f TriangulateType2(Poly& p, const TSAtomPList& atoms);
  void CreateNormals(TXAtom::Poly& pl, const vec3f& cnt);
  void CreatePoly(const TSAtomPList& atoms, short type,
    const vec3d* normal=NULL, const vec3d* center=NULL);
  // returns different names for isotropic and anisotropic atoms
protected:
  TStrList* FindPrimitiveParams(TGlPrimitive *P);
  static TTypeList<TGlPrimitiveParams> FPrimitiveParams;
  void ValidateRadius(TGraphicsStyle& GS);
  void ValidateDS(TGraphicsStyle& GS);
  static void ValidateAtomParams();
  vec3d Center;
  virtual bool DoTranslate(const vec3d& t) {  Center += t;  return true;  }
  virtual bool DoRotate(const vec3d&, double) {  return false;  }
  virtual bool DoZoom(double, bool)  {  return false;  }
  static float FTelpProb, FQPeakScale, FQPeakSizeScale, FDefZoom;
  static short FDefRad, FDefDS, FDefMask;
  static TGraphicsStyle *FAtomParams;
  static olxstr PolyTypeName;
  static TStringToList<olxstr,TGlPrimitive*> FStaticObjects;

  class TStylesClear: public AActionHandler  {
  public:
    TStylesClear(TGlRenderer& Render)  {  Render.OnStylesClear.Add(this);  }
    virtual ~TStylesClear()  {}
    bool Enter(const IEObject *Sender, const IEObject *Data, TActionQueue *);
    bool Exit(const IEObject *Sender, const IEObject *Data, TActionQueue *);
  };
  static TStylesClear *OnStylesClear;
  class TContextClear: public AActionHandler  {
  public:
    TContextClear(TGlRenderer& Render);
    virtual ~TContextClear()  {}
    bool Enter(const IEObject *Sender, const IEObject *Data, TActionQueue *);
    bool Exit(const IEObject *Sender, const IEObject *Data, TActionQueue *);
  };
  static void ClearStaticObjects();
public:
  TXAtom(TNetwork* net, TGlRenderer& Render, const olxstr& collectionName);
  virtual ~TXAtom();
  void Create(const olxstr& cName=EmptyString());
  // multiple inheritance...
  void SetTag(index_t v) {   TSAtom::SetTag(v);  }
  index_t GetTag() const {  return TSAtom::GetTag();  }
  index_t IncTag()  {  return TSAtom::IncTag();  }
  index_t DecTag()  {  return TSAtom::DecTag();  }

  TXGlLabel& GetGlLabel() const {  return *Label;  }
  virtual void Update();
  virtual void UpdateLabel()  {  GetGlLabel().UpdateLabel();  }
  inline TXAtom& Node(size_t i) const {  return (TXAtom&)TSAtom::Node(i); }
  inline TXBond& Bond(size_t i) const {  return (TXBond&)TSAtom::Bond(i); }
  // returns full legend with symm code
  static olxstr GetLegend(const TSAtom& A, const short Level=2);
  // returns level of the given legend (number of '.', basically)
  static uint16_t LegendLevel(const olxstr& legend);
  static olxstr GetLabelLegend(const TSAtom& A);
  // returns full legend for the label. e.g. "Q.Q1"

  static void GetDefSphereMaterial(const TSAtom& A, TGlMaterial &M);
  static void GetDefRimMaterial(const TSAtom& A, TGlMaterial &M);

  static void DefRad(short V);
  static void DefDS(short V);
  static void DefMask(int V);
  static void TelpProb(float V);
  static void DefZoom(float V);

  static short DefRad(); // default radius
  static short DefDS();     // default drawing style
  static int   DefMask();  // default mas for elliptical atoms eith NPD ellipsoid
  static float TelpProb();    // to use with ellipsoids
  static float DefZoom();    // to use with ellipsoids

  static float GetQPeakScale();    // to use with q-peaks
  static void SetQPeakScale(float V);    // to use with q-peaks
  static float GetQPeakSizeScale();    // to use with q-peaks
  static void SetQPeakSizeScale(float V);    // to use with q-peaks
  static float GetMinQAlpha() { return MinQAlpha; }
  static void SetMinQAlpha(float v) { MinQAlpha = v; }
  void CalcRad(short DefAtomR);

  void ApplyStyle(TGraphicsStyle& S);
  void UpdateStyle(TGraphicsStyle& S);

  double GetR()  {  return Params()[0]; }
  void SetR(double r);
  void SetZoom(double Z);
  double GetZoom()  {  return Params()[1]; }
  // this center is 'graphics' center which is updated when the object is dragged
  const vec3d& GetCenter() const {  return Center;  }
  void NullCenter()  {  Center.Null();  }

  DefPropBFIsSet(SpecialDrawing, Flags, adsSpecial)

  double GetDrawScale() const {
    double scale = FParams[1];
    if( (FRadius & (darIsot|darIsotH)) != 0 )
      scale *= TelpProb();
    if( (FDrawStyle == adsEllipsoid || FDrawStyle == adsOrtep) &&
      GetEllipsoid() != NULL )
    {
      if( GetEllipsoid()->IsNPD() )
        return (caDefIso*2*scale);
      return scale;
    }
    else
      return FParams[0]*scale;
  }
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min);

  void ListParams(TStrList& List, TGlPrimitive* Primitive);
  // for parameters of a specific primitive
  void ListParams(TStrList& List);
  // fills the list with proposal primitives to construct object
  void ListPrimitives(TStrList& List) const;
  TGraphicsStyle& Style();
  void UpdatePrimitives(int32_t Mask);
  uint32_t GetPrimitiveMask() const;

  bool OnMouseDown(const IEObject* Sender, const TMouseData& Data);
  bool OnMouseUp(const IEObject* Sender, const TMouseData& Data);
  bool OnMouseMove(const IEObject* Sender, const TMouseData& Data);

  void SetVisible(bool v)  {
    AGDrawObject::SetVisible(v);
    if (!v) {
      if (Label->IsVisible()) {
        Label->SetVisible(false);
        label_forced = true;
      }
    }
    else if (label_forced) {
      Label->SetVisible(true);
      label_forced = false;
    }
  }
  void ListDrawingStyles(TStrList &List);
  short DrawStyle() const {  return FDrawStyle;  }
  void DrawStyle(short V);

  void UpdatePrimitiveParams(TGlPrimitive* GlP);
  void OnPrimitivesCleared();
  static int16_t Quality(int16_t Val);

  void SetPolyhedronType(short type);
  int GetPolyhedronType() const;
  Poly *GetPolyhedron() const {  return Polyhedron;  }

  const_strlist ToPov(olxdict<TGlMaterial, olxstr,
    TComparableComparator> &materials) const;
  static const_strlist PovDeclare();

  const_strlist ToWrl(olxdict<TGlMaterial, olxstr,
    TComparableComparator> &materials) const;
  static const_strlist WrlDeclare();

  static TGraphicsStyle* GetParamStyle() {  return FAtomParams;  }
  static void CreateStaticObjects(TGlRenderer& parent);

  static olxdict<olxstr, olxstr, olxstrComparator<false> > &NamesRegistry() {
    static olxdict<olxstr, olxstr, olxstrComparator<false> > nr;
    return nr;
  }
};

EndGxlNamespace()
#endif
