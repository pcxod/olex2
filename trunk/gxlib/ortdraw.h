/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ort_Draw_H
#define __olx_ort_Draw_H
#include "gxapp.h"
#include "ps_writer.h"

static const uint16_t
  ortep_color_lines = 0x0001,
  ortep_color_fill  = 0x0002,
  ortep_color_bond  = 0x0004,
  ortep_atom_rims   = 0x0008,
  ortep_atom_quads  = 0x0010;

class OrtDraw;

static const vec3f NullVec;

struct a_ort_object {
  const OrtDraw& parent;
public:
  a_ort_object(const OrtDraw& _parent) : parent(_parent) {} 
  virtual ~a_ort_object(){}
  virtual void render(PSWriter&) const = 0;
  virtual float get_z() const = 0;
  virtual void update_size(evecf &size) const {  return;  }
  static void update_min_max(evecf &size, const vec3f& crd)  {
    if( size[0] > crd[0] )  size[0] = crd[0];
    if( size[2] < crd[0] )  size[2] = crd[0];
    if( size[1] > crd[1] )  size[1] = crd[1];
    if( size[3] < crd[1] )  size[3] = crd[1];
  }
};

struct ort_atom : public a_ort_object {
  const TXAtom& atom;
  vec3f crd;
  mat3f *p_elpm, *p_ielpm, *elpm;
  float draw_rad;
  uint16_t draw_style;
  uint32_t sphere_color, rim_color, mask;
  ort_atom(const OrtDraw& parent, const TXAtom& a);
  ~ort_atom()  {
    if( p_elpm != NULL )  delete p_elpm;
    if( p_ielpm != NULL )  delete p_ielpm;
    if( elpm != NULL )  delete elpm;
  }
  virtual void render(PSWriter&) const;
  virtual float get_z() const {  return crd[2];  }
  virtual void update_size(evecf &size) const;
  bool IsSpherical() const;
  bool IsSolid() const {  return mask != 0 && mask != 16;  }
protected:
  void render_rims(PSWriter&) const;
  void render_elp(PSWriter&) const;
  void render_sph(PSWriter&) const;
  void render_standalone(PSWriter&) const;
};


template <class draw_t>
struct ort_bond : public a_ort_object  {
  const draw_t &object;
  const ort_atom &atom_a, &atom_b;
  const bool swapped;
  uint16_t draw_style;
  ort_bond(const OrtDraw& parent, const draw_t &object,
    const ort_atom& a1, const ort_atom& a2);
  virtual void render(PSWriter&) const;
  //virtual float get_z() const {  return (atom_a.crd[2]+atom_b.crd[2])/2;  }
  virtual float get_z() const {
    return olx_min(atom_a.crd[2], atom_b.crd[2])+0.01f;
  }
protected:
  void _render(PSWriter&, float scalex, uint32_t mask) const;
  uint32_t get_color(int primitive, uint32_t def) const;
};

struct ort_bond_line : public a_ort_object  {
  const TXLine &line;
  ort_atom *a1, *a2;
  vec3f from, to, p_from, p_to;
  uint16_t draw_style;
  ort_bond_line(const OrtDraw& parent, const TXLine& line,
    const vec3f& from, const vec3f& to);
  virtual void render(PSWriter&) const;
  //virtual float get_z() const { return (p_from[2] + p_to[2]) / 2; }
  virtual float get_z() const { return olx_min(p_from[2], p_to[2]); }
  virtual void update_size(evecf &sz) const {
    a_ort_object::update_min_max(sz, p_from);
    a_ort_object::update_min_max(sz, p_to);
  }
protected:
  void _render(PSWriter&, float scalex, uint32_t mask) const;
  uint32_t get_color(int primitive, uint32_t def=0xFF) const;
};

