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

const int
  adsSphere       = 1,  // atom draw styles
  adsEllipsoid    = 2,
  adsStandalone   = 3,
  adsOrtep        = 4,
  adsSpecial      = 0x1000;

const int
  darPers     = 0x0001, // default atom radii
  darIsot     = 0x0002,
  darPack     = 0x0004,
  darBond     = 0x0008,
  darIsotH    = 0x0010,  // only affects H, others as darIsot
  darVdW      = 0x0020,
  darCustom   = 0x0040;
const int
  polyNone      = 0,
  polyAuto      = 1,  // polyhedron type
  polyRegular   = 2,
  polyPyramid   = 3,
  polyBipyramid = 4;

const size_t
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
  class Settings;
private:
  int FDrawStyle, FRadius;
  GLuint ActualSphere;
  // picks the correct sphere for rendering
  void InitActualSphere();
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
  TStrList* FindPrimitiveParams(TGlPrimitive *P) const;
  void ValidateRadius(TGraphicsStyle& GS);
  void ValidateDS(TGraphicsStyle& GS);
  vec3d Center;
  virtual bool DoTranslate(const vec3d& t) {  Center += t;  return true;  }
  virtual bool DoRotate(const vec3d&, double) {  return false;  }
  virtual bool DoZoom(double, bool)  {  return false;  }
  static const olxstr &PolyTypeName() {
    static olxstr v = "PolyType";
    return v;
  }
