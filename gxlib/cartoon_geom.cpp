/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "cartoon_geom.h"

BeginGxlNamespace()

namespace cartoon {

namespace {
  vec3f CatmullRom(const vec3f &p0, const vec3f &p1,
    const vec3f &p2, const vec3f &p3, float t)
  {
    const float t2 = t*t, t3 = t2*t;
    return (p1*2.0f
      + (p2 - p0)*t
      + (p0*2.0f - p1*5.0f + p2*4.0f - p3)*t2
      + (p1*3.0f - p0 - p2*3.0f + p3)*t3) * 0.5f;
  }

  vec3f AnyPerpendicular(const vec3f &v) {
    vec3f a = (olx_abs(v[0]) < olx_abs(v[2])) ? vec3f(1, 0, 0) : vec3f(0, 0, 1);
    vec3f r = v.XProdVec(a);
    const float l = r.Length();
    return (l > 1e-6f) ? r/l : vec3f(0, 1, 0);
  }

  vec3f Normalised(const vec3f &v, const vec3f &fallback) {
    const float l = v.Length();
    return (l > 1e-6f) ? v/l : fallback;
  }

  /* One cross-section along the chain: where it is, which way the chain runs,
  which way the ribbon lies flat, and how big it is there.
  */
  struct Sample {
    vec3f c, tangent, side;
    float w, h;
  };

  /* Emits the rings and triangles for a run of samples.

  The winding puts the outward face counter-clockwise, which OpenGL reads as
  the front. Getting it backwards is invisible while the material is opaque,
  because culling is only enabled for transparent ones, but it lights the
  surface from the GL_BACK material.
  */
  void StitchSamples(const TTypeList<Sample> &samples, size_t slices, Mesh &m) {
    if (samples.Count() < 2 || slices < 3) {
      return;
    }
    TArrayList<float> cs(slices), sn(slices);
    for (size_t i = 0; i < slices; i++) {
      const double a = 2*M_PI*double(i)/double(slices);
      cs[i] = float(cos(a));
      sn[i] = float(sin(a));
    }
    const size_t base_vertex = m.vertices.Count();
    for (size_t i = 0; i < samples.Count(); i++) {
      const Sample &s = samples[i];
      const vec3f up = s.tangent.XProdVec(s.side);
      for (size_t k = 0; k < slices; k++) {
        m.vertices.AddCopy(s.c + s.side*(s.w*cs[k]) + up*(s.h*sn[k]));
        /* The normal of an ellipse is not the direction of the point: for
        semi-axes w and h it is (h cos, w sin), the axes swapped. Using the
        point direction instead lights a flat ribbon as though it were round.
        */
        m.normals.AddCopy(Normalised(s.side*(s.h*cs[k]) + up*(s.w*sn[k]),
          s.side));
      }
    }
    for (size_t r = 0; r + 1 < samples.Count(); r++) {
      const size_t r1 = base_vertex + r*slices, r2 = r1 + slices;
      for (size_t k = 0; k < slices; k++) {
        const size_t k2 = (k + 1) % slices;
        m.triangles.AddNew(r1 + k, r2 + k2, r2 + k);
        m.triangles.AddNew(r1 + k, r1 + k2, r2 + k2);
      }
    }
  }