struct ort_line : public a_ort_object  {
  vec3f from, to;
  uint32_t color;
  ort_line(const OrtDraw& parent, const vec3f& _from, const vec3f _to,
    uint32_t _color)
    :  a_ort_object(parent), from(_from), to(_to), color(_color)
  {}
  virtual void render(PSWriter&) const;
  //virtual float get_z() const {  return (from[2]+to[2])/2;  }
  virtual float get_z() const { return olx_min(from[2], to[2]); }
  void update_size(evecf &sz) const {
    a_ort_object::update_min_max(sz, from);
    a_ort_object::update_min_max(sz, to);
  }
};

struct ort_poly : public a_ort_object  {
  vec3f_list points;
  bool fill;
  float line_width;
  uint32_t color;
  ort_poly(const OrtDraw& parent, bool _fill) :
    a_ort_object(parent),
    fill(_fill),
    line_width(1.0f),
    color(0x0) {  }
  virtual void render(PSWriter&) const;
  virtual float get_z() const {
    float z = 0;
    for ( size_t i=0; i < points.Count(); i++)
      z += points[i][2];
    return points.IsEmpty() ? 0 : z/points.Count();
  }
  void update_size(evecf &sz) const {
    for (size_t i=0; i < points.Count(); i++)
      a_ort_object::update_min_max(sz, points[i]);
  }
};

struct ort_circle : public a_ort_object {
  vec3f center;
  bool fill;
  float line_width, r;
  mat3f* basis;
  uint32_t color;
  ort_circle(const OrtDraw& parent, const vec3f& _center, float _r, bool _fill)
    : a_ort_object(parent),
      center(_center),
      fill(_fill),
      line_width(1.0f),
      r(_r),
      basis(NULL),
      color(0) {}
  ~ort_circle()  {
    if( basis != NULL )  delete basis;
  }
  virtual void render(PSWriter&) const;
  virtual float get_z() const {  return center[2];  }
  void update_size(evecf &sz) const;
};

struct ort_cone : public a_ort_object  {
  vec3f bottom, top;
  float bottom_r, top_r;
  uint32_t color;
  uint16_t divs;
  ort_cone(const OrtDraw& parent, const vec3f& _b, const vec3f _t, float br,
           float tr, uint32_t cl)
    : a_ort_object(parent),
      bottom(_b),
      top(_t),
      bottom_r(br),
      top_r(tr),
      color(cl),
      divs(5) {}
  virtual void render(PSWriter&) const;
  virtual float get_z() const {  return (bottom[2]+top[2])/2;  }
  void update_size(evecf &sz) const;
};