public:
  TXAtom(TNetwork* net, TGlRenderer& Render, const olxstr& collectionName);
  virtual ~TXAtom();
  void Create(const olxstr& cName=EmptyString());
  TXGlLabel& GetGlLabel() const {  return *Label;  }
  const TGlRenderer &GetParent() const { return Parent; }
  TGlRenderer &GetParent() { return Parent; }
  virtual void Update();
  virtual void UpdateLabel()  {  GetGlLabel().UpdateLabel();  }
  inline TXAtom& Node(size_t i) const {  return (TXAtom&)TSAtom::Node(i); }
  inline TXBond& Bond(size_t i) const {  return (TXBond&)TSAtom::Bond(i); }
  // returns full legend with symm code
  static olxstr GetLegend(const TSAtom& A, const short Level=2);
  // returns level of the given legend (number of '.', basically)
  static uint16_t LegendLevel(const olxstr& legend);
  // returns full legend for the label. e.g. "Q.Q1"
  static olxstr GetLabelLegend(const TSAtom& A);

  static void GetDefSphereMaterial(const TSAtom& A, TGlMaterial &M,
    const Settings &defs);
  static void GetDefSphereMaterial(const TSAtom& A, TGlMaterial &M,
    TGlRenderer &r)
  {
    return GetDefSphereMaterial(A, M, GetSettings(r));
  }
  static void GetDefRimMaterial(const TSAtom& A, TGlMaterial &M);

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
      scale *= GetSettings().GetTelpProb();
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

  // for parameters of a specific primitive
  void ListParams(TStrList& List, TGlPrimitive* Primitive);
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
  static int Quality(TGlRenderer &r, int Val);

  void SetPolyhedronType(short type);
  int GetPolyhedronType() const;
  Poly *GetPolyhedron() const {  return Polyhedron;  }

  const_strlist ToPov(olx_cdict<TGlMaterial, olxstr> &materials) const;
  static const_strlist PovDeclare(TGlRenderer &r);

  const_strlist ToWrl(olx_cdict<TGlMaterial, olxstr> &materials) const;
  static const_strlist WrlDeclare(TGlRenderer &r);

  static olxstr_dict<olxstr> &NamesRegistry() {
    static olxstr_dict<olxstr> nr;
    return nr;
  }

  class Settings : public AGOSettings {
    mutable double telp_prob, qpeak_scale, qpeak_size_scale, qpeak_min_alpha,
    zoom, radius, rim_r, rim_w,
    disk_or, disk_ir, // outer and inner radius
    disk_s; // disk separation
    mutable int r, quality, sphere_q, rim_q, disk_q,
      draw_style, mask;
    Settings(TGlRenderer &r)
      : AGOSettings(r, "AtomParams")
    {
      set_defaults();
    }
    void set_defaults() {
      telp_prob = qpeak_scale = qpeak_size_scale = qpeak_min_alpha = zoom =
        radius = rim_r = rim_w = disk_or = disk_ir = disk_s = -1;
      r = quality = sphere_q = rim_q = disk_q = draw_style = mask = -1;
    }
    void OnStyleChange() {
      ClearPrimitives();
      set_defaults();
    }
    void OnRendererClear() {
      ClearPrimitives();
    }
    void CreatePrimitives();
    TStringToList<olxstr, TGlPrimitive*> primitives;
  public:
    double GetRimR() const { return GetParam("RimR", rim_r, double(1.02)); }
    void SetRimR(double v) { style->SetParam("RimR", (rim_r = v), true); }
    double GetRimW() const { return GetParam("RimW", rim_w, double(0.05)); }
    void SetRimW(double v) { style->SetParam("RimW", (rim_w = v), true); }
    double GetDiskOR() const { return GetParam("DiskOR", disk_or, double(1.02)); }
    void SetDiskOR(double v) { style->SetParam("DiskOR", (disk_or = v), true); }
    double GetDiskIR() const { return GetParam("DiskIR", disk_ir, double(0)); }
    void SetDiskIR(double v) { style->SetParam("DiskIR", (disk_ir = v), true); }
    double GetDiskS() const { return GetParam("DiskS", disk_s, double(0.05)); }
    void SetDiskS(double v) { style->SetParam("DiskS", (disk_s = v), true); }

    double GetZoom() const { return GetParam("DefZ", zoom, double(1)); }
    void SetZoom(double v) { style->SetParam("DefZ", (zoom = v), true); }

    double GetTelpProb() const {
      return GetParam("TelpP", telp_prob, double(1));
    }
    void SetTelpProb(double v) {
      style->SetParam("TelpP", (telp_prob = v), true);
    }
    double GetQPeakScale() const {
      return GetParam("QPeakScale", qpeak_scale, double(3));
    }
    void SetQPeakScale(double v) {
      style->SetParam("QPeakScale", (qpeak_scale = v), true);
    }
    double GetQPeakSizeScale() const {
      return GetParam("QPeakSizeScale", qpeak_size_scale, double(1));
    }
    void SetQPeakSizeScale(double v) {
      style->SetParam("QPeakSizeScale", (qpeak_size_scale = v), true);
    }
    double GetQPeakMinAlpha() const {
      return GetParam("QPeakMinAlpha", qpeak_min_alpha, double(0.1));
    }
    void SetQPeakMinAlpha(double v) {
      style->SetParam("QPeakMinAlpha", (qpeak_min_alpha = v), true);
    }

    int GetR() const { return GetParam("DefR", r, int(darPers)); }
    void SetR(int v) { style->SetParam("DefR", (r = v), true); }

    int GetDS() const { return GetParam("DefDS", draw_style, int(adsSphere)); }
    void SetDS(int v) { style->SetParam("DefDS", (draw_style = v), true); }

    int GetMask() const { return GetParam("DefMask", mask, int(5)); }
    void SetMask(int v) { style->SetParam("DefMask", (mask = v), true); }

    int GetSphereQ() const { return GetParam("SphereQ", sphere_q, int(15)); }
    void SetSphereQ(int v) const {
      return style->SetParam("SphereQ", (sphere_q = v), true);
    }
    int GetRimsQ() const { return GetParam("RimQ", rim_q, int(15)); }
    void SetRimQ(int v) const {
      return style->SetParam("RimQ", (rim_q = v), true);
    }
    int GetDiskQ() const { return GetParam("DiskQ", disk_q, int(15)); }
    void SetDiskQ(int v) const {
      return style->SetParam("DiskQ", (disk_q = v), true);
    }
    int GetQuality() const { return GetParam("Quality", quality, qaMedium); }
    void SetQuality(int v) const {
      return style->SetParam("Quality", (quality = v), true);
    }
    size_t PolyhedronIndex,
      SphereIndex,
      SmallSphereIndex,
      RimsIndex,
      DisksIndex,
      CrossIndex,
      TetrahedronIndex
      ;
    GLuint
      OrtepSpheres, // 8 glLists
      LockedAtomSphere, // 1 list
      ConstrainedAtomSphere // 1 list
      ;
    float TelpProb, QPeakScale, QPeakSizeScale, DefZoom;
    TTypeList<TGlPrimitiveParams> PrimitiveParams;

    const TStringToList<olxstr, TGlPrimitive*> &GetPrimitives() const {
      return primitives;
    }
    const TStringToList<olxstr, TGlPrimitive*> &GetPrimitives(bool check)  {
      if (check && primitives.IsEmpty()) {
        CreatePrimitives();
      }
      return primitives;
    }
    void ClearPrimitives();
    static Settings& GetInstance(TGlRenderer &r) {
      AGOSettings *s = r.FindSettings("atoms");
      if (s == NULL) {
        return (Settings &)r.RegisterSettings(*(new Settings(r)), "atoms");
      }
      else {
        return dynamic_cast<Settings &>(*s);
      }
    }
  };
protected:
  mutable Settings *settings;
public:
  Settings &GetSettings() const {
    return *(settings == 0 ? (settings=&Settings::GetInstance(Parent))
      : settings);
  }
  static Settings &GetSettings(TGlRenderer &r) {
    return Settings::GetInstance(r);
  }
  virtual void OnStyleChange() {
    settings = &GetSettings(Parent);
  }
  virtual void OnPrimitivesCleared() {
    GetSettings().ClearPrimitives();
  }
};

class TXAtomLabelAligner {
  TPtrList<TXAtom> Atoms;
  double Offset;
  size_t Positions;
public:
  TXAtomLabelAligner(const TPtrList<TXAtom> & atoms,
    double offset=1.3, size_t positions=15);
  void Align();

  DefPropP(double, Offset)
  DefPropP(size_t, Positions)

  static double calc_overlap(unsigned const char *data, size_t w, size_t h,
    size_t x_, size_t y_, const TTextRect &r_);
  static void fill_rect(unsigned char *data, size_t w, size_t h,
    size_t x_, size_t y_, const TTextRect &r_);
};

typedef TTypeList<TXAtom> TXAtomList;
typedef TPtrList<TXAtom> TXAtomPList;

EndGxlNamespace()
#endif