  // cross-section size of a residue, before the arrowhead is applied
  void SizeOf(short ss, const CartoonParams &p, float &w, float &h) {
    using namespace xlib::protein;
    if (ss == ss_helix) {
      w = p.helix_width;
      h = p.helix_thickness;
    }
    else if (ss == ss_strand) {
      w = p.strand_width;
      h = p.strand_thickness;
    }
    else {
      w = h = p.coil_radius;
    }
  }
}

//.............................................................................
uint32_t RainbowColour(double t) {
  t = olx_max(0.0, olx_min(1.0, t));
  /* Four linear legs through blue, cyan, green, yellow, red. Piecewise linear
  in RGB rather than a hue sweep: a hue sweep of the same span passes through
  magenta on the way back round, and the ends stop being distinguishable.
  */
  double r, g, b;
  const double s = t*4;
  if (s < 1) { r = 0; g = s; b = 1; }
  else if (s < 2) { r = 0; g = 1; b = 2 - s; }
  else if (s < 3) { r = s - 2; g = 1; b = 0; }
  else { r = 1; g = 4 - s; b = 0; }
  /* The OLX_RGBA packing, written out rather than included: gldefs.h lives in
  glib, and this file is deliberately buildable without the graphics layer so
  that it can be compiled into the test binary.
  */
  return (uint32_t)(255*r + 0.5)
    | ((uint32_t)(255*g + 0.5) << 8)
    | ((uint32_t)(255*b + 0.5) << 16)
    | 0xff000000u;
}
//.............................................................................
vec3f RibbonWidthDirection(const vec3f &chain_step, const vec3f &carbonyl) {
  const float al = chain_step.Length();
  if (al < 1e-6f) {
    return vec3f(0, 0, 0);
  }
  const vec3f a = chain_step/al;
  const vec3f d = carbonyl - a*carbonyl.DotProd(a);
  const float dl = d.Length();
  return (dl > 1e-6f) ? d/dl : vec3f(0, 0, 0);
}
//.............................................................................
void BuildCartoon(const xlib::protein::ChainSegment &seg,
  const TArrayList<short> &ss_in, const CartoonParams &p, Mesh &m)
{
  using namespace xlib::protein;
  const size_t rc = seg.residues.Count();
  if (rc < 2 || p.slices < 3) {
    return;
  }
  const size_t steps = olx_max(size_t(1), p.steps_per_residue);
  const size_t slices = p.slices;
  const size_t base_triangle = m.triangles.Count();

  TArrayList<short> ss(rc);
  for (size_t i = 0; i < rc; i++) {
    ss[i] = (ss_in.Count() == rc && !p.trace_only) ? ss_in[i] : ss_coil;
  }

  /* Per-residue ribbon direction, following Carson and Bugg: the peptide plane
  is spanned by the chain direction and the carbonyl, so their cross product is
  the direction the ribbon lies flat along. Successive normals are flipped to
  agree with the previous one, which is what prevents the 180 degree twists that
  otherwise appear along a strand.

  A residue with no carbonyl oxygen - which the backbone classifier shows is
  common enough to plan for - falls back to a frame carried along the curve.
  */
  TArrayList<vec3f> side(rc);
  vec3f carried;
  bool have_carried = false;
  for (size_t i = 0; i < rc; i++) {
    const vec3f ca(seg.residues[i].ca);
    const vec3f next(seg.residues[i + 1 < rc ? i + 1 : i].ca);
    const vec3f prev(seg.residues[i > 0 ? i - 1 : i].ca);
    vec3f dir = (i + 1 < rc) ? (next - ca) : (ca - prev);
    dir = Normalised(dir, vec3f(0, 0, 1));
    vec3f s;
    if (seg.residues[i].has_o) {
      s = RibbonWidthDirection(dir, vec3f(seg.residues[i].o) - ca);
      if (s.QLength() < 1e-8f) {
        s = have_carried ? carried : AnyPerpendicular(dir);
      }
    }
    else if (have_carried) {
      s = carried;
    }
    else {
      s = AnyPerpendicular(dir);
    }
    // orthogonalise against the local chain direction, then agree with the last
    s = Normalised(s - dir*s.DotProd(dir),
      have_carried ? carried : AnyPerpendicular(dir));
    if (have_carried && s.DotProd(carried) < 0) {
      s = -s;
    }
    side[i] = s;
    carried = s;
    have_carried = true;
  }

  /* Arrowheads. The head occupies the last residue of a strand run, and the
  shoulder in front of it is a step rather than a ramp: two samples at the same
  point, one at strand width and one at arrow width, which is the flat face of
  the arrow.
  */
  TArrayList<bool> head(rc), shoulder(rc);
  for (size_t i = 0; i < rc; i++) {
    head[i] = shoulder[i] = false;
  }
  for (size_t i = 0; i < rc; i++) {
    if (ss[i] == ss_strand && (i + 1 == rc || ss[i+1] != ss_strand)) {
      head[i] = true;
      if (i > 0 && ss[i-1] == ss_strand) {
        shoulder[i] = true;
      }
    }
  }

  TTypeList<Sample> samples;
  TArrayList<size_t> residue_first_sample(rc);
  for (size_t i = 0; i < rc; i++) {
    residue_first_sample[i] = samples.Count();
  }

  for (size_t i = 0; i + 1 < rc; i++) {
    residue_first_sample[i] = samples.Count();
    const vec3f p1(seg.residues[i].ca);
    const vec3f p2(seg.residues[i+1].ca);
    const vec3f p0 = (i == 0) ? (p1*2.0f - p2) : vec3f(seg.residues[i-1].ca);
    const vec3f p3 = (i + 2 < rc) ? vec3f(seg.residues[i+2].ca)
      : (p2*2.0f - p1);
    // the final pair contributes the closing sample as well
    const size_t n = (i + 2 == rc) ? steps + 1 : steps;

    float w0, h0, w1, h1;
    SizeOf(ss[i], p, w0, h0);
    SizeOf(ss[i+1], p, w1, h1);
    if (head[i]) {
      w0 = p.arrow_width;
      w1 = p.arrow_tip;
      h1 = h0;
    }
    else if (head[i+1]) {
      // the next residue is the head, so this stretch ends at the shoulder
      w1 = p.strand_width;
    }

    for (size_t s = 0; s < n; s++) {
      const float t = float(s)/float(steps);
      Sample smp;
      smp.c = CatmullRom(p0, p1, p2, p3, t);
      vec3f tangent = CatmullRom(p0, p1, p2, p3, olx_min(1.0f, t + 0.01f))
        - CatmullRom(p0, p1, p2, p3, olx_max(0.0f, t - 0.01f));
      const float tl = tangent.Length();
      if (tl < 1e-6f) {
        continue;
      }
      smp.tangent = tangent/tl;
      const vec3f sd = side[i]*(1 - t) + side[i+1]*t;
      smp.side = Normalised(sd - smp.tangent*sd.DotProd(smp.tangent),
        AnyPerpendicular(smp.tangent));
      smp.w = w0 + (w1 - w0)*t;
      smp.h = h0 + (h1 - h0)*t;
      if (s == 0 && shoulder[i]) {
        // the flat face: the same point at the width the strand had
        Sample flat = smp;
        flat.w = p.strand_width;
        samples.AddCopy(flat);
      }
      samples.AddCopy(smp);
    }
  }
  residue_first_sample[rc-1] = samples.IsEmpty() ? 0 : samples.Count() - 1;

  StitchSamples(samples, slices, m);

  /* Per-residue triangle offsets, so a residue subset maps to a contiguous
  triangle range without searching. n samples make n-1 stretches, so the last
  residue closes the chain and owns none of them; giving it the sample count
  instead would put its offset past the closing sentinel.
  */
  for (size_t i = 0; i < rc; i++) {
    m.residue_triangle_offset.Add(
      base_triangle + residue_first_sample[i]*slices*2);
  }
  m.residue_triangle_offset.Add(m.triangles.Count());
}
//.............................................................................
void BuildTube(const xlib::protein::ChainSegment &seg, const TubeParams &tp,
  Mesh &m)
{
  CartoonParams p;
  p.coil_radius = tp.radius;
  p.steps_per_residue = tp.steps_per_residue;
  p.slices = tp.slices;
  p.trace_only = true;
  BuildCartoon(seg, TArrayList<short>(), p, m);
}
//.............................................................................

} // namespace cartoon

EndGxlNamespace()