class OrtDraw  {
private:
  static int OrtObjectsZSort(const a_ort_object &a1, const a_ort_object &a2)  {
    return olx_cmp_float(a1.get_z(), a2.get_z(), 1e-3f);
  }
  float DrawScale, BondRad, LinearScale, YOffset;
  mat3f ProjMatr, UnProjMatr;
  vec3f DrawOrigin, SceneOrigin;
  TGXApp& app;
  uint16_t ColorMode;
  uint32_t BondOutlineColor, AtomOutlineColor;
  float BondOutlineSize, AtomOutlineSize;
protected:
  float PieLineWidth,
    ElpLineWidth,
    QuadLineWidth,
    FontLineWidth,
    HBondScale, MultipleBondsWidth;
  const TEBasis& basis;
  bool Perspective, AutoStippleDisorder;
  uint16_t ElpDiv, PieDiv, BondDiv;
  mutable TArrayList<vec3f> ElpCrd, PieCrd, Arc, BondCrd, BondProjF, BondProjT,
    BondProjM;
  mutable TPtrList<const vec3f> FilteredArc;
  size_t PrepareArc(const TArrayList<vec3f>& in, TPtrList<const vec3f>& out,
    const vec3f& normal) const;
  template <class draw_t>
  float GetBondRad(const ort_bond<draw_t>& b, uint32_t mask) const {
    float r = (b.atom_a.atom.GetType() == iHydrogenZ ||
      b.atom_b.atom.GetType() == iHydrogenZ) ? BondRad*HBondScale : BondRad;
    if ((mask&((1<<13)|(1<<12)|(1<<11)|(1<<7)|(1<<6))) != 0)
      r /= 4;
    else if ((mask&((1 << 14) | (1 << 15))) != 0)
      r *= 1.25;
    else if ((mask&((1 << 16) | (1 << 17))) != 0)
      r *= 1.5;
    return r;
  }
  float GetLineRad(const ort_bond_line& b, uint32_t mask) const;
  vec3f ProjectPoint(const vec3f& p) const {
    return (p + SceneOrigin)*ProjMatr+DrawOrigin;
  }
  void RenderRims(PSWriter& pw, const mat3f& pm, const vec3f& normal) const;
  void RenderQuads(PSWriter& pw, const mat3f& pm) const;
  void _process_points(TPtrList<vec3f>& points, ort_poly& otp)  {
    for( size_t i=0; i < otp.points.Count(); i++ )
      points.Add(otp.points[i]);
  }
  struct ContourDrawer {
    const OrtDraw& parent;
    TTypeList<a_ort_object>& objects;
    uint32_t color;
    ContourDrawer(const OrtDraw& _parent, TTypeList<a_ort_object>& _objects,
      uint32_t _color)
      : parent(_parent), objects(_objects), color(_color)  {}
    void draw(float x1, float y1, float x2, float y2, float z);
  };
  TPtrList<ort_atom> atoms;
public:
  OrtDraw() : app(TGXApp::GetInstance()), basis(app.GetRender().GetBasis()) {
    ElpDiv = 36;
    BondDiv = 12;
    PieDiv = 4;
    QuadLineWidth = PieLineWidth = 0.5;
    ElpLineWidth = FontLineWidth = 1;
    BondRad = 1;
    ColorMode = 0;
    HBondScale = 0.5;
    AtomOutlineColor = BondOutlineColor = ~0;
    BondOutlineSize = 0.1;
    AtomOutlineSize = 0.05;
    Perspective = false;
    AutoStippleDisorder = true;
    MultipleBondsWidth = 0;
  }
  // create ellipse and pie coordinates
  void Init(PSWriter& pw);

  void Render(const olxstr& fileName);

  static size_t stipples_for_mask(uint32_t mask, bool half) {
    size_t rv = 0;
    if ((mask &((1 << 13) | (1 << 11) | (1 << 10) | (1 << 9) | (1 << 8))) != 0)
      return half ? 6 : 12;
    return 0;
  }
  size_t GetStippleCount(const ort_bond<TXBond> &b, bool half) const {
    if (!IsAutoStippleDisorder()) return 0;
    int8_t pa = b.object.A().CAtom().GetPart(),
      pb = b.object.B().CAtom().GetPart();
    if ((pa != 0 && pa != 1) || (pb != 0 && pb != 1))
      return (half ? 6 :12);
    return stipples_for_mask(b.object.GetPrimitiveMask(), half);
  }
  template <class draw_t>
  size_t GetStippleCount(const ort_bond<draw_t> &b, bool half) const {
    return stipples_for_mask(b.object.GetPrimitiveMask(), half);
  }
  size_t GetStippleCount(const ort_bond_line &l, bool half) const;

  DefPropP(uint16_t, ElpDiv)
  DefPropP(uint16_t, PieDiv)
  DefPropP(uint16_t, BondDiv)
  DefPropP(uint16_t, ColorMode)
  DefPropP(uint32_t, BondOutlineColor)
  DefPropP(uint32_t, AtomOutlineColor)
  DefPropP(float, BondOutlineSize)
  DefPropP(float, AtomOutlineSize)
  DefPropP(float, HBondScale)
  DefPropP(float, FontLineWidth)
  DefPropP(float, QuadLineWidth)
  DefPropP(float, PieLineWidth)
  DefPropP(float, ElpLineWidth)
  DefPropP(float, MultipleBondsWidth)
  DefPropBIsSet(Perspective)
  DefPropBIsSet(AutoStippleDisorder)

  friend struct ort_bond_base;
  friend struct ort_bond<TXBond>;
  friend struct ort_bond<TXLine>;
  friend struct ort_bond_line;
  friend struct ort_atom;
  friend struct ort_poly;
};


#endif
